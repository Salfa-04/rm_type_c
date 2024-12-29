#ifndef __SHOOT_H
#define __SHOOT_H

#include "can.h"
#include "pid.h"
#include "remtctrl.h"
#include "user_lib.h"

/** TEMP ****/
#define GIMBAL_CONTROL_TIME 1
#define FRIC_OFF 0
#define FRIC_DOWN 9800
/** EO TEMP */

// 射击发射开关通道数据
#define SHOOT_RC_MODE_CHANNEL 1  // rc.s[1]: 左拨杆
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
// 电机反馈编码值范围
#define HALF_ECD_RANGE 4096
#define ECD_RANGE 8191
// 电机rpm变化成旋转速度的比例
#define MOTOR_RPM_TO_SPEED 0.00290888208665721596153948461415f
#define MOTOR_ECD_TO_ANGLE 0.000021305288720633905968306772076277f
#define MOTOR_ECD_TO_ANGLE_3508 0.000040372843796333501319967915790495f

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
/// 我也不知道这个参数怎么确定！！！

#define KEY_OFF_JUGUE_TIME 500

/// 微动开关被按下和松开
#define SWITCH_TRIGGER_ON 1
#define SWITCH_TRIGGER_OFF 0

// 卡蛋时间 以及 反转时间
#define BLOCK_TRIGGER_SPEED 1.0f
#define BLOCK_TIME 700
#define REVERSE_TIME 500
#define REVERSE_SPEED_LIMIT 13.0f

#define PI_FOUR 0.78539816339744830961566084581988f
#define PI_TEN 1.2f  // 0.4186f//0.314f

// #define TRIGGER_BULLET_PID_MAX_OUT  10000.0f
// #define TRIGGER_BULLET_PID_MAX_IOUT 9000.0f

// #define TRIGGER_READY_PID_MAX_OUT   10000.0f
// #define TRIGGER_READY_PID_MAX_IOUT  7000.0f

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

// 拨弹盘电机卡弹标志
#define BAKE_TIME_OVER 1000

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /// 射击状态
  SHOOT_STOP = 0,      // 射击关闭状态
  SHOOT_READY_FRIC,    // 准备摩擦轮, 摩擦轮加速
  SHOOT_READY_BULLET,  // 准备弹药, 自动上弹
  SHOOT_READY,         // 准备就绪, 等待射击
  SHOOT_BULLET,        // 射击状态, 发射子弹
  SHOOT_DONE = 5,      // 一次射击完毕

  /// 卡弹状态
  SHOOT_BULLET_STOP,  // 拨弹盘电机被卡弹
  SHOOT_FRIC_STOP,    // 摩擦轮电机被卡弹
} shoot_mode_e;

typedef struct {
  ///! 射击模式
  shoot_mode_e shoot_mode;
  shoot_mode_e last_shoot_mode;

  // 摩擦轮准备就绪时间, 用于判断是否卡弹
  // 数值大于 1000tick 被认为摩擦轮卡弹
  uint32_t shoot_ready_fric_time;

  ///! 遥控器数据
  const remtctrl_t *shoot_rc;

  ///! 电机数据
  const motor_measure_t *mot_trig;    // 拨弹盘电机
  const motor_measure_t *mot_fric_m;  // 摩擦轮电机
  const motor_measure_t *mot_fric_s;  // 摩擦轮电机

  ///! 拨弹盘电机
  fp32 trig_speed, trig_angle;
  int16_t trig_current;   // 拨弹盘电机电流
  int16_t trig_laps_sum;  // 拨弹盘电机圈数

  fp32 add_angle, angle_set;
  int16_t trigger_flag;

  // 退弹计数器, 当该值大于某一值时被认为需要退弹
  int return_back_flag;
  fp32 last_angle;

  ///! 摩擦轮, 发射电机
  fp32 fric_speed_set;
  int16_t fric1_current, fric2_current;
  fp32 fric1_speed, fric2_speed;

  ///! 键盘按键
  bool_t press_l, last_press_l;
  bool_t press_r, last_press_r;

  uint16_t block_time;

  // 发弹口处微动开关; 1: 按下, 0: 未按下
  bool_t trig_button_hold;

  int key_time;

  uint16_t heat_limit;
  uint16_t heat;
} sctrl_t;

// 由于射击和云台使用同一个can的id故也射击任务在云台任务中执行
void shoot_init(void);
void shoot_control_loop(void);

#ifdef __cplusplus
}
#endif

#endif /* __SHOOT_H */
