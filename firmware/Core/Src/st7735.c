#include "st7735.h"

#include "font.h"
#include "gpio.h"
#include "main.h"
#include "spi.h"
#include "stdbool.h"

inline void st7735_reset(bool x) {
  if (x) {
    HAL_GPIO_WritePin(DISP_RES_GPIO_Port, DISP_RES_Pin, RESET);
  } else {
    HAL_GPIO_WritePin(DISP_RES_GPIO_Port, DISP_RES_Pin, SET);
  }
}

inline void st7735_chip_select(bool x) {
  if (x) {
    HAL_GPIO_WritePin(DISP_CS_GPIO_Port, DISP_CS_Pin, RESET);
  } else {
    HAL_GPIO_WritePin(DISP_CS_GPIO_Port, DISP_CS_Pin, SET);
  }
}

inline void st7735_set_command_mode() {
  HAL_GPIO_WritePin(DISP_DC_GPIO_Port, DISP_DC_Pin, RESET);
}

inline void st7735_set_data_mode() {
  HAL_GPIO_WritePin(DISP_DC_GPIO_Port, DISP_DC_Pin, SET);
}

inline void st7735_send_byte(uint8_t data) {
  while (!(SPI1->SR & SPI_SR_TXE))
    ;
  SPI1->DR = data;
  while (!(SPI1->SR & SPI_SR_RXNE))
    ;
}

inline void st7735_write_command(uint8_t data) {
  st7735_set_command_mode();
  st7735_send_byte(data);
}

inline void st7735_write_data(uint8_t data) {
  st7735_set_data_mode();
  st7735_send_byte(data);
}

void st7735_disp_init(void) {
  st7735_chip_select(false);
  st7735_reset(true);
  HAL_Delay(1); //
  st7735_reset(false);
  st7735_chip_select(true);
  HAL_Delay(1);

  st7735_write_command(ST7735_SWRESET);
  HAL_Delay(1);//
  st7735_write_command(ST7735_SLPOUT);
  HAL_Delay(1);//

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
  HAL_Delay(1);//
  st7735_write_command(ST7735_DISPON);
  HAL_Delay(1);//
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
  uint8_t first_byte = (uint8_t)(color >> 8);
  uint8_t second_byte = (uint8_t)color;
  st7735_set_region(0, 0, 159, 79);
  st7735_set_data_mode();
  for (uint32_t x = 0; x < 12800; x++) {
    st7735_send_byte(first_byte);
    st7735_send_byte(second_byte);
  }
}

void st7735_output_pixel(uint16_t color) {
  uint8_t first_byte = (uint8_t)(color >> 8);
  uint8_t second_byte = (uint8_t)color;

  st7735_write_data(first_byte);
  st7735_write_data(second_byte);
}

void st7735_output_symbol_8x12(uint8_t symbol, uint8_t x, uint8_t y,
                               uint16_t color) {
  st7735_set_region(x, y, x + FONT_WIDTH_SYMBOL - 1,
                    y + FONT_HEIGHT_SYMBOL - 1);
  symbol = symbol - ' ';
  for (uint8_t i = 0; i < FONT_BYTE_PER_SYMBOL; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      if (*(SmallFont + i + (symbol * FONT_BYTE_PER_SYMBOL)) & (0x80 >> j)) {
        st7735_output_pixel(color);
      } else {
        st7735_output_pixel(COLOR_BACKGROUND);
      }
    }
  }
}

void st7735_output_text_8x12(uint8_t *text, uint8_t x, uint8_t y,
                             uint16_t color) {
  uint32_t i = 0;
  while (*(text + i) != '\0') {
    st7735_output_symbol_8x12(*(text + i), x + i * FONT_WIDTH_SYMBOL, y, color);
    i++;
  }
}

void st7735_output_symbol_16x24(uint8_t symbol, uint8_t x, uint8_t y,
                                uint16_t color) {
  st7735_set_region(x, y, x + FONT_WIDTH_SYMBOL * 2 - 1,
                    y + FONT_HEIGHT_SYMBOL * 2 - 1);
  symbol = symbol - ' ';
  for (uint8_t i = 0; i < FONT_BYTE_PER_SYMBOL; i++) {
    for (uint8_t b = 0; b < 2; b++) {
      for (uint8_t j = 0; j < 8; j++) {
        for (uint8_t a = 0; a < 2; a++) {
          if (*(SmallFont + i + (symbol * FONT_BYTE_PER_SYMBOL)) &
              (0x80 >> j)) {
            st7735_output_pixel(color);
          } else {
            st7735_output_pixel(COLOR_BACKGROUND);
          }
        }
      }
    }
  }
}

void st7735_output_text_16x24(uint8_t *text, uint8_t x, uint8_t y,
                              uint16_t color) {
  uint32_t i = 0;
  while (*(text + i) != '\0') {
    st7735_output_symbol_16x24(*(text + i), x + i * FONT_WIDTH_SYMBOL * 2, y,
                               color);
    i++;
  }
}