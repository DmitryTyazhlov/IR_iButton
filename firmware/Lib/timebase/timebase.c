#include "timebase.h"

void timebase_init(void) {
  __HAL_RCC_TIM22_CLK_ENABLE();
  TIM22->PSC = (SystemCoreClock / 1000000U) - 1U;
  TIM22->ARR = 0xFFFF;
  TIM22->EGR = TIM_EGR_UG;
  TIM22->CR1 = TIM_CR1_CEN;
}
