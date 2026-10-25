#include "ir.h"

#include <stdlib.h>

#include "main.h"
#include "timebase.h"

#define CARRIER_HZ 38000U
#define RX_FRAME_GAP_US 20000U
#define RX_MIN_PULSES 4

/* ---- Transmit ------------------------------------------------------- */

static volatile bool carrier_on;

void ir_tx_init(void) {
  __HAL_RCC_TIM21_CLK_ENABLE();
  /* The carrier ISR toggles the pin, so it fires at twice the carrier. */
  TIM21->PSC = 0;
  TIM21->ARR = (SystemCoreClock / (2 * CARRIER_HZ)) - 1;
  TIM21->EGR = TIM_EGR_UG;
  TIM21->SR = 0;
  TIM21->DIER = TIM_DIER_UIE;
  HAL_NVIC_SetPriority(TIM21_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM21_IRQn);
}

void ir_tx_carrier_isr(void) {
  TIM21->SR = 0;
  if (!carrier_on) return;
  if (IR_LED_GPIO_Port->ODR & IR_LED_Pin) {
    IR_LED_GPIO_Port->BSRR = (uint32_t)IR_LED_Pin << 16;
  } else {
    IR_LED_GPIO_Port->BSRR = IR_LED_Pin;
  }
}

static void delay_long_us(uint16_t us) { delay_us(us); }

static void tx_mark(uint16_t us) {
  TIM21->CNT = 0;
  carrier_on = true;
  TIM21->CR1 |= TIM_CR1_CEN;
  delay_long_us(us);
  carrier_on = false;
  TIM21->CR1 &= ~TIM_CR1_CEN;
  IR_LED_GPIO_Port->BSRR = (uint32_t)IR_LED_Pin << 16;
}

void ir_send_raw(const uint16_t *pulse, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    if (i % 2 == 0) {
      tx_mark(pulse[i]);
    } else {
      delay_long_us(pulse[i]);
    }
  }
}

void ir_send_frame(const ir_frame_t *frame) {
  ir_send_raw(frame->pulse, frame->len);
}

#define NEC_HDR_MARK 9000
#define NEC_HDR_SPACE 4500
#define NEC_BIT_MARK 560
#define NEC_ONE_SPACE 1690
#define NEC_ZERO_SPACE 560

void ir_send_nec(uint8_t address, uint8_t command) {
  uint32_t data = (uint32_t)address | ((uint32_t)(uint8_t)~address << 8) |
                  ((uint32_t)command << 16) | ((uint32_t)(uint8_t)~command << 24);
  tx_mark(NEC_HDR_MARK);
  delay_long_us(NEC_HDR_SPACE);
  for (uint8_t i = 0; i < 32; i++) {
    tx_mark(NEC_BIT_MARK);
    delay_long_us((data >> i) & 1 ? NEC_ONE_SPACE : NEC_ZERO_SPACE);
  }
  tx_mark(NEC_BIT_MARK);
}

/* ---- Receive -------------------------------------------------------- */

static volatile uint16_t rx_pulse[IR_MAX_PULSES];
static volatile uint16_t rx_len;
static volatile uint16_t rx_last_edge;
static volatile bool rx_started;
static volatile bool rx_running;

void ir_rx_start(void) {
  rx_len = 0;
  rx_started = false;
  rx_running = true;

  GPIO_InitTypeDef init = {0};
  init.Pin = IR_REC_Pin;
  init.Mode = GPIO_MODE_IT_RISING_FALLING;
  init.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(IR_REC_GPIO_Port, &init);
  __HAL_GPIO_EXTI_CLEAR_IT(IR_REC_Pin);
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

void ir_rx_stop(void) {
  rx_running = false;
  EXTI->IMR &= ~(uint32_t)IR_REC_Pin;
  __HAL_GPIO_EXTI_CLEAR_IT(IR_REC_Pin);
}

void ir_rx_edge_isr(void) {
  if (!__HAL_GPIO_EXTI_GET_IT(IR_REC_Pin)) return;
  __HAL_GPIO_EXTI_CLEAR_IT(IR_REC_Pin);
  if (!rx_running) return;

  uint16_t now = micros16();
  bool low = (IR_REC_GPIO_Port->IDR & IR_REC_Pin) == 0;
  if (!rx_started) {
    if (!low) return;
    rx_started = true;
  } else if (rx_len < IR_MAX_PULSES) {
    rx_pulse[rx_len++] = (uint16_t)(now - rx_last_edge);
  }
  rx_last_edge = now;
}

bool ir_rx_poll(ir_frame_t *frame) {
  if (!rx_running || !rx_started) return false;
  uint16_t idle = (uint16_t)(micros16() - rx_last_edge);
  if (idle < RX_FRAME_GAP_US && rx_len < IR_MAX_PULSES) return false;

  uint16_t len = rx_len;
  if (len < RX_MIN_PULSES) {
    /* noise burst: forget it and keep listening */
    rx_len = 0;
    rx_started = false;
    return false;
  }
  ir_rx_stop();
  for (uint16_t i = 0; i < len; i++) frame->pulse[i] = rx_pulse[i];
  frame->len = len;
  return true;
}

/* ---- Decode --------------------------------------------------------- */

static bool near(uint16_t value, uint16_t target, uint16_t tolerance) {
  return abs((int)value - (int)target) <= (int)tolerance;
}

ir_decoded_t ir_decode(const ir_frame_t *frame) {
  ir_decoded_t out = {IR_PROTO_UNKNOWN, 0};
  const uint16_t *p = frame->pulse;
  if (frame->len < 3 || !near(p[0], NEC_HDR_MARK, 1500)) return out;

  if (near(p[1], 2250, 600) && frame->len <= 4) {
    out.proto = IR_PROTO_NEC_REPEAT;
    return out;
  }
  if (!near(p[1], NEC_HDR_SPACE, 900) || frame->len < 66) return out;

  uint32_t data = 0;
  for (uint8_t i = 0; i < 32; i++) {
    uint16_t mark = p[2 + 2 * i];
    uint16_t space = p[3 + 2 * i];
    if (!near(mark, NEC_BIT_MARK, 300)) return out;
    if (near(space, NEC_ONE_SPACE, 450)) {
      data |= (uint32_t)1 << i;
    } else if (!near(space, NEC_ZERO_SPACE, 300)) {
      return out;
    }
  }
  out.proto = IR_PROTO_NEC;
  out.data = data;
  return out;
}
