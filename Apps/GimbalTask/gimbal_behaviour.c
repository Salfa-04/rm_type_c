#include "gimbal_behaviour.h"

#include "arm_math.h"
#include "user_lib.h"

#define int_abs(x) abs(x)

#define rc_deadband_limit(input, output, dealine)      \
  {                                                    \
    if ((input) > (dealine) || (input) < -(dealine)) { \
      (output) = (input);                              \
    } else {                                           \
      (output) = 0;                                    \
    }                                                  \
  }

/**
 * @brief          通过判断角速度来判断云台是否到达极限位置
 * @param          对应轴的角速度，单位rad/s
 * @param          计时时间，到达GIMBAL_CALI_STEP_TIME的时间后归零
 * @param          记录的角度 rad
 * @param          反馈的角度 rad
 * @param          记录的编码值 raw
 * @param          反馈的编码值 raw
 * @param          校准的步骤 完成一次 加一
 */
#define gimbal_cali_gyro_judge(gyro, cmd_time, angle_set, angle, ecd_set, ecd, \
                               step)                                           \
  {                                                                            \
    if ((gyro) < GIMBAL_CALI_GYRO_LIMIT) {                                     \
      (cmd_time)++;                                                            \
      if ((cmd_time) > GIMBAL_CALI_STEP_TIME) {                                \
        (cmd_time) = 0;                                                        \
        (angle_set) = (angle);                                                 \
        (ecd_set) = (ecd);                                                     \
        (step)++;                                                              \
      }                                                                        \
    }                                                                          \
  }

/**
 * @brief          云台行为状态机设置.
 * @param[in]      gimbal_mode_set: 云台数据指针
 * @retval         none
 */
static void gimbal_behavour_set(gimbal_control_t *gimbal_mode_set);

/**
 * @brief          当云台行为模式是GIMBAL_ZERO_FORCE,
 * 这个函数会被调用,云台控制模式是raw模式.原始模式意味着
 *                 设定值会直接发送到CAN总线上,这个函数将会设置所有为0.
 * @param[in]      yaw:发送yaw电机的原始值，会直接通过can 发送到电机
 * @param[in]      pitch:发送pitch电机的原始值，会直接通过can 发送到电机
 * @param[in]      gimbal_control_set: 云台数据指针
 * @retval         none
 */
static void gimbal_zero_force_control(fp32 *yaw, fp32 *pitch,
                                      gimbal_control_t *gimbal_control_set);

/**
 * @brief 云台初始化控制，电机是陀螺仪角度控制，云台先抬起pitch轴，后旋转yaw轴
 * @param[out]     yaw轴角度控制，为角度的增量 单位 rad
 * @param[out]     pitch轴角度控制，为角度的增量 单位 rad
 * @param[in]      云台数据指针
 * @retval         返回空
 */
static void gimbal_init_control(fp32 *yaw, fp32 *pitch,
                                gimbal_control_t *gimbal_control_set);

/**
 * @brief
 * 云台校准控制，电机是raw控制，云台先抬起pitch，放下pitch，在正转yaw，最后反转yaw，记录当时的角度和编码值
 * @author         RM
 * @param[out]     yaw:发送yaw电机的原始值，会直接通过can 发送到电机
 * @param[out]     pitch:发送pitch电机的原始值，会直接通过can 发送到电机
 * @param[in]      gimbal_control_set:云台数据指针
 * @retval         none
 */
static void gimbal_cali_control(fp32 *yaw, fp32 *pitch,
                                gimbal_control_t *gimbal_control_set);

/**
 * @brief          云台陀螺仪控制，电机是陀螺仪角度控制，
 * @param[out]     yaw: yaw轴角度控制，为角度的增量 单位 rad
 * @param[out]     pitch:pitch轴角度控制，为角度的增量 单位 rad
 * @param[in]      gimbal_control_set:云台数据指针
 * @retval         none
 */
static void gimbal_absolute_angle_control(fp32 *yaw, fp32 *pitch,
                                          gimbal_control_t *gimbal_control_set);

