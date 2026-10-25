#ifndef TIMEBASE_H_
#define TIMEBASE_H_

#include <stdint.h>

#include "stm32l0xx.h"

/* Free-running 16-bit microsecond counter on TIM22 (wraps every 65.5 ms). */
void timebase_init(void);

static inline uint16_t micros16(void) { return (uint16_t)TIM22->CNT; }

/* Busy-wait, any value up to 65535 us per call. */
static inline void delay_us(uint16_t us) {
  uint16_t start = micros16();
  while ((uint16_t)(micros16() - start) < us) {
  }
}

#endif
