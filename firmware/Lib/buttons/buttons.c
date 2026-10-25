#include "buttons.h"

#include "main.h"

#define BUTTONS_TICK_MS 10
#define DEBOUNCE_TICKS 2
#define REPEAT_DELAY_TICKS 50
#define REPEAT_PERIOD_TICKS 15
#define QUEUE_SIZE 8

typedef struct {
  GPIO_TypeDef *port;
  uint16_t pin;
  bool active_high;
} button_hw_t;

/* OK has a pull-down (and doubles as BOOT0/WKUP1), the rest are pulled up. */
static const button_hw_t hw[BTN_COUNT] = {
    [BTN_OK] = {BUT_OK_GPIO_Port, BUT_OK_Pin, true},
    [BTN_UP] = {BUT_UP_GPIO_Port, BUT_UP_Pin, false},
    [BTN_DOWN] = {BUT_DOWN_GPIO_Port, BUT_DOWN_Pin, false},
    [BTN_BACK] = {BUT_BACK_GPIO_Port, BUT_BACK_Pin, false},
};

static volatile bool stable[BTN_COUNT];
static uint8_t candidate_ticks[BTN_COUNT];
static uint16_t held_ticks[BTN_COUNT];

static volatile button_t queue[QUEUE_SIZE];
static volatile uint8_t head;
static volatile uint8_t tail;

static bool raw_down(button_t b) {
  bool level = HAL_GPIO_ReadPin(hw[b].port, hw[b].pin) == GPIO_PIN_SET;
  return level == hw[b].active_high;
}

static void push(button_t b) {
  uint8_t next = (head + 1) % QUEUE_SIZE;
  if (next == tail) return;
  queue[head] = b;
  head = next;
}

void buttons_init(void) {
  head = tail = 0;
  for (int b = 0; b < BTN_COUNT; b++) {
    stable[b] = raw_down((button_t)b);
    candidate_ticks[b] = 0;
    held_ticks[b] = 0;
  }
}

void buttons_tick(void) {
  for (int i = 0; i < BTN_COUNT; i++) {
    button_t b = (button_t)i;
    bool now = raw_down(b);
    if (now != stable[b]) {
      if (++candidate_ticks[b] >= DEBOUNCE_TICKS) {
        stable[b] = now;
        candidate_ticks[b] = 0;
        held_ticks[b] = 0;
        if (now) push(b);
      }
      continue;
    }
    candidate_ticks[b] = 0;
    if (now && (b == BTN_UP || b == BTN_DOWN)) {
      if (++held_ticks[b] >= REPEAT_DELAY_TICKS) {
        held_ticks[b] = REPEAT_DELAY_TICKS - REPEAT_PERIOD_TICKS;
        push(b);
      }
    }
  }
}

button_t buttons_get(void) {
  if (head == tail) return BTN_NONE;
  button_t b = queue[tail];
  tail = (tail + 1) % QUEUE_SIZE;
  return b;
}

button_t buttons_wait(void) {
  button_t b;
  while ((b = buttons_get()) == BTN_NONE) {
  }
  return b;
}

bool buttons_is_down(button_t button) { return stable[button]; }