/**
 * @brief          云台编码值控制，电机是相对角度控制，
 * @param[in]      yaw: yaw轴角度控制，为角度的增量 单位 rad
 * @param[in]      pitch: pitch轴角度控制，为角度的增量 单位 rad
 * @param[in]      gimbal_control_set: 云台数据指针
 * @retval         none
 */
static void gimbal_relative_angle_control(fp32 *yaw, fp32 *pitch,
                                          gimbal_control_t *gimbal_control_set);

/**
 * @brief          云台进入遥控器无输入控制，电机是相对角度控制，
 * @author         RM
 * @param[in]      yaw: yaw轴角度控制，为角度的增量 单位 rad
 * @param[in]      pitch: pitch轴角度控制，为角度的增量 单位 rad
 * @param[in]      gimbal_control_set:云台数据指针
 * @retval         none
 */
static void gimbal_motionless_control(fp32 *yaw, fp32 *pitch,
                                      gimbal_control_t *gimbal_control_set);

// 云台行为状态机
static gimbal_behaviour_e gimbal_behaviour = GIMBAL_ZERO_FORCE;

/**
 * @brief
 * 被gimbal_set_mode函数调用在gimbal_task.c,云台行为状态机以及电机状态机设置
 * @param[out]     gimbal_mode_set: 云台数据指针
 * @retval         none
 */

void gimbal_behaviour_mode_set(gimbal_control_t *gimbal_mode_set) {
  if (gimbal_mode_set == NULL) {
    return;
  }
  // set gimbal_behaviour variable
  // 云台行为状态机设置
  gimbal_behavour_set(gimbal_mode_set);

  // accoring to gimbal_behaviour, set motor control mode
  // 根据云台行为状态机设置电机状态机
  if (gimbal_behaviour == GIMBAL_ZERO_FORCE) {
    gimbal_mode_set->gimbal_yaw_motor.gimbal_motor_mode = GIMBAL_MOTOR_RAW;
    gimbal_mode_set->gimbal_pitch_motor.gimbal_motor_mode = GIMBAL_MOTOR_RAW;
  } else if (gimbal_behaviour == GIMBAL_INIT) {
    gimbal_mode_set->gimbal_yaw_motor.gimbal_motor_mode = GIMBAL_MOTOR_ENCONDE;
    gimbal_mode_set->gimbal_pitch_motor.gimbal_motor_mode =
        GIMBAL_MOTOR_ENCONDE;
  } else if (gimbal_behaviour == GIMBAL_CALI) {
    gimbal_mode_set->gimbal_yaw_motor.gimbal_motor_mode = GIMBAL_MOTOR_RAW;
    gimbal_mode_set->gimbal_pitch_motor.gimbal_motor_mode = GIMBAL_MOTOR_RAW;
  } else if (gimbal_behaviour == GIMBAL_ABSOLUTE_ANGLE) {
    gimbal_mode_set->gimbal_yaw_motor.gimbal_motor_mode = GIMBAL_MOTOR_GYRO;
    gimbal_mode_set->gimbal_pitch_motor.gimbal_motor_mode = GIMBAL_MOTOR_GYRO;
  } else if (gimbal_behaviour == GIMBAL_RELATIVE_ANGLE) {
    gimbal_mode_set->gimbal_yaw_motor.gimbal_motor_mode = GIMBAL_MOTOR_ENCONDE;
    gimbal_mode_set->gimbal_pitch_motor.gimbal_motor_mode =
        GIMBAL_MOTOR_ENCONDE;
  } else if (gimbal_behaviour == GIMBAL_MOTIONLESS) {
    gimbal_mode_set->gimbal_yaw_motor.gimbal_motor_mode = GIMBAL_MOTOR_ENCONDE;
    gimbal_mode_set->gimbal_pitch_motor.gimbal_motor_mode =
        GIMBAL_MOTOR_ENCONDE;
  }
}

/**
 * @brief          云台行为控制，根据不同行为采用不同控制函数
 * @param[out]     add_yaw:设置的yaw角度增加值，单位 rad
 * @param[out]     add_pitch:设置的pitch角度增加值，单位 rad
 * @param[in]      gimbal_mode_set:云台数据指针
 * @retval         none
 */
