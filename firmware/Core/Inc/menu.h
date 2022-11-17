#ifndef _MENU_H_
#define _MENU_H_
#include "stdbool.h"
#include "stdint.h"


typedef struct {
  const uint8_t parrents_id;
  uint8_t sequence_in_parrent;
  uint8_t quantity_item;
  uint8_t choosing_item;
  const uint8_t type;
  const uint8_t name[11];
} menu_t;

extern menu_t menu[];

void menu_run();

#endif