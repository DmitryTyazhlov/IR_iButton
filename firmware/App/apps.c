#include "apps.h"

#include <stdbool.h>

#include "buttons.h"
#include "ibutton.h"
#include "ir.h"
#include "max17043.h"
#include "main.h"
#include "power.h"
#include "ui.h"

static ibutton_key_t saved_key;
static bool has_saved_key;

static ir_frame_t saved_frame;
static bool has_saved_frame;
static uint8_t nec_address;
static uint8_t nec_command;

static void show_key(const ibutton_key_t *key) {
  char half[9];
  ui_hex(half, key->id, 4);
  st7735_output_text_16x24(half, 0, 16, ST7735_GREEN);
  ui_hex(half, key->id + 4, 4);
  st7735_output_text_16x24(half, 0, 42, ST7735_GREEN);
}

static void toggle_led(void) { HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); }

static void led_off(void) { HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); }

/* ---- iButton -------------------------------------------------------- */

void app_ibutton_read(void) {
  ui_screen("iButton: read");
  ui_row(2, "Touch the key to", ST7735_WHITE);
  ui_row(3, "the reader", ST7735_WHITE);
  ui_row(5, "BACK: exit", ST7735_YELLOW);

  uint8_t shown = 0;
  while (1) {
    button_t b = buttons_get();
    if (b == BTN_BACK) return;

    ibutton_key_t key;
    ibutton_status_t status = ibutton_read(&key);
    if (status == IBUTTON_OK) {
      if (!has_saved_key || shown != 1) {
        saved_key = key;
        has_saved_key = true;
        ui_screen("Key saved");
        show_key(&key);
        ui_row(5, "BACK: exit", ST7735_YELLOW);
        shown = 1;
      }
    } else if (status == IBUTTON_CRC_ERROR && shown != 2) {
      ui_screen("iButton: read");
      ui_row(2, "CRC error,", ST7735_RED);
      ui_row(3, "retry contact", ST7735_RED);
      shown = 2;
    } else if (status == IBUTTON_NO_DEVICE && shown != 0) {
      ui_screen("iButton: read");
      ui_row(2, "Touch the key to", ST7735_WHITE);
      ui_row(3, "the reader", ST7735_WHITE);
      ui_row(5, "BACK: exit", ST7735_YELLOW);
      shown = 0;
    }
    HAL_Delay(100);
  }
}

void app_ibutton_saved(void) {
  ui_screen("Saved key");
  if (has_saved_key) {
    show_key(&saved_key);
  } else {
    ui_row(2, "Nothing saved", ST7735_RED);
    ui_row(3, "Read a key first", ST7735_WHITE);
  }
  while (buttons_wait() != BTN_BACK) {
  }
}

void app_ibutton_emulate(void) {
  ui_screen("iButton: emulate");
  if (!has_saved_key) {
    ui_row(2, "No saved key", ST7735_RED);
    ui_row(3, "Read a key first", ST7735_WHITE);
    while (buttons_wait() != BTN_BACK) {
    }
    return;
  }
  show_key(&saved_key);
  ui_row(5, "Reads: 0", ST7735_YELLOW);

  uint16_t reads = 0;
  while (buttons_get() != BTN_BACK) {
    if (ibutton_emulate_poll(&saved_key)) {
      reads++;
      char line[16] = "Reads: ";
      char digits[6];
      uint8_t n = 0;
      for (uint16_t v = reads; v > 0; v /= 10) digits[n++] = '0' + v % 10;
      for (uint8_t i = 0; i < n; i++) line[7 + i] = digits[n - 1 - i];
      line[7 + n] = '\0';
      ui_row(5, line, ST7735_YELLOW);
      toggle_led();
    }
  }
  led_off();
}

/* ---- IR ------------------------------------------------------------- */

