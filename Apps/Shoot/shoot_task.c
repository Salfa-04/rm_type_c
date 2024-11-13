#include "shoot_task.h"

#include "arm_math.h"
#include "can.h"
#include "cmsis_os.h"
#include "pid.h"
#include "stdlib.h"
#include "user_lib.h"

#define shoot_laser_on() laser_on()    // 激光开启宏定义
#define shoot_laser_off() laser_off()  // 激光关闭宏定义
// 微动开关IO
#define BUTTEN_TRIG_PIN HAL_GPIO_ReadPin(BUTTON_TRIG_GPIO_Port, BUTTON_TRIG_Pin)

/**
 * @brief
 * 射击状态机设置，遥控器上拨一次开启，再上拨关闭，下拨1次发射1颗，一直处在下，则持续发射，用于3min准备时间清理子弹
 * @param[in]      void
 * @retval         void
 */
static void shoot_set_mode(void);

/// 射击反馈更新
static void shoot_feedback_update(void);

/// 堵转倒转处理
static void trigger_motor_turn_back(void);

/**
 * @brief          射击控制，控制拨弹电机角度，完成一次发射
 * @param[in]      void
 * @retval         void
 */
static void shoot_bullet_control(void);

/// 射击数据
shoot_control_t shoot_control;

int RAMP_STOP = 0;  // 斜波停止的速度改变量

void shoot_init(void) {
  static const fp32 Trigger_speed_pid[3] = {
      TRIGGER_SPEED_PID_KP, TRIGGER_SPEED_PID_KI, TRIGGER_SPEED_PID_KD};
  static const fp32 Trigger_angle_pid[3] = {
      TRIGGER_ANGLE_PID_KP, TRIGGER_ANGLE_PID_KI, TRIGGER_ANGLE_PID_KD};
  static const fp32 Fric1_speed_pid[3] = {FRIC1_KP, FRIC1_KI, FRIC1_KD};
  static const fp32 Fric2_speed_pid[3] = {FRIC2_KP, FRIC2_KI, FRIC2_KD};

  shoot_control.shoot_mode = SHOOT_STOP;
  // 遥控器指针
  shoot_control.shoot_rc = get_remote_control_point();
  // 电机指针  //这是拨弹电机官方例程用的是2006，但是英雄实际用的3508；
  shoot_control.shoot_motor_measure = get_trigger_motor_measure_point();
  shoot_control.shoot_fric1_measure = get_fric_motor_measure_point(0);
  shoot_control.shoot_fric2_measure = get_fric_motor_measure_point(1);
  shoot_control.fric2_ramp.input = shoot_control.shoot_fric2_measure->speed_rpm;
  shoot_control.fric1_ramp.input = shoot_control.shoot_fric1_measure->speed_rpm;
  // 初始化PID
  PID_init(&shoot_control.trigger_motor_speed_pid, PID_POSITION,
           Trigger_speed_pid, TRIGGER_ANGLE_PID_MAX_KP,
           TRIGGER_ANGLE_PID_MAX_KI);
  // 加一个角度环PID
  PID_init(&shoot_control.trigger_motor_angle_pid, PID_POSITION,
           Trigger_angle_pid, TRIGGER_SPEED_PID_MAX_KP,
           TRIGGER_SPEED_PID_MAX_KI);
  // 增加摩擦轮PID
  PID_init(&shoot_control.fric1_ramp_pid, PID_POSITION, Fric1_speed_pid,
           FRIC12_PID_MAX_OUT, FRIC12_PID_MAX_IOUT);
  PID_init(&shoot_control.fric2_ramp_pid, PID_POSITION, Fric2_speed_pid,
           FRIC12_PID_MAX_OUT, FRIC12_PID_MAX_IOUT);
  // 更新数据
  shoot_feedback_update();

  // 保留此函数给shoot_control.fric1_ramp.max_value赋值
  ramp_init(&shoot_control.fric1_ramp, -SHOOT_CONTROL_TIME * 0.001f, -FRIC_DOWN,
            FRIC_OFF);
  ramp_init(&shoot_control.fric2_ramp, -SHOOT_CONTROL_TIME * 0.001f, FRIC_DOWN,
            FRIC_OFF);

  shoot_control.fric_current1 = 0;
  shoot_control.fric_current2 = 0;
  shoot_control.fric1_speed = 0;
  shoot_control.fric1_speed = 0;
  shoot_control.fric1_set_speed = 0;
  shoot_control.trigger_flag = 0;
  shoot_control.angle = 0;
  shoot_control.add_angle = 0;
  shoot_control.angle_set = 0;
  shoot_control.ecd_Sum = 0;

  shoot_control.given_current = 0;
  shoot_control.rpm_speed = 0.0f;
  shoot_control.key_time = 0;
}

