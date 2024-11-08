#include "blinky.h"

#include "cmsis_os.h"

void blinky(void const *args) {
  (void)args;

  /* Infinite loop */
  for (;;) {
    for (uint8_t i = 0; i < 3; i++) {
      HAL_GPIO_TogglePin(GPIOH, 1 << (10 + i));
      osDelay(300);
    }
  }
}
