#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#include <stdbool.h>
#include <stdint.h>

#include "main.h"

/* 1-Wire line on an open-drain GPIO pin (standard speed, bit-banged). */
typedef struct {
  GPIO_TypeDef *port;
  uint16_t pin;
} onewire_t;

static inline void onewire_low(const onewire_t *ow) {
  ow->port->BSRR = (uint32_t)ow->pin << 16;
}

static inline void onewire_release(const onewire_t *ow) {
  ow->port->BSRR = ow->pin;
}

static inline bool onewire_level(const onewire_t *ow) {
  return (ow->port->IDR & ow->pin) != 0;
}

/* Master side. Returns true if a device answered with a presence pulse. */
bool onewire_reset(const onewire_t *ow);
void onewire_write_bit(const onewire_t *ow, bool bit);
bool onewire_read_bit(const onewire_t *ow);
void onewire_write_byte(const onewire_t *ow, uint8_t byte);
uint8_t onewire_read_byte(const onewire_t *ow);

/* Dallas CRC-8 (x^8 + x^5 + x^4 + 1). CRC over a valid ROM id is 0. */
uint8_t onewire_crc8(const uint8_t *data, uint8_t len);

#endif