/**
 * @brief          射击循环
 * @param[in]      void
 * @retval         返回can控制值
 */
uint16_t addspped = 0;
int fric_speed = 0;
int shoot_down_state = 0;

int16_t shoot_control_loop(void) {
  shoot_set_mode();         // 设置状态机
  shoot_feedback_update();  // 更新数据

  if (shoot_control.shoot_mode == SHOOT_STOP) {
    // 设置拨弹轮
    shoot_control.trigger_rpm_speed_set = 0.0f;
    PID_calc(
        &shoot_control.trigger_motor_speed_pid,
        MOTOR_TO_TRIGGER_RPM * shoot_control.shoot_motor_measure->speed_rpm,
        shoot_control.trigger_rpm_speed_set);
    shoot_control.given_current =
        (int16_t)(shoot_control.trigger_motor_speed_pid.out);
    // 设置摩擦轮
    if (shoot_control.last_shoot_mode != SHOOT_STOP) {
      shoot_down_state = 0;
    }

    if (shoot_down_state == 0) {
      if (shoot_control.shoot_fric1_measure->speed_rpm != 0) {
        addspped = addspped + 1;
        if (addspped == -FRIC_SPEED) {
          addspped = -FRIC_SPEED;
        }
        fric_speed = FRIC_SPEED + addspped;

        PID_calc(&shoot_control.fric1_ramp_pid,
                 shoot_control.shoot_fric1_measure->speed_rpm, fric_speed);
        PID_calc(&shoot_control.fric2_ramp_pid,
                 shoot_control.shoot_fric2_measure->speed_rpm, -fric_speed);

        shoot_control.fric_current1 =
            shoot_control.fric1_ramp_pid
                .out;  // shoot_control.fric1_ramp.out;//shoot_control.fric1_ramp_pid.out;//PID_calc
                       // (&shoot_control .fric1_ramp_pid,shoot_control
                       // .shoot_fric1_measure ->speed_rpm ,0);
        shoot_control.fric_current2 =
            shoot_control.fric2_ramp_pid
                .out;  // shoot_control.fric2_ramp.out;//shoot_control.fric2_ramp_pid.out;//PID_calc
                       // (&shoot_control .fric2_ramp_pid,shoot_control
                       // .shoot_fric2_measure ->speed_rpm ,0);
      }
    }
    if (abs(shoot_control.shoot_fric1_measure->speed_rpm) < 1500) {
      shoot_down_state = 1;
      addspped = 0;
      fric_speed = 0;
      shoot_control.fric_current1 =
          0;  // shoot_control.fric1_ramp.out;//shoot_control.fric1_ramp_pid.out;//PID_calc
              // (&shoot_control .fric1_ramp_pid,shoot_control
              // .shoot_fric1_measure ->speed_rpm ,0);
      shoot_control.fric_current2 =
          0;  // shoot_control.fric2_ramp.out;//shoot_control.fric2_ramp_pid.out;//PID_calc
              // (&shoot_control .fric2_ramp_pid,shoot_control
              // .shoot_fric2_measure ->speed_rpm ,0);
    }
  }

  else if (shoot_control.shoot_mode == SHOOT_READY_FRIC) {
    // 设置拨弹轮的速度
    shoot_laser_on();
    shoot_control.trigger_rpm_speed_set = 0.0f;
    PID_calc(&shoot_control.trigger_motor_speed_pid,
             shoot_control.shoot_motor_measure->speed_rpm,
             shoot_control.trigger_rpm_speed_set);
    PID_calc(&shoot_control.fric1_ramp_pid,
             shoot_control.shoot_fric1_measure->speed_rpm,
             FRIC_SPEED /*FRIC12_SPEED_TO_RPM*/);
    PID_calc(&shoot_control.fric2_ramp_pid,
             shoot_control.shoot_fric2_measure->speed_rpm,
             -FRIC_SPEED /*-FRIC12_SPEED_TO_RPM*/);

    shoot_control.fric_current1 = (int16_t)(shoot_control.fric1_ramp_pid.out);
    shoot_control.fric_current2 = (int16_t)(shoot_control.fric2_ramp_pid.out);
    shoot_control.given_current =
        (int16_t)(shoot_control.trigger_motor_speed_pid.out);
  }
  // 如果摩擦轮转不起来说明有但
  if (shoot_control.shoot_mode == SHOOT_FRIC_STOP) {
    PID_calc(&shoot_control.fric1_ramp_pid,
             shoot_control.shoot_fric1_measure->speed_rpm, -FRIC_SPEED);
    PID_calc(&shoot_control.fric2_ramp_pid,
             shoot_control.shoot_fric2_measure->speed_rpm, +FRIC_SPEED);

    shoot_control.fric_current1 = (int16_t)(shoot_control.fric1_ramp_pid.out);
    shoot_control.fric_current2 = (int16_t)(shoot_control.fric2_ramp_pid.out);
    shoot_control.given_current = 0;
    shoot_control.shoot_ready_fric_time--;
    if (shoot_control.shoot_ready_fric_time <= 500) {
      shoot_control.shoot_mode = SHOOT_READY_FRIC;
      shoot_control.shoot_ready_fric_time = 0;
    }

  } else if (shoot_control.shoot_mode == SHOOT_READY_BULLET)  // 装弹写成速度环
  {
    PID_calc(&shoot_control.fric1_ramp_pid,
             shoot_control.shoot_fric1_measure->speed_rpm,
             FRIC_SPEED /*FRIC12_SPEED_TO_RPM*/);
    PID_calc(&shoot_control.fric2_ramp_pid,
             shoot_control.shoot_fric2_measure->speed_rpm,
             -FRIC_SPEED /*-FRIC12_SPEED_TO_RPM*/);
    shoot_control.fric_current1 = (int16_t)(shoot_control.fric1_ramp_pid.out);
    shoot_control.fric_current2 = (int16_t)(shoot_control.fric2_ramp_pid.out);
    if (shoot_control.key == SWITCH_TRIGGER_OFF) {
      // 设置拨弹轮的拨动速度,并开启堵转反转处理(这两个也要改成角度)
      shoot_control.trigger_rpm_speed_set =
          SHOOT_READY_BULLET_RPM;  // 0.006*19;//trigger_speed_set我按输出轴rpm算的：每秒转60°

      trigger_motor_turn_back();
    }

    else {
      shoot_control.trigger_rpm_speed_set = 0.0f;
    }

    PID_calc(
        &shoot_control.trigger_motor_speed_pid,
        MOTOR_TO_TRIGGER_RPM * shoot_control.shoot_motor_measure->speed_rpm,
        shoot_control.trigger_rpm_speed_set);
    shoot_control.given_current =
        (int16_t)(shoot_control.trigger_motor_speed_pid.out);

  }

  else if (shoot_control.shoot_mode == SHOOT_READY) {
    shoot_laser_on();
    // 设置拨弹轮的速度
    // shoot_control.trigger_rpm_speed_set  = 0.0f;
    shoot_control.add_angle = 0;
    shoot_control.angle_set = shoot_control.angle + shoot_control.add_angle;
    PID_calc(&shoot_control.fric1_ramp_pid,
             shoot_control.shoot_fric1_measure->speed_rpm,
             FRIC_SPEED /*FRIC12_SPEED_TO_RPM*/);
    PID_calc(&shoot_control.fric2_ramp_pid,
             shoot_control.shoot_fric2_measure->speed_rpm,
             -FRIC_SPEED /*-FRIC12_SPEED_TO_RPM*/);

    PID_calc(&shoot_control.trigger_motor_angle_pid, shoot_control.angle,
             shoot_control.angle_set);
    PID_calc(
        &shoot_control.trigger_motor_speed_pid,
        MOTOR_TO_TRIGGER_RPM * shoot_control.shoot_motor_measure->speed_rpm,
        shoot_control.trigger_motor_angle_pid.out);

    shoot_control.given_current =
        (int16_t)(shoot_control.trigger_motor_speed_pid.out);
    shoot_control.fric_current1 = (int16_t)(shoot_control.fric1_ramp_pid.out);
    shoot_control.fric_current2 = (int16_t)(shoot_control.fric2_ramp_pid.out);

  } else if (shoot_control.shoot_mode == SHOOT_BULLET) {
    shoot_laser_on();
    shoot_control.trigger_rpm_speed_set = SHOOT_BULLET_RPM;

    PID_calc(&shoot_control.fric1_ramp_pid,
             shoot_control.shoot_fric1_measure->speed_rpm,
             FRIC_SPEED /*FRIC12_SPEED_TO_RPM*/);
    PID_calc(&shoot_control.fric2_ramp_pid,
             shoot_control.shoot_fric2_measure->speed_rpm,
             -FRIC_SPEED /*-FRIC12_SPEED_TO_RPM*/);
    PID_calc(
        &shoot_control.trigger_motor_speed_pid,
        MOTOR_TO_TRIGGER_RPM * shoot_control.shoot_motor_measure->speed_rpm,
        shoot_control.trigger_rpm_speed_set);

    // 判断弹丸是否发射出去，若完成进入下一个状态
    shoot_bullet_control();
    shoot_control.given_current =
        (int16_t)(shoot_control.trigger_motor_speed_pid.out);

    trigger_motor_turn_back();

    shoot_control.fric_current1 = (int16_t)(shoot_control.fric1_ramp_pid.out);
    shoot_control.fric_current2 = (int16_t)(shoot_control.fric2_ramp_pid.out);
  }

  else if (shoot_control.shoot_mode == SHOOT_DONE) {
    shoot_laser_on();
    PID_calc(&shoot_control.fric1_ramp_pid,
             shoot_control.shoot_fric1_measure->speed_rpm,
             FRIC_SPEED /*FRIC12_SPEED_TO_RPM*/);
    PID_calc(&shoot_control.fric2_ramp_pid,
             shoot_control.shoot_fric2_measure->speed_rpm,
             -FRIC_SPEED /*-FRIC12_SPEED_TO_RPM*/);
    shoot_control.fric_current1 = (int16_t)(shoot_control.fric1_ramp_pid.out);
    shoot_control.fric_current2 = (int16_t)(shoot_control.fric2_ramp_pid.out);
    //					//速度环控制
    shoot_control.trigger_rpm_speed_set = 0.0f;
    PID_calc(
        &shoot_control.trigger_motor_speed_pid,
        MOTOR_TO_TRIGGER_RPM * shoot_control.shoot_motor_measure->speed_rpm,
        shoot_control.trigger_rpm_speed_set);
    shoot_control.given_current =
        (int16_t)(shoot_control.trigger_motor_speed_pid.out);
  }

  // 卡弹状态

  if (shoot_control.shoot_mode == SHOOT_BULLET_STOP) {
    // 摩擦轮：与非停止模式一样
    PID_calc(&shoot_control.fric1_ramp_pid,
             shoot_control.shoot_fric1_measure->speed_rpm,
             FRIC_SPEED /*FRIC12_SPEED_TO_RPM*/);
    PID_calc(&shoot_control.fric2_ramp_pid,
             shoot_control.shoot_fric2_measure->speed_rpm,
             -FRIC_SPEED /*-FRIC12_SPEED_TO_RPM*/);
    shoot_control.fric_current1 = (int16_t)(shoot_control.fric1_ramp_pid.out);
    shoot_control.fric_current2 = (int16_t)(shoot_control.fric2_ramp_pid.out);

    if (shoot_control.return_back_flag ==
        BAKE_TIME_OVER)  // BAKE_TIME_OVER是卡弹标志宏
    {
      shoot_control.add_angle = +1.20f;  // 电机逆时针转增加角度为负
      shoot_control.angle_set = shoot_control.angle + shoot_control.add_angle;
      shoot_control.return_back_flag = 0;
    }

    PID_calc(&shoot_control.trigger_motor_angle_pid, shoot_control.angle,
             shoot_control.angle_set);
    PID_calc(&shoot_control.trigger_motor_speed_pid,
             shoot_control.shoot_motor_measure->speed_rpm,
             shoot_control.trigger_motor_angle_pid.out);
    shoot_control.given_current =
        (int16_t)(shoot_control.trigger_motor_speed_pid.out);
    if (fabs(shoot_control.rpm_speed) < 0.001) {
      shoot_control.return_back_flag++;
    }
    if (fabs(shoot_control.angle - shoot_control.angle_set) < 0.1 ||
        shoot_control.return_back_flag == 1000) {
      shoot_control.shoot_mode = SHOOT_READY_BULLET;
      shoot_control.return_back_flag = 0;
    }
  }

  // 摩擦轮电机
  CAN_cmd_firc(shoot_control.fric_current1, shoot_control.fric_current2, 0, 0);
  // 返回播弹电机电流值
  return shoot_control.given_current;  // 反回的这个电流是底盘拨弹机构的。
}

