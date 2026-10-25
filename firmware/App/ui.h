#ifndef UI_H_
#define UI_H_

#include <stdint.h>

#include "st7735.h"

#define UI_ROWS 6 /* 8x12 text rows on the 160x80 display */

/* One 8x12 text row, padded with spaces to the full width. */
void ui_row(uint8_t row, const char *text, uint16_t color);

/* Clears the screen and shows a title in the first row. */
void ui_screen(const char *title);

/* Formats `len` bytes as upper-case hex into out (2*len + 1 bytes). */
void ui_hex(char *out, const uint8_t *data, uint8_t len);

#endif
