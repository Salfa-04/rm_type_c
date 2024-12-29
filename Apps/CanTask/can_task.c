#include "can_task.h"

#include "can.h"
#include "freertos.h"
#include "pid.h"
#include "type_def.h"

static pid_t pid_fric_m = {0};
static pid_t pid_fric_s = {0};
static pid_t pid_trig_s = {0};
static pid_t pid_trig_a = {0};

/******************* PIDs 参数 ********************/

const fp32 pids_trig_s[4] = {
    TRIG_KP,
    TRIG_KI,
    TRIG_KD,
    FRIC_KS,
};

const fp32 pids_trig_a[4] = {
    TRIG_KP,
    TRIG_KI,
    TRIG_KD,
    FRIC_KS,
};

const fp32 pids_fric_m[4] = {
    FRIC_MAIN_KP,
    FRIC_MAIN_KI,
    FRIC_MAIN_KD,
    0,
};

const fp32 pids_fric_s[4] = {
    FRIC_SUB_KP,
    FRIC_SUB_KI,
    FRIC_SUB_KD,
    0,
};

const fp32 pids_trig_io[2] = {TRIG_MAXI, TRIG_MAXO};
const fp32 pids_fric_io[2] = {FRIC_MAXI, FRIC_MAXO};

/******************* CAN Task Start ******************* */

/// 1: 自由模式, 2: 锁定模式, 3: 退弹模式, 4: ......
static uint8_t trig_flag = 0;

void can_task(void const *args) {
  (void)args;

  const motor_measure_t *mot_fric_m = getp_mot_fric(0);
  const motor_measure_t *mot_fric_s = getp_mot_fric(1);
  const motor_measure_t *mot_trig = getp_mot_trig();

  // PIDS 初始化
  pid_init(&pid_trig_s, pids_trig_s, pids_trig_io);
  pid_init(&pid_fric_m, pids_fric_m, pids_fric_io);
  pid_init(&pid_fric_s, pids_fric_s, pids_fric_io);

  pid_fric_m.target = pid_fric_s.target = 0;
  pid_trig_s.target = 0;

  /* Infinite loop */
  for (;;) {
    TickType_t nowtick = xTaskGetTickCount();

    /******************* 摩擦轮 ********************/
    pid_update(&pid_fric_m, mot_fric_m->speed_rpm);
    pid_update(&pid_fric_s, mot_fric_s->speed_rpm);

    /******************* 拨弹盘 ********************/
    pid_update(&pid_trig_s, mot_trig->speed_rpm);
    if (trig_flag == 1) pid_clear(&pid_trig_s);

    /***************** 云台抬头电机 ******************/

    can_high_cmd(0, (int16_t)pid_fric_m.output, (int16_t)pid_fric_s.output);

    vTaskDelayUntil(&nowtick, 100);
  }
}

void pid_get(pid_t **pid_left, pid_t **pid_right) {
  *pid_left = &pid_fric_m;
  *pid_right = &pid_fric_s;
}

/******************* 摩擦轮控制 ********************/

#define FRIC_TARGET_SPEED 6000

void can_fric_forward(void) {
  pid_set(&pid_fric_m, FRIC_TARGET_SPEED);
  pid_set(&pid_fric_s, FRIC_TARGET_SPEED);
}

void can_fric_reverse(void) {
  pid_set(&pid_fric_m, -FRIC_TARGET_SPEED);
  pid_set(&pid_fric_s, -FRIC_TARGET_SPEED);
}

void can_fric_off(void) {
  pid_set(&pid_fric_m, 0);
  pid_set(&pid_fric_s, 0);
}

/******************* 拨弹盘控制 ********************/

void can_trig_on(void) { trig_flag = 0; }

//! TODO: 加入控制位置的模式, 注意边缘变化的编码值
void can_trig_hold(void) { trig_flag = 2; }

void can_trig_off(void) {
  trig_flag = 0;
  pid_set(&pid_trig_s, 0);
}

void can_trig_free(void) {
  trig_flag = 1;
  pid_set(&pid_trig_s, 0);
}

//! TODO: 加入退弹的模式, 电机反向转 x编码值
void can_trig_back(void) { trig_flag = 3; }
