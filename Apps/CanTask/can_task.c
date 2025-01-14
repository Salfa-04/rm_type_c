#include "can_task.h"

#include "can.h"
#include "freertos.h"
#include "pid.h"
#include "remtctrl.h"
#include "type_def.h"

/// 拨弹盘电机编码值
static int64_t trig_ecd_v = 0;

/// 主副摩擦轮
static pid_t pid_fric_m = {0};
static pid_t pid_fric_s = {0};
/// 拨弹盘速度和角度环
static pid_t pid_trig_v = {0};
static pid_t pid_trig_a = {0};

/******************* CAN Task Start ******************* */

/// [`trig_speed_flag`] =
///     0 : Speed Mode
///     1 : Angle Mode
static bool_t trig_speed_flag = 0;

void can_task(void const *args) {
  (void)args;

  const remtctrl_t *ctrl = getp_remtctrl();

  const motor_measure_t *mot_fric_m = getp_mot_fric(0);
  const motor_measure_t *mot_fric_s = getp_mot_fric(1);
  const motor_measure_t *mot_trig = getp_mot_trig();

  {  // PIDS 初始化

    const fp32 pids_fric_m[6] = {
        FRIC_KP, FRIC_KI, 0, 0, FRIC_MX, FRIC_MX,
    };

    const fp32 pids_fric_s[6] = {
        FRIC_KP, FRIC_KI, 0, 0, FRIC_MX, FRIC_MX,
    };

    const fp32 pids_trig_v[6] = {
        TRIG_KP_V, TRIG_KI_V, TRIG_KD_V, TRIG_KS, TRIG_MX, TRIG_MX,
    };

    const fp32 pids_trig_a[6] = {
        TRIG_KP_A, TRIG_KI_A, TRIG_KD_A, TRIG_KS, TRIG_MX, TRIG_MX,
    };

    pid_init(&pid_fric_m, pids_fric_m, pids_fric_m + 4);
    pid_init(&pid_fric_s, pids_fric_s, pids_fric_s + 4);

    pid_init(&pid_trig_v, pids_trig_v, pids_trig_v + 4);
    pid_init(&pid_trig_a, pids_trig_a, pids_trig_a + 4);

    pid_fric_m.target = pid_fric_s.target = 0;
    pid_trig_v.target = pid_trig_a.target = 0;
  }

  /* Infinite loop */
  for (;;) {
    TickType_t nowtick = xTaskGetTickCount();

    if (!switch_is_down(ctrl->rc.s[0])) {  /// 非无力模式
      /******************* 摩擦轮 ********************/

      pid_update(&pid_fric_m, mot_fric_m->speed_rpm);
      pid_update(&pid_fric_s, mot_fric_s->speed_rpm);

      /******************* 拨弹盘 ********************/

      {  /// 更新拨弹盘编码值 [`trig_ecd_v`]
        int16_t deata_ecdv = mot_trig->ecd - mot_trig->last_ecd;

        trig_ecd_v += deata_ecdv;
        if (deata_ecdv > 4096) trig_ecd_v -= 8192;
        if (deata_ecdv < -4096) trig_ecd_v += 8192;
      }

      if (trig_speed_flag) {  // Speed Mode
        pid_update(&pid_trig_v, mot_trig->speed_rpm);
        if (pid_fric_s.target == 0) pid_clear(&pid_trig_v);
      } else {  // Angle Mode
        pid_update(&pid_trig_a, trig_ecd_v);
        pid_update(&pid_trig_v, pid_trig_a.output);
      }

      /***************** 云台抬头电机 ******************/

      can_high_cmd(0, (int16_t)pid_fric_m.output, (int16_t)pid_fric_s.output);

    } else {  /// 无力模式
      can_high_cmd(0, 0, 0);
      can_mid_cmd(0, 0);
      can_low_cmd(0, 0, 0, 0);
    }

    vTaskDelayUntil(&nowtick, 100);
  }
}

#ifdef DEBUG

void pid_get(pid_t **pid_left, pid_t **pid_right) {
  *pid_left = &pid_fric_m;
  *pid_right = &pid_fric_s;
}

#endif

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

//! TODO: 重新计算速度, 注意电机为3508, 减速比为19, 连续编码值为8192
#define TRIG_TARGET_SPEED 60  // 此处为正转速度, 注意正负
#define TRIG_BACK_ECD_V 60    // 退弹角度, 应转换为编码值, 注意正负

/// [`trig_speed_flag`] =
///   0 : Speed Mode
///   1 : Angle Mode

/// 拨弹盘正转(发射)
void can_trig_on(void) {
  trig_speed_flag = true;  // Normal
  pid_set(&pid_trig_v, TRIG_TARGET_SPEED);
}

/// 拨弹盘锁定
void can_trig_hold(void) {
  trig_speed_flag = false;
  pid_set(&pid_trig_a, trig_ecd_v);
}

//! TODO: 加入退弹的模式, 电机反向转至少45度
/// 拨弹盘退弹
void can_trig_back(void) {
  trig_speed_flag = false;
  pid_set(&pid_trig_a, trig_ecd_v - TRIG_BACK_ECD_V);
}
