#include "ui.h"

#define COLS (ST7735_WIDTH / 8)

void ui_row(uint8_t row, const char *text, uint16_t color) {
  char line[COLS + 1];
  uint8_t i = 0;
  while (i < COLS && text[i] != '\0') {
    line[i] = text[i];
    i++;
  }
  while (i < COLS) line[i++] = ' ';
  line[COLS] = '\0';
  st7735_output_text_8x12(line, 0, row * 12, color);
}

void ui_screen(const char *title) {
  st7735_output_background(ST7735_BLACK);
  ui_row(0, title, ST7735_CYAN);
}

void ui_hex(char *out, const uint8_t *data, uint8_t len) {
  static const char digits[] = "0123456789ABCDEF";
  for (uint8_t i = 0; i < len; i++) {
    out[2 * i] = digits[data[i] >> 4];
    out[2 * i + 1] = digits[data[i] & 0x0F];
  }
  out[2 * len] = '\0';
}
