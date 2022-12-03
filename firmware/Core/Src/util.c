#include "util.h"
#include "core_cm3.h"

void tick_counter_init(){
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

inline uint32_t get_tick(){
  return DWT->CYCCNT;
}
