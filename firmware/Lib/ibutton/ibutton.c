#include "ibutton.h"

#include "onewire.h"
#include "timebase.h"

#define CMD_READ_ROM 0x33
#define CMD_READ_ROM_ALT 0x0F

#define RESET_MIN_US 360
#define EDGE_TIMEOUT_US 5000

static const onewire_t reader_line = {IBT_RW_GPIO_Port, IBT_RW_Pin};
static const onewire_t emu_line = {IBT_EMU_GPIO_Port, IBT_EMU_Pin};

ibutton_status_t ibutton_read(ibutton_key_t *key) {
  if (!onewire_reset(&reader_line)) return IBUTTON_NO_DEVICE;
  onewire_write_byte(&reader_line, CMD_READ_ROM);
  uint8_t id[IBUTTON_ID_SIZE];
  for (uint8_t i = 0; i < IBUTTON_ID_SIZE; i++) {
    id[i] = onewire_read_byte(&reader_line);
  }
  if (onewire_crc8(id, IBUTTON_ID_SIZE) != 0) return IBUTTON_CRC_ERROR;
  for (uint8_t i = 0; i < IBUTTON_ID_SIZE; i++) key->id[i] = id[i];
  return IBUTTON_OK;
}

bool ibutton_key_valid(const ibutton_key_t *key) {
  return onewire_crc8(key->id, IBUTTON_ID_SIZE) == 0;
}

/* Waits for the emulation line to reach `level`; false on timeout. */
static bool wait_level(bool level, uint16_t timeout_us) {
  uint16_t start = micros16();
  while (onewire_level(&emu_line) != level) {
    if ((uint16_t)(micros16() - start) > timeout_us) return false;
  }
  return true;
}

static bool receive_byte(uint8_t *byte) {
  uint8_t value = 0;
  for (uint8_t i = 0; i < 8; i++) {
    if (!wait_level(false, EDGE_TIMEOUT_US)) return false;
    delay_us(25);
    value >>= 1;
    if (onewire_level(&emu_line)) value |= 0x80;
    if (!wait_level(true, EDGE_TIMEOUT_US)) return false;
  }
  *byte = value;
  return true;
}

static bool send_id(const ibutton_key_t *key) {
  for (uint8_t i = 0; i < IBUTTON_ID_SIZE; i++) {
    for (uint8_t b = 0; b < 8; b++) {
      if (!wait_level(false, EDGE_TIMEOUT_US)) return false;
      if (!((key->id[i] >> b) & 1)) {
        onewire_low(&emu_line);
        delay_us(30);
        onewire_release(&emu_line);
      }
      if (!wait_level(true, EDGE_TIMEOUT_US)) return false;
    }
  }
  return true;
}

bool ibutton_emulate_poll(const ibutton_key_t *key) {
  onewire_release(&reader_line);
  onewire_release(&emu_line);
  if (onewire_level(&emu_line)) return false;

  uint16_t fell = micros16();
  if (!wait_level(true, EDGE_TIMEOUT_US)) return false;
  if ((uint16_t)(micros16() - fell) < RESET_MIN_US) return false;

  delay_us(30);
  onewire_low(&emu_line);
  delay_us(120);
  onewire_release(&emu_line);

  uint8_t cmd;
  if (!receive_byte(&cmd)) return false;
  if (cmd != CMD_READ_ROM && cmd != CMD_READ_ROM_ALT) return false;
  return send_id(key);
}
