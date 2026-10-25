#include "st7735.h"

#include "font.h"
#include "gpio.h"
#include "main.h"
#include "spi.h"
#include <stdbool.h>

static inline void st7735_reset(bool x) {
  if (x) {
    HAL_GPIO_WritePin(DISP_RES_GPIO_Port, DISP_RES_Pin, RESET);
  } else {
    HAL_GPIO_WritePin(DISP_RES_GPIO_Port, DISP_RES_Pin, SET);
  }
}

static inline void st7735_chip_select(bool x) {
  if (x) {
    HAL_GPIO_WritePin(DISP_CS_GPIO_Port, DISP_CS_Pin, RESET);
  } else {
    HAL_GPIO_WritePin(DISP_CS_GPIO_Port, DISP_CS_Pin, SET);
  }
}

static inline void st7735_set_command_mode() {
  HAL_GPIO_WritePin(DISP_DC_GPIO_Port, DISP_DC_Pin, RESET);
}

static inline void st7735_set_data_mode() {
  HAL_GPIO_WritePin(DISP_DC_GPIO_Port, DISP_DC_Pin, SET);
}

static inline void st7735_send_byte(uint8_t data) {
  while (!(SPI1->SR & SPI_SR_TXE))
    ;
  SPI1->DR = data;
  while (!(SPI1->SR & SPI_SR_RXNE))
    ;
}

static inline void st7735_write_command(uint8_t data) {
  st7735_set_command_mode();
  st7735_send_byte(data);
}

static inline void st7735_write_data(uint8_t data) {
  st7735_set_data_mode();
  st7735_send_byte(data);
}

void st7735_disp_init(void) {
  st7735_chip_select(false);
  st7735_reset(true);
  HAL_Delay(10);
  st7735_reset(false);
  st7735_chip_select(true);
  HAL_Delay(10);

  st7735_write_command(ST7735_SWRESET);
  HAL_Delay(120);
  st7735_write_command(ST7735_SLPOUT);
  HAL_Delay(120);

  st7735_write_command(ST7735_FRMCTR1);
  st7735_write_data(0x01);
  st7735_write_data(0x2C);
  st7735_write_data(0x2D);
  st7735_write_command(ST7735_FRMCTR2);
  st7735_write_data(0x01);
  st7735_write_data(0x2C);
  st7735_write_data(0x2D);
  st7735_write_command(ST7735_FRMCTR3);
  st7735_write_data(0x01);
  st7735_write_data(0x2C);
  st7735_write_data(0x2D);
  st7735_write_data(0x01);
  st7735_write_data(0x2C);
  st7735_write_data(0x2D);

  st7735_write_command(ST7735_INVCTR);
  // st7735_write_data(0x07);
  st7735_write_data(0x00);

  st7735_write_command(ST7735_PWCTR1);
  st7735_write_data(0xA2);
  st7735_write_data(0x02);
  st7735_write_data(0x84);
  st7735_write_command(ST7735_PWCTR2);
  st7735_write_data(0xC5);
  st7735_write_command(ST7735_PWCTR3);
  st7735_write_data(0x0A);
  st7735_write_data(0x00);
  st7735_write_command(ST7735_PWCTR3);
  st7735_write_data(0x8A);
  st7735_write_data(0x2A);
  st7735_write_command(ST7735_PWCTR5);
  st7735_write_data(0x8A);
  st7735_write_data(0xEE);
  st7735_write_data(0x0E);

  st7735_write_command(ST7735_INVON);
  st7735_write_command(ST7735_MADCTL);
  st7735_write_data(0xB8);

  st7735_write_command(ST7735_COLMOD);
  st7735_write_data(0x05);

  st7735_write_command(ST7735_GMCTRP1);
  st7735_write_data(0x02);
  st7735_write_data(0x1c);
  st7735_write_data(0x07);
  st7735_write_data(0x12);
  st7735_write_data(0x37);
  st7735_write_data(0x32);
  st7735_write_data(0x29);
  st7735_write_data(0x2d);
  st7735_write_data(0x29);
  st7735_write_data(0x25);
  st7735_write_data(0x2b);
  st7735_write_data(0x39);
  st7735_write_data(0x00);
  st7735_write_data(0x01);
  st7735_write_data(0x03);
  st7735_write_data(0x10);

  st7735_write_command(ST7735_GMCTRN1);
  st7735_write_data(0x03);
  st7735_write_data(0x1d);
  st7735_write_data(0x07);
  st7735_write_data(0x06);
  st7735_write_data(0x2e);
  st7735_write_data(0x2c);
  st7735_write_data(0x29);
  st7735_write_data(0x2d);
  st7735_write_data(0x2e);
  st7735_write_data(0x2e);
  st7735_write_data(0x37);
  st7735_write_data(0x3f);
  st7735_write_data(0x00);
  st7735_write_data(0x00);
  st7735_write_data(0x02);
  st7735_write_data(0x10);

  st7735_write_command(ST7735_NORON);
  HAL_Delay(10);
  st7735_write_command(ST7735_DISPON);
  HAL_Delay(20);
}

