#include "power.h"

#include "buttons.h"
#include "gpio.h"
#include "main.h"
#include "spi.h"
#include "st7735.h"

/* Pins that would leak into the switched-off peripheral rail. */
static void park_pins(void) {
  GPIO_InitTypeDef init = {0};
  init.Mode = GPIO_MODE_ANALOG;
  init.Pull = GPIO_NOPULL;
  init.Pin = DISP_CS_Pin | DISP_SCK_Pin | DISP_RES_Pin | DISP_MOSI_Pin |
             DISP_DC_Pin | IR_REC_Pin;
  HAL_GPIO_Init(GPIOA, &init);
  HAL_GPIO_WritePin(IR_LED_GPIO_Port, IR_LED_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(IBT_RW_GPIO_Port, IBT_RW_Pin, GPIO_PIN_RESET);
}

static void arm_wakeup(void) {
  GPIO_InitTypeDef init = {0};
  init.Pin = BUT_OK_Pin;
  init.Mode = GPIO_MODE_IT_RISING;
  init.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUT_OK_GPIO_Port, &init);
  __HAL_GPIO_EXTI_CLEAR_IT(BUT_OK_Pin);
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
}

void power_sleep(void) {
  st7735_sleep();
  HAL_GPIO_WritePin(POW_PERIPH_GPIO_Port, POW_PERIPH_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
  park_pins();

  /* OK is the wake-up button: it must be released or we wake at once. */
  while (HAL_GPIO_ReadPin(BUT_OK_GPIO_Port, BUT_OK_Pin) == GPIO_PIN_SET) {
  }
  arm_wakeup();

  HAL_NVIC_DisableIRQ(TIM2_IRQn);
  HAL_SuspendTick();
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

  /* Awake on MSI: bring everything back. */
  HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
  EXTI->IMR &= ~(uint32_t)BUT_OK_Pin;
  SystemClock_Config();
  HAL_ResumeTick();

  MX_GPIO_Init();                /* peripheral power on, pins as before */
  HAL_SPI_MspInit(&hspi1);       /* SCK/MOSI back to SPI alternate function */
  HAL_Delay(20);
  st7735_disp_init();

  /* Swallow the press that woke us up so the menu does not see it. */
  while (HAL_GPIO_ReadPin(BUT_OK_GPIO_Port, BUT_OK_Pin) == GPIO_PIN_SET) {
  }
  buttons_init();
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
}