void gimbal_behaviour_control_set(fp32 *add_yaw, fp32 *add_pitch,
                                  gimbal_control_t *gimbal_control_set) {
  if (add_yaw == NULL || add_pitch == NULL || gimbal_control_set == NULL) {
    return;
  }

  if (gimbal_behaviour == GIMBAL_ZERO_FORCE) {
    gimbal_zero_force_control(add_yaw, add_pitch, gimbal_control_set);
  } else if (gimbal_behaviour == GIMBAL_INIT) {
    gimbal_init_control(add_yaw, add_pitch, gimbal_control_set);
  } else if (gimbal_behaviour == GIMBAL_CALI) {
    gimbal_cali_control(add_yaw, add_pitch, gimbal_control_set);
  } else if (gimbal_behaviour == GIMBAL_ABSOLUTE_ANGLE) {
    gimbal_absolute_angle_control(add_yaw, add_pitch, gimbal_control_set);
  } else if (gimbal_behaviour == GIMBAL_RELATIVE_ANGLE) {
    gimbal_relative_angle_control(add_yaw, add_pitch, gimbal_control_set);
  } else if (gimbal_behaviour == GIMBAL_MOTIONLESS) {
    gimbal_motionless_control(add_yaw, add_pitch, gimbal_control_set);
  }
}

/**
 * @brief          云台在某些行为下，需要底盘不动
 * @param[in]      none
 * @retval         1: no move 0:normal
 */

bool_t gimbal_cmd_to_chassis_stop(void) {
  if (gimbal_behaviour == GIMBAL_INIT || gimbal_behaviour == GIMBAL_CALI ||
      gimbal_behaviour == GIMBAL_MOTIONLESS ||
      gimbal_behaviour == GIMBAL_ZERO_FORCE) {
    return 1;
  } else {
    return 0;
  }
}

/**
 * @brief          云台在某些行为下，需要射击停止
 * @param[in]      none
 * @retval         1: no move 0:normal
 */

bool_t gimbal_cmd_to_shoot_stop(void) {
  if (gimbal_behaviour == GIMBAL_INIT || gimbal_behaviour == GIMBAL_CALI ||
      gimbal_behaviour == GIMBAL_ZERO_FORCE) {
    return 1;
  } else {
    return 0;
  }
}

extern chassis_move_t chassis_move;

/**
 * @brief          gimbal behave mode set.
 * @param[in]      gimbal_mode_set: gimbal data
 * @retval         none
 */
/**
 * @brief          云台行为状态机设置.
 * @param[in]      gimbal_mode_set: 云台数据指针
 * @retval         none
 */
