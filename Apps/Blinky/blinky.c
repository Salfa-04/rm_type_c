#include "blinky.h"

#include <stdint.h>

#include "cmsis_os.h"
#include "usb.h"

uint8_t buffer[] = "我草泥马";

void blinky(void const *args) {
  (void)args;

  usb_device_init();

  /* Infinite loop */
  for (;;) {
    for (uint8_t i = 0; i < 3; i++) {
      HAL_GPIO_TogglePin(GPIOH, 1 << (10 + i));
      osDelay(300);
    }

    CDC_Transmit_FS(buffer, sizeof(buffer));
  }
}
