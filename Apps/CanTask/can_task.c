#include "can_task.h"

#include "can.h"
#include "freertos.h"
#include "pid.h"
#include "type_def.h"

#define FRIC_SPEED 6000
#define FRIC_TARGET_STEP 80

static pid_t main_fric_pid = {0};
static pid_t sub_fric_pid = {0};
static pid_t trig_pid_speed = {0};
static pid_t trig_pid_angle = {0};
static bool_t fric_is_off = 1;

void can_task(void const *args) {
  (void)args;

  const motor_measure_t *main_fric_mot = getp_fric_motor(0);
  const motor_measure_t *sub_fric_mot = getp_fric_motor(1);

  // 拨弹盘 PID 初始化
  pid_init(&trig_pid_speed, TRIG_SPEED_KP, TRIG_SPEED_KI, TRIG_SPEED_KD, 0,
           TRIG_ANGLE_PID_MAXI, TRIG_ANGLE_PID_MAXO);
  pid_init(&trig_pid_angle, TRIG_ANGLE_KP, TRIG_ANGLE_KI, TRIG_ANGLE_KD, 0,
           TRIG_SPEED_PID_MAXI, TRIG_SPEED_PID_MAXO);

  // 摩擦轮 PID 初始化
  pid_init(&main_fric_pid, FRIC_MAIN_KP, FRIC_MAIN_KI, FRIC_MAIN_KD,
           FRIC_TARGET_STEP, FRIC_PID_MAXI, FRIC_PID_MAXO);
  pid_init(&sub_fric_pid, FRIC_SUB_KP, FRIC_SUB_KI, FRIC_SUB_KD,
           FRIC_TARGET_STEP, FRIC_PID_MAXI, FRIC_PID_MAXO);

  main_fric_pid.target = sub_fric_pid.target = 0;

  fric_is_off = 1;  // 默认摩擦轮关闭

  /* Infinite loop */
  for (;;) {
    pid_update(&main_fric_pid, main_fric_mot->speed_rpm);
    pid_update(&sub_fric_pid, sub_fric_mot->speed_rpm);

    if (fric_is_off) pid_clear(&main_fric_pid), pid_clear(&sub_fric_pid);
    can_fric_cmd((int16_t)main_fric_pid.output, (int16_t)sub_fric_pid.output);

    vTaskDelay(30);
  }
}

void can_fric_forward(void) {
  fric_is_off = 0;
  main_fric_pid.target = sub_fric_pid.target = FRIC_SPEED;
}

void can_fric_reverse(void) {
  fric_is_off = 0;
  main_fric_pid.target = sub_fric_pid.target = -FRIC_SPEED;
}

void can_fric_off(void) {
  fric_is_off = 1;
  main_fric_pid.target = sub_fric_pid.target = 0;
  pid_clear(&main_fric_pid), pid_clear(&sub_fric_pid);
}

void pid_get(pid_t **pid_left, pid_t **pid_right) {
  *pid_left = &main_fric_pid;
  *pid_right = &sub_fric_pid;
}