static void gimbal_behavour_set(gimbal_control_t *gimbal_mode_set) {
  if (gimbal_mode_set == NULL) {
    return;
  }
  // in cali mode, return
  // 校准行为，return 不会设置其他的模式
  if (gimbal_behaviour == GIMBAL_CALI &&
      gimbal_mode_set->gimbal_cali.step != GIMBAL_CALI_END_STEP) {
    return;
  }
  // if other operate make step change to start, means enter cali mode
  // 如果外部使得校准步骤从0 变成 start，则进入校准模式
  if (gimbal_mode_set->gimbal_cali.step == GIMBAL_CALI_START_STEP) {
    gimbal_behaviour = GIMBAL_CALI;
    return;
  }

  // init mode, judge if gimbal is in middle place
  // 初始化模式判断是否到达中值位置
  if (gimbal_behaviour == GIMBAL_INIT) {
    static uint16_t init_time = 0;
    static uint16_t init_stop_time = 0;
    init_time++;

    if ((fabs(gimbal_mode_set->gimbal_yaw_motor.relative_angle - INIT_YAW_SET) <
             GIMBAL_INIT_ANGLE_ERROR &&
         fabs(gimbal_mode_set->gimbal_pitch_motor.absolute_angle -
              INIT_PITCH_SET) < GIMBAL_INIT_ANGLE_ERROR)) {
      if (init_stop_time < GIMBAL_INIT_STOP_TIME) {
        init_stop_time++;
      }
    } else {
      if (init_time < GIMBAL_INIT_TIME) {
        init_time++;
      }
    }

    // 超过初始化最大时间，或者已经稳定到中值一段时间，退出初始化状态开关打下档，或者掉线
    if (init_time < GIMBAL_INIT_TIME &&
        init_stop_time < GIMBAL_INIT_STOP_TIME &&
        !switch_is_down(
            gimbal_mode_set->gimbal_rc_ctrl->rc.s[GIMBAL_MODE_CHANNEL])) {
      return;
    } else {
      init_stop_time = 0;
      init_time = 0;
    }
  }
  // 开关控制 云台状态
  if (chassis_move.control_mode_flag == rc) {
    if (switch_is_down(
            gimbal_mode_set->gimbal_rc_ctrl->rc.s[GIMBAL_MODE_CHANNEL])) {
      gimbal_behaviour = GIMBAL_ZERO_FORCE;
    } else if (switch_is_mid(gimbal_mode_set->gimbal_rc_ctrl->rc
                                 .s[GIMBAL_MODE_CHANNEL]) ||
               switch_is_up(gimbal_mode_set->gimbal_rc_ctrl->rc
                                .s[GIMBAL_MODE_CHANNEL])) {
      gimbal_behaviour = GIMBAL_ABSOLUTE_ANGLE;  // GIMBAL_RELATIVE_ANGLE;
    }
  }
  // 键盘控制 云台状态
  if (gimbal_mode_set->gimbal_rc_ctrl->key.v == KEY_PRESSED_OFFSET_B) {
    gimbal_behaviour = GIMBAL_ZERO_FORCE;

  } else if (gimbal_mode_set->gimbal_rc_ctrl->key.v == KEY_PRESSED_OFFSET_V ||
             gimbal_mode_set->gimbal_rc_ctrl->key.v == KEY_PRESSED_OFFSET_C) {
    gimbal_behaviour = GIMBAL_ABSOLUTE_ANGLE;  // GIMBAL_RELATIVE_ANGLE;
  }

  ///! TODO: add own logic to enter the new mode
  // if (toe_is_error(DBUS_TOE)) {
  //   gimbal_behaviour = GIMBAL_ZERO_FORCE;
  // }

  // 判断进入init状态机
  {
    static gimbal_behaviour_e last_gimbal_behaviour = GIMBAL_ZERO_FORCE;
    if (last_gimbal_behaviour == GIMBAL_ZERO_FORCE &&
        gimbal_behaviour != GIMBAL_ZERO_FORCE) {
      gimbal_behaviour = GIMBAL_INIT;
    }
    last_gimbal_behaviour = gimbal_behaviour;
  }
}

/**
 * @brief          当云台行为模式是GIMBAL_ZERO_FORCE,
 * 这个函数会被调用,云台控制模式是raw模式.原始模式意味着
 *                 设定值会直接发送到CAN总线上,这个函数将会设置所有为0.
 * @param[in]      yaw:发送yaw电机的原始值，会直接通过can 发送到电机
 * @param[in]      pitch:发送pitch电机的原始值，会直接通过can 发送到电机
 * @param[in]      gimbal_control_set: 云台数据指针
 * @retval         none
 */
static void gimbal_zero_force_control(fp32 *yaw, fp32 *pitch,
                                      gimbal_control_t *gimbal_control_set) {
  if (yaw == NULL || pitch == NULL || gimbal_control_set == NULL) {
    return;
  }

  *yaw = 0.0f;
  *pitch = 0.0f;
}

/**
 * @brief 云台初始化控制，电机是陀螺仪角度控制，云台先抬起pitch轴，后旋转yaw轴
 * @author         RM
 * @param[out]     yaw轴角度控制，为角度的增量 单位 rad
 * @param[out]     pitch轴角度控制，为角度的增量 单位 rad
 * @param[in]      云台数据指针
 * @retval         返回空
 */