/**
 * @brief
 * 射击状态机设置，遥控器上拨一次开启，再上拨关闭，下拨1次发射1颗，一直处在下，则持续发射，用于3min准备时间清理子弹
 *
 * @param[in]      void
 * @retval         void
 */
static void shoot_set_mode(void) {
  shoot_control.last_shoot_mode = shoot_control.shoot_mode;
  static int8_t last_s = RC_SW_UP;

  // 上拨判断， 一次开启，再次关闭  ：开启状态表现为摩擦轮加速
  if ((switch_is_up(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) &&
       !switch_is_up(last_s) && shoot_control.shoot_mode == SHOOT_STOP)) {
    shoot_control.shoot_mode = SHOOT_READY_FRIC;
  } else if (shoot_control.shoot_mode == SHOOT_READY_FRIC &&
             fabs(shoot_control.shoot_fric1_measure->speed_rpm) < 500 &&
             fabs(shoot_control.shoot_fric1_measure->speed_rpm) < 500) {
    shoot_control.shoot_ready_fric_time++;
    if (shoot_control.shoot_ready_fric_time > 1000) {
      shoot_control.shoot_mode = SHOOT_FRIC_STOP;
    }
  } else if ((switch_is_up(
                  shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) &&
              !switch_is_up(last_s) &&
              shoot_control.shoot_mode != SHOOT_STOP)) {
    shoot_control.shoot_mode = SHOOT_STOP;
  }

  // 处于中档， 可以使用键盘开启摩擦轮 宏SHOOT_ON_KEYBOARD对应的是键盘上的Q
  if (switch_is_mid(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) &&
      (shoot_control.shoot_rc->key.v & SHOOT_ON_KEYBOARD) &&
      shoot_control.shoot_mode == SHOOT_STOP) {
    shoot_control.shoot_mode = SHOOT_READY_FRIC;
  }
  // 处于中档， 可以使用键盘关闭摩擦轮 //也有一个宏定义
  else if (switch_is_mid(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) &&
           (shoot_control.shoot_rc->key.v & SHOOT_OFF_KEYBOARD) &&
           shoot_control.shoot_mode != SHOOT_STOP) {
    shoot_control.shoot_mode = SHOOT_STOP;
  }
  // 三位开关位于上中，且摩擦轮加速到最高速度。模式设置成装弹
  if (shoot_control.shoot_mode == SHOOT_READY_FRIC &&
      fabs(shoot_control.shoot_fric1_measure->speed_rpm) >
          fabs(FRIC_SPEED + 50) &&
      shoot_control.shoot_fric2_measure->speed_rpm > -FRIC_SPEED - 50) {
    shoot_control.shoot_mode = SHOOT_READY_BULLET;
  }
  // 装弹完毕（微动开关被按下），状态设置成准备射击。微动开关没有被按下就一直是装弹弹状态
  else if (shoot_control.shoot_mode == SHOOT_READY_BULLET &&
           shoot_control.key == SWITCH_TRIGGER_ON) {
    shoot_control.shoot_mode = SHOOT_READY;
  } else if (shoot_control.shoot_mode == SHOOT_READY &&
             shoot_control.key == SWITCH_TRIGGER_OFF)  // 位动开关没有被按下
  {
    shoot_control.shoot_mode = SHOOT_READY_BULLET;
  } else if (shoot_control.shoot_mode == SHOOT_READY) {
    // 下拨一次或者鼠标按下一次，进入射击状态——!switch_is_down(last_s)确保上一次状态不是开关在下。
    if ((switch_is_down(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) &&
         !switch_is_down(last_s)) ||
        (shoot_control.press_l && shoot_control.last_press_l == 0) ||
        (shoot_control.press_r && shoot_control.last_press_r == 0)) {
      shoot_control.move_flag = 0;
      shoot_control.shoot_mode = SHOOT_BULLET;
    }
  } else if (shoot_control.shoot_mode ==
             SHOOT_DONE)  // SHOOT_DONE转其他模式的唯一途径
  {
    if (shoot_control.key == SWITCH_TRIGGER_OFF)  // 记录微动开关松下的时间
    {
      shoot_control.key_time++;
      if (shoot_control.key_time > SHOOT_DONE_KEY_OFF_TIME_time)  // 防止误触发
      {
        shoot_control.key_time = 0;
        shoot_control.shoot_mode = SHOOT_READY_BULLET;
      }
    } else {
      shoot_control.key_time = 0;
      shoot_control.shoot_mode = SHOOT_READY;  // SHOOT_BULLET;
    }
  }

  // 热量限制
  if (shoot_control.shoot_rc->key.v !=
      KEY_PRESSED_OFFSET_SHIFT)  // shift关闭热量限制（只有一直摁着才没有热量限制）
  {
    get_shoot_heat0_limit_and_heat0(&shoot_control.heat_limit,
                                    &shoot_control.heat);
    if (!toe_is_error(REFEREE_TOE) &&
        (shoot_control.heat + SHOOT_HEAT_REMAIN_VALUE >
         shoot_control.heat_limit)) {
      if (shoot_control.shoot_mode == SHOOT_BULLET) {
        shoot_control.shoot_mode = SHOOT_READY;
      }
    }
  }
  // 正式比赛修改此处
  // 如果云台状态是 无力状态，就关闭射击
  if (gimbal_cmd_to_shoot_stop()) {
    shoot_control.shoot_mode = SHOOT_STOP;
  }

  last_s = shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL];
}
/**
 * @brief          射击数据更新
 * @param[in]      void
 * @retval         void
 */
