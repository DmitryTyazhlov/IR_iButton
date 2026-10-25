#ifndef IBUTTON_H_
#define IBUTTON_H_

#include <stdbool.h>
#include <stdint.h>

#define IBUTTON_ID_SIZE 8

/* 64-bit ROM id: family code, 6 byte serial, CRC-8. */
typedef struct {
  uint8_t id[IBUTTON_ID_SIZE];
} ibutton_key_t;

typedef enum {
  IBUTTON_OK,
  IBUTTON_NO_DEVICE,
  IBUTTON_CRC_ERROR,
} ibutton_status_t;

/* Reads the ROM id of a key touching the reader contact (Dallas DS1990A). */
ibutton_status_t ibutton_read(ibutton_key_t *key);

bool ibutton_key_valid(const ibutton_key_t *key);

/*
 * Emulates a DS1990A on the emulation contact. Call as often as possible:
 * returns immediately while the line is idle, and serves one complete reader
 * transaction (reset, presence, READ ROM) when a reader pulls the line low.
 * Returns true if the id was fully sent to the reader.
 */
bool ibutton_emulate_poll(const ibutton_key_t *key);

#endif