static void gimbal_init_control(fp32 *yaw, fp32 *pitch,
                                gimbal_control_t *gimbal_control_set) {
  if (yaw == NULL || pitch == NULL || gimbal_control_set == NULL) {
    return;
  }

  // 初始化状态控制量计算
  if (fabs(INIT_PITCH_SET -
           gimbal_control_set->gimbal_pitch_motor.relative_angle) >
      GIMBAL_INIT_ANGLE_ERROR) {
    *pitch = (INIT_PITCH_SET -
              gimbal_control_set->gimbal_pitch_motor.absolute_angle) *
             GIMBAL_INIT_PITCH_SPEED;
    *yaw = 0.0f;
  } else {
    *pitch = (INIT_PITCH_SET -
              gimbal_control_set->gimbal_pitch_motor.relative_angle) *
             GIMBAL_INIT_PITCH_SPEED;
    *yaw =
        (INIT_YAW_SET - gimbal_control_set->gimbal_yaw_motor.relative_angle) *
        GIMBAL_INIT_YAW_SPEED;
  }
}

/**
 * @brief
 * 云台校准控制，电机是raw控制，云台先抬起pitch，放下pitch，在正转yaw，最后反转yaw，记录当时的角度和编码值
 * @author         RM
 * @param[out]     yaw:发送yaw电机的原始值，会直接通过can 发送到电机
 * @param[out]     pitch:发送pitch电机的原始值，会直接通过can 发送到电机
 * @param[in]      gimbal_control_set:云台数据指针
 * @retval         none
 */
static void gimbal_cali_control(fp32 *yaw, fp32 *pitch,
                                gimbal_control_t *gimbal_control_set) {
  if (yaw == NULL || pitch == NULL || gimbal_control_set == NULL) {
    return;
  }
  static uint16_t cali_time = 0;

  if (gimbal_control_set->gimbal_cali.step == GIMBAL_CALI_PITCH_MAX_STEP) {
    *pitch = GIMBAL_CALI_MOTOR_SET;
    *yaw = 0;

    // 判断陀螺仪数据， 并记录最大最小角度数据
    gimbal_cali_gyro_judge(
        gimbal_control_set->gimbal_pitch_motor.motor_gyro, cali_time,
        gimbal_control_set->gimbal_cali.max_pitch,
        gimbal_control_set->gimbal_pitch_motor.absolute_angle,
        gimbal_control_set->gimbal_cali.max_pitch_ecd,
        gimbal_control_set->gimbal_pitch_motor.gimbal_motor_measure->ecd,
        gimbal_control_set->gimbal_cali.step);
  } else if (gimbal_control_set->gimbal_cali.step ==
             GIMBAL_CALI_PITCH_MIN_STEP) {
    *pitch = -GIMBAL_CALI_MOTOR_SET;
    *yaw = 0;

    gimbal_cali_gyro_judge(
        gimbal_control_set->gimbal_pitch_motor.motor_gyro, cali_time,
        gimbal_control_set->gimbal_cali.min_pitch,
        gimbal_control_set->gimbal_pitch_motor.absolute_angle,
        gimbal_control_set->gimbal_cali.min_pitch_ecd,
        gimbal_control_set->gimbal_pitch_motor.gimbal_motor_measure->ecd,
        gimbal_control_set->gimbal_cali.step);
  } else if (gimbal_control_set->gimbal_cali.step == GIMBAL_CALI_YAW_MAX_STEP) {
    *pitch = 0;
    *yaw = GIMBAL_CALI_MOTOR_SET;

    gimbal_cali_gyro_judge(
        gimbal_control_set->gimbal_yaw_motor.motor_gyro, cali_time,
        gimbal_control_set->gimbal_cali.max_yaw,
        gimbal_control_set->gimbal_yaw_motor.absolute_angle,
        gimbal_control_set->gimbal_cali.max_yaw_ecd,
        gimbal_control_set->gimbal_yaw_motor.gimbal_motor_measure->ecd,
        gimbal_control_set->gimbal_cali.step);
  }

  else if (gimbal_control_set->gimbal_cali.step == GIMBAL_CALI_YAW_MIN_STEP) {
    *pitch = 0;
    *yaw = -GIMBAL_CALI_MOTOR_SET;

    gimbal_cali_gyro_judge(
        gimbal_control_set->gimbal_yaw_motor.motor_gyro, cali_time,
        gimbal_control_set->gimbal_cali.min_yaw,
        gimbal_control_set->gimbal_yaw_motor.absolute_angle,
        gimbal_control_set->gimbal_cali.min_yaw_ecd,
        gimbal_control_set->gimbal_yaw_motor.gimbal_motor_measure->ecd,
        gimbal_control_set->gimbal_cali.step);
  } else if (gimbal_control_set->gimbal_cali.step == GIMBAL_CALI_END_STEP) {
    cali_time = 0;
  }
}

