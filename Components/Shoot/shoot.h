#ifndef __SHOOT_H
#define __SHOOT_H

#include "can.h"
#include "remtctrl.h"

// 射击发射开关通道数据
#define SHOOT_RC_MODE_CHANNEL 1  // rc.s[1]: 左拨杆

// 射击摩擦轮激光打开 关闭
#define SHOOT_ON_KEYBOARD KEY_PRESSED_OFFSET_Q
#define SHOOT_OFF_KEYBOARD KEY_PRESSED_OFFSET_E

#define KEY_OFF_JUGUE_TIME 500

/// 微动开关被按下和松开
#define SWITCH_TRIGGER_ON 1
#define SWITCH_TRIGGER_OFF 0

#define SHOOT_HEAT_REMAIN_VALUE 100  // 80

// 摩擦轮转速
#define FRIC_SPEED -4200  //-4000//-4500//-8450//-6000//-8450//-9200//-8450

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

  // 摩擦轮准备就绪次数, 用于判断是否卡弹
  // 数值大于 1000tick 被认为摩擦轮卡弹
  uint32_t shoot_ready_fric_time;

  // 拨弹盘退弹计数器, 用于判断是否卡弹
  // 当该值大于某一值时被认为需要退弹
  uint32_t shoot_ready_trig_time;

  ///! 遥控器数据
  const remtctrl_t *shoot_rc;

  ///! 电机数据
  const motor_measure_t *mot_trig;    // 拨弹盘电机
  const motor_measure_t *mot_fric_m;  // 摩擦轮电机
  const motor_measure_t *mot_fric_s;  // 摩擦轮电机

  ///! 键盘按键
  bool_t press_l, last_press_l;
  bool_t press_r, last_press_r;

  // 发弹口处微动开关; 1: 按下, 0: 未按下
  bool_t trig_button_hold;

  uint32_t key_time;

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
