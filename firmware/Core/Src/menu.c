#include "menu.h"

#include "buttons.h"
#include "main.h"
#include "st7735.h"

#define SIZE_MENU 10
#define LINE_PER_PAGE 3
#define FOLDER_ROOTS 255
#define FOL 0
#define APP 1

uint8_t current_page = 0;

menu_t menu[] = {
    {255, 0, 0, 0, FOL, "roots"}, {0, 0, 0, 0, FOL, "Games"},
    {0, 0, 0, 0, FOL, "Utils"},   {0, 0, 0, 0, FOL, "Secrets"},
    {1, 0, 0, 0, FOL, "Snake"},   {1, 0, 0, 0, FOL, "Tetris"},
    {1, 0, 0, 0, FOL, "Dice"},    {2, 0, 0, 0, APP, "Blink"},
    {2, 0, 0, 0, FOL, "Biper"},   {3, 0, 0, 0, FOL, "(_|_)"},
};

void menu_view_list(uint8_t id_folder) {
  st7735_output_background(ST7735_BLACK);
  for (uint8_t i = 0; i < SIZE_MENU; i++) {
    if (menu[i].parrents_id == id_folder) {
      st7735_output_text_16x24(menu[i].name, 16,
                               menu[i].sequence_in_parrent * 24, ST7735_GREEN);
    }
  }
  st7735_output_text_16x24(">", 0, menu[id_folder].choosing_item * 24,
                           ST7735_GREEN);
}

void menu_init() {
  for (uint8_t i = 0; i < SIZE_MENU; i++) {
    menu[i].sequence_in_parrent = 0;
    menu[i].quantity_item = 0;
    menu[i].choosing_item = 0;
  }
  for (uint8_t i = 0; i < SIZE_MENU; i++) {
    if (menu[i].parrents_id == 255) continue;
    uint8_t parrent = menu[i].parrents_id;
    menu[parrent].quantity_item++;
    menu[i].sequence_in_parrent = menu[parrent].quantity_item - 1;
  }
}

void start_app(){
  st7735_output_background(ST7735_BLACK);
  st7735_output_text_16x24("BLINK!!!", 20, 24, ST7735_YELLOW);
  while(1){
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    HAL_Delay(500);
    if(request_button() == 3){
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, SET);
      break;
    }
  } 
}

void handler_ok() {
  // return;

  for (uint8_t i = 0; i < SIZE_MENU; i++) {
    if (i == current_page) continue;
    bool parr_id = (current_page == menu[i].parrents_id);
    bool choos_seq =
        (menu[current_page].choosing_item == menu[i].sequence_in_parrent);
    if (parr_id && choos_seq) {
      if(menu[i].type == FOL){
        current_page = i;
      } else {
        start_app();
      }
      break;
    }
  }

  menu_view_list(current_page);
}

void handler_up() {
  if (menu[current_page].choosing_item == 0) {
    menu[current_page].choosing_item = 2;
  } else {
    menu[current_page].choosing_item--;
  }
  menu_view_list(current_page);
}

void handler_down() {
  if (menu[current_page].choosing_item == 2) {
    menu[current_page].choosing_item = 0;
  } else {
    menu[current_page].choosing_item++;
  }
  menu_view_list(current_page);
}

void handler_back() {
  if (current_page == 0) {
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    HAL_PWR_EnterSTANDBYMode();
  }
  current_page = menu[current_page].parrents_id;
  menu_view_list(current_page);
}

void menu_run() {
  menu_init();
  menu_view_list(current_page);

  while (1) {
    switch (request_button()) {
      case 0:
        handler_ok();
        break;
      case 1:
        handler_up();
        break;
      case 2:
        handler_down();
        break;
      case 3:
        handler_back();
        break;
      default:
        break;
    }
  }
}