static void show_decoded(const ir_frame_t *frame) {
  ir_decoded_t d = ir_decode(frame);
  char line[24];
  if (d.proto == IR_PROTO_NEC) {
    uint8_t bytes[2] = {ir_nec_address(d.data), ir_nec_command(d.data)};
    ui_row(1, "NEC", ST7735_GREEN);
    ui_hex(line, bytes, 1);
    st7735_output_text_16x24("A:", 0, 16, ST7735_WHITE);
    st7735_output_text_16x24(line, 32, 16, ST7735_GREEN);
    ui_hex(line, bytes + 1, 1);
    st7735_output_text_16x24("C:", 0, 42, ST7735_WHITE);
    st7735_output_text_16x24(line, 32, 42, ST7735_GREEN);
    nec_address = bytes[0];
    nec_command = bytes[1];
  } else if (d.proto == IR_PROTO_NEC_REPEAT) {
    ui_row(1, "NEC repeat", ST7735_YELLOW);
  } else {
    ui_row(1, "Raw signal", ST7735_YELLOW);
    char digits[6];
    uint8_t n = 0;
    for (uint16_t v = frame->len; v > 0; v /= 10) digits[n++] = '0' + v % 10;
    for (uint8_t i = 0; i < n; i++) line[i] = digits[n - 1 - i];
    line[n] = '\0';
    st7735_output_text_16x24(line, 0, 16, ST7735_GREEN);
    ui_row(5, "pulses", ST7735_WHITE);
  }
}

void app_ir_receive(void) {
  ui_screen("IR: receive");
  ui_row(2, "Point a remote", ST7735_WHITE);
  ui_row(3, "at the sensor", ST7735_WHITE);
  ui_row(5, "BACK: exit", ST7735_YELLOW);
  ir_rx_start();

  ir_frame_t frame;
  while (buttons_get() != BTN_BACK) {
    if (!ir_rx_poll(&frame)) continue;
    saved_frame = frame;
    has_saved_frame = true;
    ui_screen("IR: received");
    show_decoded(&frame);
    ui_row(5, "BACK:exit OK:next", ST7735_YELLOW);
    toggle_led();
    button_t b;
    while ((b = buttons_wait()) != BTN_OK && b != BTN_BACK) {
    }
    led_off();
    if (b == BTN_BACK) break;
    ui_screen("IR: receive");
    ui_row(2, "Point a remote", ST7735_WHITE);
    ui_row(3, "at the sensor", ST7735_WHITE);
    ui_row(5, "BACK: exit", ST7735_YELLOW);
    ir_rx_start();
  }
  ir_rx_stop();
}

void app_ir_replay(void) {
  ui_screen("IR: replay");
  if (!has_saved_frame) {
    ui_row(2, "Nothing saved", ST7735_RED);
    ui_row(3, "Receive first", ST7735_WHITE);
    while (buttons_wait() != BTN_BACK) {
    }
    return;
  }
  show_decoded(&saved_frame);
  ui_row(5, "OK:send BACK:exit", ST7735_YELLOW);
  while (1) {
    button_t b = buttons_wait();
    if (b == BTN_BACK) return;
    if (b != BTN_OK) continue;
    ir_decoded_t d = ir_decode(&saved_frame);
    if (d.proto == IR_PROTO_NEC) {
      ir_send_nec(ir_nec_address(d.data), ir_nec_command(d.data));
    } else {
      ir_send_frame(&saved_frame);
    }
    toggle_led();
    HAL_Delay(50);
    led_off();
  }
}

static void draw_nec(void) {
  char hex[3];
  ui_screen("IR: send NEC");
  ui_hex(hex, &nec_address, 1);
  st7735_output_text_16x24("A:", 0, 16, ST7735_WHITE);
  st7735_output_text_16x24(hex, 32, 16, ST7735_GREEN);
  ui_hex(hex, &nec_command, 1);
  st7735_output_text_16x24("C:", 0, 42, ST7735_WHITE);
  st7735_output_text_16x24(hex, 32, 42, ST7735_GREEN);
  ui_row(5, "UP/DN:cmd OK:send", ST7735_YELLOW);
}

void app_ir_send_nec(void) {
  draw_nec();
  while (1) {
    switch (buttons_wait()) {
      case BTN_UP:
        nec_command++;
        draw_nec();
        break;
      case BTN_DOWN:
        nec_command--;
        draw_nec();
        break;
      case BTN_OK:
        ir_send_nec(nec_address, nec_command);
        toggle_led();
        HAL_Delay(50);
        led_off();
        break;
      case BTN_BACK:
        return;
      default:
        break;
    }
  }
}

/* ---- Battery -------------------------------------------------------- */

