#include "loop_task.h"

#include <stdint.h>

#include "can.h"
#include "can_task.h"
#include "freertos.h"
#include "pid.h"
#include "remtctrl.h"
#include "type_def.h"

void pid_get(pid_t **pid_left, pid_t **pid_right);

void loop_task(void const *args) {
  (void)args;

  const remtctrl_t *remote = getp_remtctrl();
  const motor_measure_t *frig_left = getp_mot_fric(0);
  const motor_measure_t *frig_right = getp_mot_fric(1);

  uint32_t len = 0;
  uint8_t *buf = NULL;
  pid_t *pid_left = NULL;
  pid_t *pid_right = NULL;

  pid_get(&pid_left, &pid_right);

  /* Infinite loop */
  for (;;) {
    buf = usb_bufget(&len);
    if (len == 4) {
      pid_left->kp = pid_right->kp = (fp32)buf[0] / 10;
      pid_left->ki = pid_right->ki = (fp32)buf[1] / 10;
      pid_left->kd = pid_right->kd = (fp32)buf[2] / 10;
      pid_left->ks = pid_right->ks = (fp32)buf[3] * 10;
    }

    if (remote->rc.ch[0] > 440) {  // > 330
      can_fric_forward();
    } else if (remote->rc.ch[0] < -440) {  // < -330
      can_fric_reverse();
    } else {
      can_fric_off();
    }

    uprintf(
        "left: %d;;;left_v: %d, right_v:%d;;; P: %f, I: %f, D: %f, S: %f\r\n ",
        remote->rc.ch[0], frig_left->speed_rpm, frig_right->speed_rpm,
        pid_left->kp, pid_left->ki, pid_left->kd, pid_left->ks);
    // uprintf("loop\r\n");

    vTaskDelay(30);
  }
}
