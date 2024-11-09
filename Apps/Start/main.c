#include "main.h"

#include "freertos.h"

int main(void) {
  system_init();

  freertos_init();
  osKernelStart();

  for (;;) {
    /* We should never get here */
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM7) {
    HAL_IncTick();
  } /* @type tick = ms */
}
