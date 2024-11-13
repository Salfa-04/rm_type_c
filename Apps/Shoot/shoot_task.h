#ifndef __SHOOT_H
#define __SHOOT_H

#include "can.h"
#include "pid.h"
#include "remtctrl.h"
#include "user_lib.h"

// 射击发射开关通道数据
#define SHOOT_RC_MODE_CHANNEL 1
// 云台模式使用的开关通道

#define SHOOT_CONTROL_TIME GIMBAL_CONTROL_TIME

#define SHOOT_FRIC_PWM_ADD_VALUE 3000.0f

// 射击摩擦轮激光打开 关闭
#define SHOOT_ON_KEYBOARD KEY_PRESSED_OFFSET_Q
#define SHOOT_OFF_KEYBOARD KEY_PRESSED_OFFSET_E

// 射击完成后 子弹弹出去后，判断时间，以防误触发
#define SHOOT_DONE_KEY_OFF_TIME 500  // 15
// 鼠标长按判断
#define PRESS_LONG_TIME 400
// 遥控器射击开关打下档一段时间后 连续发射子弹 用于清单
#define RC_S_LONG_TIME 3000
// 摩擦轮高速 加速 时间
#define UP_ADD_TIME 80
// 电机反馈码盘值范围
#define HALF_ECD_RANGE 4096
#define ECD_RANGE 8191
// 电机rmp 变化成 旋转速度的比例
#define MOTOR_RPM_TO_SPEED 0.00290888208665721596153948461415f
#define MOTOR_ECD_TO_ANGLE 0.000021305288720633905968306772076277f

#define MOTOR_ECD_TO_ANGLE_3508 0.000040372843796333501319967915790495f

#define FULL_COUNT 18

// 拨弹速度
// #define TRIGGER_SPEED               10.0f
// #define CONTINUE_TRIGGER_SPEED      15.0f
// #define READY_TRIGGER_SPEED         5.0f

// ADD_SPEED
#define READY_ADD_SPEED \
  0.001f  // 0.5f//1.0471975511965977461542144610932f;//60°？？
#define CONYINUE_ADD_SPEED \
  1.5f  // 0.7f//1.0471975511965977461542144610932f;//60°？？
#define TRIGGER_SPEED 0.5f
#define TRIGGER_READY_ANGLE_PID_MAX_OUT 3.14   // 参数怎么确定
#define TRIGGER_READY_ANGLE_PID_MAX_IOUT 3.14  // 参数怎么确定
#define TRIGGER_READY_SPEED_PID_MAX_OUT 3.14   // 参数怎么确定
#define TRIGGER_READY_SPEED_PID_MAX_IOUT 3.14  // 参数怎么确定

#define KEY_OFF_JUGUE_TIME 500
#define SWITCH_TRIGGER_ON 0
#define SWITCH_TRIGGER_OFF 1

// 卡单时间 以及反转时间
#define BLOCK_TRIGGER_SPEED 1.0f
#define BLOCK_TIME 700
#define REVERSE_TIME 500
#define REVERSE_SPEED_LIMIT 13.0f

#define PI_FOUR 0.78539816339744830961566084581988f
#define PI_TEN 1.2f  // 0.4186f//0.314f

// 拨弹轮电机速度环PID
#define TRIGGER_SPEED_PID_KP 30.0f  // 20.0f//30.0f
#define TRIGGER_SPEED_PID_KI 4.0f   // 3.1f//1.20f
#define TRIGGER_SPEED_PID_KD 1.0f   // 0.1f
#define TRIGGER_SPEED_PID_MAX_KP 16000.0f
#define TRIGGER_SPEED_PID_MAX_KI 2000.0f

// 拨弹轮电机角度环PID
#define TRIGGER_ANGLE_PID_KP 2500.0f
#define TRIGGER_ANGLE_PID_KI 16.0f         // 15.0f//0.0f
#define TRIGGER_ANGLE_PID_KD 9.0f          // 10.0f
#define TRIGGER_ANGLE_PID_MAX_KP 80000.0f  // 3250.0f
#define TRIGGER_ANGLE_PID_MAX_KI 10000.0f  // 0.0f

// #define TRIGGER_BULLET_PID_MAX_OUT  10000.0f
// #define TRIGGER_BULLET_PID_MAX_IOUT 9000.0f

