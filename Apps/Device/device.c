#include "device.h"

#include "cdc.h"
#include "freertos.h"
#include "ins.h"
#include "main.h"
#include "type_def.h"

void device(void const *args) {
  (void)args;

  usb_cdc_init();

  vTaskDelay(600);
  const fp32 *angle = get_INS_angle_point();

  uint32_t cnt = 0;

  /* Infinite device */
  for (;;) {
    uprintf("x=%f,y=%f,z=%f\n", cnt++, angle[0], angle[1], angle[2]);
    vTaskDelay(10);
  }
}
