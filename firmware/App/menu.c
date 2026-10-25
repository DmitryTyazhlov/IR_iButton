#include "menu.h"

#include <stdbool.h>
#include <stddef.h>

#include "apps.h"
#include "buttons.h"
#include "power.h"
#include "st7735.h"

#define VISIBLE_ITEMS 3
#define ITEM_HEIGHT 24
#define MAX_DEPTH 4
#define NAME_MAX_CHARS 9

struct menu;

typedef struct {
  const char *name;
  const struct menu *submenu; /* open this menu, or... */
  void (*action)(void);       /* ...run this app */
} menu_item_t;

typedef struct menu {
  const menu_item_t *items;
  uint8_t count;
} menu_t;

#define MENU(name, ...)                                           \
  static const menu_item_t name##_items[] = {__VA_ARGS__};        \
  static const menu_t name = {name##_items,                       \
                              sizeof(name##_items) / sizeof(menu_item_t)}

MENU(ibutton_menu, {"Read", NULL, app_ibutton_read},
     {"Emulate", NULL, app_ibutton_emulate},
     {"Saved ID", NULL, app_ibutton_saved});

MENU(ir_menu, {"Receive", NULL, app_ir_receive},
     {"Replay", NULL, app_ir_replay}, {"Send NEC", NULL, app_ir_send_nec});

MENU(settings_menu, {"LED test", NULL, app_led_test},
     {"Sleep", NULL, app_sleep});

MENU(root_menu, {"iButton", &ibutton_menu, NULL}, {"IR", &ir_menu, NULL},
     {"Battery", NULL, app_battery}, {"Settings", &settings_menu, NULL});

typedef struct {
  const menu_t *menu;
  uint8_t cursor;
  uint8_t top;
} level_t;

static level_t stack[MAX_DEPTH];
static uint8_t depth;

static void draw(void) {
  const level_t *level = &stack[depth];
  st7735_output_background(ST7735_BLACK);

  for (uint8_t row = 0; row < VISIBLE_ITEMS; row++) {
    uint8_t index = level->top + row;
    if (index >= level->menu->count) break;

    bool selected = (index == level->cursor);
    uint16_t fg = selected ? ST7735_BLACK : ST7735_GREEN;
    uint16_t bg = selected ? ST7735_GREEN : ST7735_BLACK;

    char name[NAME_MAX_CHARS + 1] = {0};
    const char *src = level->menu->items[index].name;
    for (uint8_t i = 0; i < NAME_MAX_CHARS; i++) {
      name[i] = (*src != '\0') ? *src++ : ' ';
    }
    st7735_output_text(name, 2, row * ITEM_HEIGHT, fg, bg, 2);
  }

  /* scroll bar */
  if (level->menu->count > VISIBLE_ITEMS) {
    uint8_t bar_h = ST7735_HEIGHT * VISIBLE_ITEMS / level->menu->count;
    uint8_t bar_y = ST7735_HEIGHT * level->top / level->menu->count;
    st7735_fill_rect(ST7735_WIDTH - 4, bar_y, 3, bar_h, ST7735_GREEN);
  }
}

static void move(int8_t step) {
  level_t *level = &stack[depth];
  uint8_t count = level->menu->count;
  level->cursor = (level->cursor + count + step) % count;
  if (level->cursor < level->top) level->top = level->cursor;
  if (level->cursor >= level->top + VISIBLE_ITEMS) {
    level->top = level->cursor - VISIBLE_ITEMS + 1;
  }
  /* wrapping from the last item to the first (or back) */
  if (level->cursor == 0) level->top = 0;
  if (level->cursor == count - 1 && count > VISIBLE_ITEMS) {
    level->top = count - VISIBLE_ITEMS;
  }
}

static void enter(void) {
  const level_t *level = &stack[depth];
  const menu_item_t *item = &level->menu->items[level->cursor];
  if (item->submenu != NULL) {
    if (depth + 1 < MAX_DEPTH) {
      stack[++depth] = (level_t){item->submenu, 0, 0};
    }
  } else if (item->action != NULL) {
    item->action();
  }
}

void menu_run(void) {
  depth = 0;
  stack[0] = (level_t){&root_menu, 0, 0};
  draw();

  while (1) {
    switch (buttons_wait()) {
      case BTN_UP:
        move(-1);
        break;
      case BTN_DOWN:
        move(1);
        break;
      case BTN_OK:
        enter();
        break;
      case BTN_BACK:
        if (depth == 0) {
          power_sleep();
        } else {
          depth--;
        }
        break;
      default:
        break;
    }
    draw();
  }
}