/**
 * @brief          云台陀螺仪控制，电机是陀螺仪角度控制，
 * @param[out]     yaw: yaw轴角度控制，为角度的增量 单位 rad
 * @param[out]     pitch:pitch轴角度控制，为角度的增量 单位 rad
 * @param[in]      gimbal_control_set:云台数据指针
 * @retval         none
 */
static void gimbal_absolute_angle_control(
    fp32 *yaw, fp32 *pitch, gimbal_control_t *gimbal_control_set) {
  if (yaw == NULL || pitch == NULL || gimbal_control_set == NULL) {
    return;
  }

  static int16_t yaw_channel = 0, pitch_channel = 0;

  rc_deadband_limit(gimbal_control_set->gimbal_rc_ctrl->rc.ch[YAW_CHANNEL],
                    yaw_channel, RC_DEADBAND);
  rc_deadband_limit(gimbal_control_set->gimbal_rc_ctrl->rc.ch[PITCH_CHANNEL],
                    pitch_channel, RC_DEADBAND);

  *yaw = yaw_channel * YAW_RC_SEN -
         gimbal_control_set->gimbal_rc_ctrl->mouse.x * YAW_MOUSE_SEN;
  *pitch = pitch_channel * PITCH_RC_SEN +
           gimbal_control_set->gimbal_rc_ctrl->mouse.y * PITCH_MOUSE_SEN;
}

/**
 * @brief          云台编码值控制，电机是相对角度控制，
 * @param[in]      yaw: yaw轴角度控制，为角度的增量 单位 rad
 * @param[in]      pitch: pitch轴角度控制，为角度的增量 单位 rad
 * @param[in]      gimbal_control_set: 云台数据指针
 * @retval         none
 */
static void gimbal_relative_angle_control(
    fp32 *yaw, fp32 *pitch, gimbal_control_t *gimbal_control_set) {
  if (yaw == NULL || pitch == NULL || gimbal_control_set == NULL) {
    return;
  }
  static int16_t yaw_channel = 0, pitch_channel = 0;

  rc_deadband_limit(gimbal_control_set->gimbal_rc_ctrl->rc.ch[YAW_CHANNEL],
                    yaw_channel, RC_DEADBAND);
  rc_deadband_limit(gimbal_control_set->gimbal_rc_ctrl->rc.ch[PITCH_CHANNEL],
                    pitch_channel, RC_DEADBAND);

  *yaw =
      yaw_channel * YAW_RC_SEN +
      gimbal_control_set->gimbal_rc_ctrl->mouse.x *
          YAW_MOUSE_SEN;  // 如果鼠标移动方向和云台移动方向相反，把第二个负号改成加
  *pitch = pitch_channel * PITCH_RC_SEN +
           gimbal_control_set->gimbal_rc_ctrl->mouse.y * PITCH_MOUSE_SEN;
}

/**
 * @brief          云台进入遥控器无输入控制，电机是相对角度控制，
 * @author         RM
 * @param[in]      yaw: yaw轴角度控制，为角度的增量 单位 rad
 * @param[in]      pitch: pitch轴角度控制，为角度的增量 单位 rad
 * @param[in]      gimbal_control_set:云台数据指针
 * @retval         none
 */
static void gimbal_motionless_control(fp32 *yaw, fp32 *pitch,
                                      gimbal_control_t *gimbal_control_set) {
  if (yaw == NULL || pitch == NULL || gimbal_control_set == NULL) {
    return;
  }
  *yaw = 0.0f;
  *pitch = 0.0f;
}
