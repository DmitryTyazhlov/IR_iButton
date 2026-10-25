#include "onewire.h"

#include "timebase.h"

/* Bit slots are timing critical: keep interrupts off for the slot only. */
#define SLOT_BEGIN() uint32_t primask = __get_PRIMASK(); __disable_irq()
#define SLOT_END() __set_PRIMASK(primask)

bool onewire_reset(const onewire_t *ow) {
  onewire_low(ow);
  delay_us(480);
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  onewire_release(ow);
  delay_us(70);
  bool present = !onewire_level(ow);
  __set_PRIMASK(primask);
  delay_us(410);
  return present;
}

void onewire_write_bit(const onewire_t *ow, bool bit) {
  SLOT_BEGIN();
  onewire_low(ow);
  delay_us(bit ? 6 : 60);
  onewire_release(ow);
  delay_us(bit ? 64 : 10);
  SLOT_END();
}

bool onewire_read_bit(const onewire_t *ow) {
  SLOT_BEGIN();
  onewire_low(ow);
  delay_us(3);
  onewire_release(ow);
  delay_us(10);
  bool bit = onewire_level(ow);
  delay_us(53);
  SLOT_END();
  return bit;
}

void onewire_write_byte(const onewire_t *ow, uint8_t byte) {
  for (uint8_t i = 0; i < 8; i++) {
    onewire_write_bit(ow, byte & 1);
    byte >>= 1;
  }
}

uint8_t onewire_read_byte(const onewire_t *ow) {
  uint8_t byte = 0;
  for (uint8_t i = 0; i < 8; i++) {
    byte >>= 1;
    if (onewire_read_bit(ow)) byte |= 0x80;
  }
  return byte;
}

uint8_t onewire_crc8(const uint8_t *data, uint8_t len) {
  uint8_t crc = 0;
  while (len--) {
    uint8_t byte = *data++;
    for (uint8_t i = 0; i < 8; i++) {
      uint8_t mix = (crc ^ byte) & 1;
      crc >>= 1;
      if (mix) crc ^= 0x8C;
      byte >>= 1;
    }
  }
  return crc;
}