// #define TRIGGER_READY_PID_MAX_OUT   10000.0f
// #define TRIGGER_READY_PID_MAX_IOUT  7000.0f

// 摩擦轮电机PID
#define FRIC1_KP 20.0f  // 16.0f//20.0f//15.0f
#define FRIC1_KI 1.0f   // 0.5f//0.8f//0.95f//0.80f//1.2f//0.6f
#define FRIC1_KD 0.0f   // 100.0f//100.0f//0.01f

#define FRIC2_KP 20.0f  // 16.0f//20.0f//15.0f
#define FRIC2_KI 1.0f   // 0.5f//0.8f//1.005f//0.7f//1.2f//0.6f
#define FRIC2_KD 0.0f   // 100.0f//100.0f	//0.01f

#define FRIC12_PID_MAX_OUT 16800.0f
#define FRIC12_PID_MAX_IOUT 16000.0f

#define SHOOT_HEAT_REMAIN_VALUE 100  // 80

#define FRIC12_FRIC_UP 7000           // 对应射速16m/s
#define FRIC12_FFRIC_DOWN 1000        // 4000
#define FRIC12_FFRIC_DOWN_DOWN 10000  // 4000
#define FRIC12_SPEED_TO_RPM 7000      // 最大电流下的转速

#define pai 6.283185307179586476925286766559f

#define MOTOR_TO_TRIGGER_RPM 0.05263157894736842105263157894737f

// SHOOT_READY_BULLET状态下的拨弹盘转速，单位rpm
#define SHOOT_BULLET_RPM -50.0f
// 摩擦轮转速
#define FRIC_SPEED -4200  //-4000//-4500//-8450//-6000//-8450//-9200//-8450
// SHOOT_BULLET状态下的的拨弹盘转速，单位rpm
#define SHOOT_READY_BULLET_RPM -60.0f  //-50.0f//-40.0f
// SHOOT_DONE状态下检测微动开关松开时间，防止误触发
#define SHOOT_DONE_KEY_OFF_TIME_time 500

#define BAKE_TIME_OVER 1000

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  SHOOT_STOP = 0,
  SHOOT_READY_FRIC,    // FRIC摩擦
  SHOOT_READY_BULLET,  // BULLET弹丸
  SHOOT_READY,
  SHOOT_BULLET,
  SHOOT_DONE = 5,

  SHOOT_BULLET_STOP,  // 卡弹
  SHOOT_FRIC_STOP,    // 有弹卡在摩擦轮之间
} shoot_mode_e;

typedef struct {
  shoot_mode_e shoot_mode;
  uint32_t shoot_ready_fric_time;
  const remtctrl_t *shoot_rc;
  const motor_measure_t *trig_motor_data;

  pid_t trigger_motor_speed_pid;
  pid_t trigger_motor_angle_pid;
  fp32 trigger_rpm_speed_set;
  fp32 rpm_speed;
  fp32 angle;
  int16_t given_current;
  int16_t ecd_Sum;
  fp32 add_angle;
  fp32 angle_set;
  int16_t trigger_flag;
  int return_back_flag;
  fp32 last_angle;
  shoot_mode_e last_shoot_mode;

  // 摩擦轮
  const motor_measure_t *shoot_fric1_measure;
  const motor_measure_t *shoot_fric2_measure;
  ramp_function_source_t fric1_ramp;
  int16_t fric_current1;
  int16_t fric_current2;
  ramp_function_source_t fric2_ramp;
  pid_t fric1_ramp_pid;
  pid_t fric2_ramp_pid;
  fp32 fric1_set_speed;
  fp32 fric1_speed;
  fp32 fric2_speed;

  bool_t press_l;
  bool_t press_r;
  bool_t last_press_l;
  bool_t last_press_r;
  uint16_t press_l_time;
  uint16_t press_r_time;
  uint16_t rc_s_time;

  uint16_t block_time;
  uint16_t reverse_time;
  bool_t move_flag;

  bool_t key;        // 二次备弹标志
  bool_t key_first;  // 一次备弹标志
  int key_time;

  uint16_t heat_limit;
  uint16_t heat;
} shoot_control_t;

// 由于射击和云台使用同一个can的id故也射击任务在云台任务中执行
extern void shoot_init(void);
extern int16_t shoot_control_loop(void);

#ifdef __cplusplus
}
#endif

#endif /* __SHOOT_H */
