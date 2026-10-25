#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdbool.h>

typedef enum {
  BTN_NONE = -1,
  BTN_OK = 0,
  BTN_UP,
  BTN_DOWN,
  BTN_BACK,
  BTN_COUNT
} button_t;

void buttons_init(void);

/* Called every BUTTONS_TICK_MS from the TIM2 interrupt. */
void buttons_tick(void);

/* Next queued press event (UP/DOWN auto-repeat while held), or BTN_NONE. */
button_t buttons_get(void);

/* Block until a press event arrives. */
button_t buttons_wait(void);

bool buttons_is_down(button_t button);

#endif