static uint8_t put_number(char *out, uint16_t value) {
  char digits[6];
  uint8_t n = 0;
  do {
    digits[n++] = '0' + value % 10;
    value /= 10;
  } while (value > 0);
  for (uint8_t i = 0; i < n; i++) out[i] = digits[n - 1 - i];
  return n;
}

static void draw_battery(uint16_t mv, uint8_t percent) {
  char line[12];
  uint8_t n = put_number(line, mv / 1000);
  line[n++] = '.';
  line[n++] = '0' + (mv / 100) % 10;
  line[n++] = '0' + (mv / 10) % 10;
  line[n++] = 'V';
  line[n] = '\0';
  st7735_output_text_16x24(line, 0, 16, ST7735_GREEN);

  n = put_number(line, percent);
  line[n++] = '%';
  line[n++] = ' ';
  line[n] = '\0';
  st7735_output_text_16x24(line, 0, 42, ST7735_GREEN);

  /* charge bar on the right of the percentage */
  const uint8_t x = 70, w = 84, h = 16, y = 46;
  uint8_t filled = (uint16_t)w * percent / 100;
  uint16_t color = percent > 20 ? ST7735_GREEN : ST7735_RED;
  st7735_fill_rect(x, y, filled, h, color);
  st7735_fill_rect(x + filled, y, w - filled, h, ST7735_BLACK);
}

void app_battery(void) {
  ui_screen("Battery");
  if (!max17043_init()) {
    uint8_t levels = max17043_bus_levels();
    uint8_t found[4];
    uint8_t count = max17043_scan(found, sizeof(found));
    uint16_t err = max17043_last_error();
    uint8_t err_bytes[2] = {err >> 8, err & 0xFF};
    char line[24] = "SCL=. SDA=.";
    line[4] = (levels & 1) ? '1' : '0';
    line[10] = (levels & 2) ? '1' : '0';
    ui_row(1, "Gauge not found", ST7735_RED);
    ui_row(2, line, ST7735_WHITE);
    char err_line[12] = "err=0x";
    ui_hex(err_line + 6, err_bytes, 2);
    ui_row(3, err_line, ST7735_WHITE);
    char list[24] = "found: ";
    uint8_t pos = 7;
    if (count == 0) {
      list[pos++] = '-';
    }
    for (uint8_t i = 0; i < count; i++) {
      ui_hex(list + pos, &found[i], 1);
      pos += 2;
      list[pos++] = ' ';
    }
    list[pos] = '\0';
    ui_row(4, list, ST7735_WHITE);
    while (buttons_wait() != BTN_BACK) {
    }
    return;
  }
  {
    uint8_t info[3] = {max17043_address(), max17043_version() >> 8,
                       max17043_version() & 0xFF};
    char line[24] = "Bat A:";
    ui_hex(line + 6, info, 1);
    line[8] = ' ';
    line[9] = 'V';
    line[10] = ':';
    ui_hex(line + 11, info + 1, 2);
    ui_row(0, line, ST7735_CYAN);
  }

  while (1) {
    uint16_t mv;
    uint8_t percent;
    if (max17043_read_mv(&mv) && max17043_read_percent(&percent)) {
      draw_battery(mv, percent);
      uint16_t raw = max17043_vcell_raw();
      uint8_t raw_bytes[2] = {raw >> 8, raw & 0xFF};
      char line[24] = "raw=";
      ui_hex(line + 4, raw_bytes, 2);
      ui_row(5, line, ST7735_YELLOW);
    } else {
      ui_row(2, "Read error", ST7735_RED);
    }
    /* refresh once a second, but react to BACK at once */
    for (uint8_t i = 0; i < 10; i++) {
      if (buttons_get() == BTN_BACK) return;
      HAL_Delay(100);
    }
  }
}

/* ---- Settings ------------------------------------------------------- */

void app_led_test(void) {
  ui_screen("LED test");
  ui_row(2, "Blinking...", ST7735_YELLOW);
  ui_row(5, "BACK: exit", ST7735_YELLOW);
  while (buttons_get() != BTN_BACK) {
    toggle_led();
    HAL_Delay(250);
  }
  led_off();
}

void app_sleep(void) { power_sleep(); }
