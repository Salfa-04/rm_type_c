#include "loop_task.h"

#include "can.h"
#include "freertos.h"

void loop_task(void const *args) {
  (void)args;

  can_init();

  const motor_measure_t *motor = getp_fric_motor(0);

  /* Infinite loop */
  for (;;) {
    uprintf("ecd=%d,speed=%d\n", motor->ecd, motor->speed_rpm);
    vTaskDelay(30);
  }
}
