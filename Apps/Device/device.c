#include "device.h"

#include "AHRS_MiddleWare.h"
#include "cdc.h"
#include "freertos.h"
#include "main.h"

void device(void const *args) {
  (void)args;

  usb_cdc_init();

  uint32_t cnt = 0;

  /* Infinite device */
  for (;;) {
    uprintf("Hello~: %d, %f", cnt++, AHRS_sinf(0.52359877f));
    vTaskDelay(1000);
  }
}