void st7735_set_region(uint8_t x_start, uint8_t y_start, uint8_t x_end,
                       uint8_t y_end) {
  const uint8_t offset_x = 1;
  const uint8_t offset_y = 26;
  st7735_write_command(ST7735_CASET);
  st7735_write_data(0x00);
  st7735_write_data(x_start + offset_x);
  st7735_write_data(0x00);
  st7735_write_data(x_end + offset_x);

  st7735_write_command(ST7735_RASET);
  st7735_write_data(0x00);
  st7735_write_data(y_start + offset_y);
  st7735_write_data(0x00);
  st7735_write_data(y_end + offset_y);
  st7735_write_command(ST7735_RAMWR);
}

void st7735_output_background(uint16_t color) {
  st7735_fill_rect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, color);
}

void st7735_output_pixel(uint16_t color) {
  st7735_send_byte((uint8_t)(color >> 8));
  st7735_send_byte((uint8_t)color);
}

void st7735_output_symbol(char symbol, uint8_t x, uint8_t y, uint16_t fg,
                          uint16_t bg, uint8_t scale) {
  if (symbol < ' ' || symbol > '~') symbol = '?';
  const uint8_t *glyph = SmallFont + (symbol - ' ') * FONT_BYTE_PER_SYMBOL;
  st7735_set_region(x, y, x + FONT_WIDTH_SYMBOL * scale - 1,
                    y + FONT_HEIGHT_SYMBOL * scale - 1);
  st7735_set_data_mode();
  for (uint8_t row = 0; row < FONT_BYTE_PER_SYMBOL; row++) {
    for (uint8_t ry = 0; ry < scale; ry++) {
      for (uint8_t bit = 0; bit < 8; bit++) {
        uint16_t color = (glyph[row] & (0x80 >> bit)) ? fg : bg;
        for (uint8_t rx = 0; rx < scale; rx++) st7735_output_pixel(color);
      }
    }
  }
}

void st7735_output_text(const char *text, uint8_t x, uint8_t y, uint16_t fg,
                        uint16_t bg, uint8_t scale) {
  while (*text != '\0' && x + FONT_WIDTH_SYMBOL * scale <= ST7735_WIDTH) {
    st7735_output_symbol(*text++, x, y, fg, bg, scale);
    x += FONT_WIDTH_SYMBOL * scale;
  }
}

void st7735_output_text_8x12(const char *text, uint8_t x, uint8_t y,
                             uint16_t color) {
  st7735_output_text(text, x, y, color, ST7735_BLACK, 1);
}

void st7735_output_text_16x24(const char *text, uint8_t x, uint8_t y,
                              uint16_t color) {
  st7735_output_text(text, x, y, color, ST7735_BLACK, 2);
}

void st7735_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                      uint16_t color) {
  st7735_set_region(x, y, x + w - 1, y + h - 1);
  st7735_set_data_mode();
  for (uint16_t i = 0; i < (uint16_t)w * h; i++) st7735_output_pixel(color);
}

void st7735_sleep(void) {
  st7735_write_command(ST7735_DISPOFF);
  st7735_write_command(ST7735_SLPIN);
}