static void shoot_feedback_update(void) {
  if (shoot_control.last_shoot_mode != shoot_control.shoot_mode) {
    PID_clear(&shoot_control.trigger_motor_angle_pid);
    PID_clear(&shoot_control.trigger_motor_speed_pid);
  }
  // 记录上一次
  shoot_control.last_angle = shoot_control.angle;

  static fp32 speed_fliter_1 = 0.0f;
  static fp32 speed_fliter_2 = 0.0f;
  static fp32 speed_fliter_3 = 0.0f;

  // 拨弹轮电机速度滤波一下
  static const fp32 fliter_num[3] = {1.725709860247969f, -0.75594777109163436f,
                                     0.030237910843665373f};

  // 二阶低通滤波
  speed_fliter_1 = speed_fliter_2;
  speed_fliter_2 = speed_fliter_3;
  speed_fliter_3 =
      speed_fliter_2 * fliter_num[0] + speed_fliter_1 * fliter_num[1] +
      (shoot_control.shoot_motor_measure->speed_rpm /** MOTOR_RPM_TO_SPEED)*/ *
       fliter_num[2]);
  shoot_control.rpm_speed = speed_fliter_3;

  // 电机圈数重置， 因为输出轴旋转一圈， 电机轴旋转
  // 19圈，将电机轴数据处理成输出轴数据，用于控制输出轴角度
  // shoot_control.ecd_count就是输出轴的数据（把这几个宏改了）
  if (shoot_control.shoot_motor_measure->ecd -
          shoot_control.shoot_motor_measure->last_ecd >
      HALF_ECD_RANGE) {
    shoot_control.ecd_Sum--;
  } else if (shoot_control.shoot_motor_measure->ecd -
                 shoot_control.shoot_motor_measure->last_ecd <
             -HALF_ECD_RANGE) {
    shoot_control.ecd_Sum++;
  }

  // shoot_control .angle  = shoot_control .ecd_Sum *
  // MOTOR_ECD_TO_ANGLE_3508;//算出从起始位置到现在电机输出轴转过的角度
  // 计算输出轴角度
  shoot_control.angle = (shoot_control.ecd_Sum * ECD_RANGE +
                         shoot_control.shoot_motor_measure->ecd) *
                        MOTOR_ECD_TO_ANGLE_3508;

  // 微动开关
  shoot_control.key = BUTTEN_TRIG_PIN;  // 微动开关被按下是1，复位是0；
  shoot_control.key_first = BUTTEN_TRIG_PIN;
  // 鼠标按键
  shoot_control.last_press_l = shoot_control.press_l;
  shoot_control.last_press_r = shoot_control.press_r;
  shoot_control.press_l = shoot_control.shoot_rc->mouse.press_l;
  shoot_control.press_r = shoot_control.shoot_rc->mouse.press_r;
  // 长按计时（暂时不需要）
  //    if (shoot_control.press_l)
  //    {
  //        if (shoot_control.press_l_time < PRESS_LONG_TIME)
  //        {
  //            shoot_control.press_l_time++;
  //        }
  //    }
  //    else
  //    {
  //        shoot_control.press_l_time = 0;
  //    }

  //    if (shoot_control.press_r)
  //    {
  //        if (shoot_control.press_r_time < PRESS_LONG_TIME)
  //        {
  //            shoot_control.press_r_time++;
  //        }
  //    }
  //    else
  //    {
  //        shoot_control.press_r_time = 0;
  //    }

  // 射击开关下档时间计时（连续反射不需要）
  //    if (shoot_control.shoot_mode != SHOOT_STOP &&
  //    switch_is_down(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]))
  //    {

  //        if (shoot_control.rc_s_time < RC_S_LONG_TIME)
  //        {
  //            shoot_control.rc_s_time++;
  //        }
  //    }
  //    else
  //    {
  //        shoot_control.rc_s_time = 0;
  //    }

  // 鼠标右键按下加速摩擦轮，使得左键低速射击， 右键高速射击
  //    static uint16_t up_time = 0;
  //    if (shoot_control.press_r)
  //    {
  //        up_time = UP_ADD_TIME;
  //    }

  //    if (up_time > 0)
  //    {
  //        shoot_control.fric1_ramp.max_value = FRIC_UP;
  //        shoot_control.fric2_ramp.max_value = -FRIC_UP;
  //        up_time--;
  //    }
  //    else
  //    {
  //        shoot_control.fric1_ramp.max_value = FRIC_DOWN;
  //        shoot_control.fric2_ramp.max_value = -FRIC_DOWN;
  //    }
}
// 卡弹反馈
static void trigger_motor_turn_back(void) {
  if (fabs(shoot_control.rpm_speed) < 0.00000000001) {
    shoot_control.return_back_flag++;
    if (shoot_control.return_back_flag > BAKE_TIME_OVER) {
      shoot_control.return_back_flag = BAKE_TIME_OVER;
      shoot_control.shoot_mode = SHOOT_BULLET_STOP;
    }
  }
}

/**
 * @brief          射击控制，控制拨弹电机角度，完成一次发射
 * @param[in]      void
 * @retval         void
 */
static void shoot_bullet_control(void) {
  if (shoot_control.key ==
      SWITCH_TRIGGER_OFF)  // 检测到位动开关松开，则判断为已经完成发射。
  {
    shoot_control.shoot_mode = SHOOT_DONE;
    // shoot_control.rpm_speed = 0;
  }
}
