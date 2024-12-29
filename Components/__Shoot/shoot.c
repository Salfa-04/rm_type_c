#include "shoot.h"

// #include "arm_math.h"
#include "can.h"
#include "can_task.h"
#include "cmsis_os.h"
#include "laser.h"
#include "stdlib.h"
#include "type_def.h"
// #include "user_lib.h"

#define shoot_laser_on() laser_on()    // 激光开启宏定义
#define shoot_laser_off() laser_off()  // 激光关闭宏定义

// 微动开关IO: 按下是0, 松开是1
#define TRIG_BUTTEN_HOLD() \
  HAL_GPIO_ReadPin(BUTTON_TRIG_GPIO_Port, BUTTON_TRIG_Pin)

// 遥控器射击开关通道数据
#define RC_LEVER_SHOOT shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]

// 遥控器电脑按键数据
#define RC_KEY_HOLD() shoot_control.shoot_rc->key.v

static sctrl_t shoot_control = {0};

static void shoot_set_mode(void);
static void shoot_feedback_update(void);
static void trig_motor_fallback(void);

/// 二阶低通滤波, 用于拨弹轮电机速度滤波
static const fp32 fliter_num[3] = {
    1.725709860247969f,
    -0.75594777109163436f,
    0.030237910843665373f,
};

void shoot_init(void) {
  shoot_control.shoot_mode = SHOOT_STOP;
  shoot_control.shoot_rc = getp_remtctrl();
  shoot_control.mot_trig = getp_mot_trig();
  shoot_control.mot_fric_m = getp_mot_fric(0);
  shoot_control.mot_fric_s = getp_mot_fric(1);

  /// 初始化射击数据
  shoot_feedback_update();
  shoot_control.fric1_current = 0;
  shoot_control.fric2_current = 0;
  shoot_control.fric1_speed = 0;
  shoot_control.fric2_speed = 0;
  shoot_control.fric_speed_set = 0;
  shoot_control.trigger_flag = 0;
  shoot_control.trig_angle = 0;
  shoot_control.add_angle = 0;
  shoot_control.angle_set = 0;
  shoot_control.trig_laps_sum = 0;
  shoot_control.trig_current = 0;
  shoot_control.trig_speed = 0.0f;
  shoot_control.key_time = 0;
}

void shoot_control_loop(void) {
  shoot_set_mode();         // 更新状态机
  shoot_feedback_update();  // 更新数据

  //! 摩擦轮射击关闭状态
  if (shoot_control.shoot_mode == SHOOT_STOP) {
    shoot_laser_off();  // 关闭激光
    can_trig_off();     // 关闭拨弹盘
    can_fric_off();     // 关闭摩擦轮

  }  //! 摩擦轮准备状态, 加速
  else if (shoot_control.shoot_mode == SHOOT_READY_FRIC) {
    shoot_laser_on();    // 开启激光
    can_trig_off();      // 关闭拨弹盘
    can_fric_forward();  // 开启摩擦轮

  }  //! 摩擦轮卡弹状态, 退弹, 设置摩擦轮电机反向最大速度
  else if (shoot_control.shoot_mode == SHOOT_FRIC_STOP) {
    can_fric_reverse();  // 反向转动摩擦轮
    can_trig_free();     // 拨弹盘电机自由转动

    /// 解除卡弹状态
    if (--shoot_control.shoot_ready_fric_time <= 500) {
      shoot_control.shoot_mode = SHOOT_READY_FRIC;
      shoot_control.shoot_ready_fric_time = 0;
    }

  }  //! 准备弹药, 自动上弹
  else if (shoot_control.shoot_mode == SHOOT_READY_BULLET) {
    can_fric_forward();  // 开启摩擦轮

    if (shoot_control.trig_button_hold == SWITCH_TRIGGER_OFF) {  /// 没有进弹
      // 设置拨弹轮的拨动速度, 并开启堵转反转处理
      can_trig_on();  // 开启拨弹盘
      // 0.006*19;//trigger_speed_set我按输出轴rpm算的：每秒转60°
      trig_motor_fallback();
    } else {           /// 已经进弹
      can_trig_off();  // 关闭拨弹盘
    }

  }  //! 准备就绪, 等待射击
  else if (shoot_control.shoot_mode == SHOOT_READY) {
    shoot_laser_on();    // 开启激光
    can_fric_forward();  // 开启摩擦轮
    can_trig_hold();     // 拨弹盘电机保持当前位置

  }  //! 射击状态, 发射子弹
  else if (shoot_control.shoot_mode == SHOOT_BULLET) {
    shoot_laser_on();    // 开启激光
    can_fric_forward();  // 开启摩擦轮
    can_trig_on();       // 开启拨弹盘

    // 判断弹丸是否发射出去，若完成进入下一个状态
    /// 检测到位动开关松开, 则判断为已经完成发射
    if (shoot_control.trig_button_hold == SWITCH_TRIGGER_OFF) {
      shoot_control.shoot_mode = SHOOT_DONE;
    }

    trig_motor_fallback();  // 判断电机是否被卡弹
  }  //! 射击完成后 子弹弹出去后，判断时间，以防误触发
  else if (shoot_control.shoot_mode == SHOOT_DONE) {
    shoot_laser_on();    // 开启激光
    can_fric_forward();  // 开启摩擦轮
    can_trig_off();      // 关闭拨弹盘

  }  //! 弹盘电机被卡弹
  else if (shoot_control.shoot_mode == SHOOT_BULLET_STOP) {
    can_fric_forward();  // 开启摩擦轮

    // 拨弹盘电机卡弹数值 BAKE_TIME_OVER
    if (shoot_control.return_back_flag >= BAKE_TIME_OVER) {
      can_trig_back();  // 拨弹盘电机反向转动角度
      shoot_control.return_back_flag = 0;
    }

    /// 退弹完成, 返回准备状态
    if (fabsf(shoot_control.trig_speed) < 0.001)
      shoot_control.return_back_flag++;
    if (fabs(shoot_control.trig_angle - shoot_control.angle_set) < 0.1 ||
        shoot_control.return_back_flag == 1000) {
      shoot_control.shoot_mode = SHOOT_READY_BULLET;
      shoot_control.return_back_flag = 0;
    }
  }
}

/// 射击状态机设置
/// 遥控器上拨一次开启，再上拨关闭，下拨1次发射1颗，一直处在下，则持续发射，用于3min准备时间清理子弹
///
/// 遥控器上拨一次开启加速摩擦轮，再次上拨关闭摩擦轮
/// 上拨一次后, 摩擦轮加速, 若卡弹自动退弹
static void shoot_set_mode(void) {
  static int8_t last_s = RC_SW_UP;

  /// 记录上一次的射击模式
  shoot_control.last_shoot_mode = shoot_control.shoot_mode;

  ///! NOTE: 遥控器状态更新
  // 拨杆上拨, 摩擦轮状态更新
  if (switch_is_up(RC_LEVER_SHOOT) && !switch_is_up(last_s)) {
    if (shoot_control.shoot_mode == SHOOT_STOP)
      // 射击停止 SHOOT_STOP 状态下, 摩擦轮开始旋转
      shoot_control.shoot_mode = SHOOT_READY_FRIC;
    else
      // 非射击停止 !SHOOT_STOP 状态下, 停止摩擦轮旋转
      shoot_control.shoot_mode = SHOOT_STOP;
  }  // 拨杆没有上拨, 转速很低
  else if (shoot_control.shoot_mode == SHOOT_READY_FRIC &&
           abs(shoot_control.mot_fric_m->speed_rpm) < 500 &&
           abs(shoot_control.mot_fric_m->speed_rpm) < 500) {
    // 摩擦轮加速时间超过 1000tick, 说明摩擦轮被卡弹
    if (++shoot_control.shoot_ready_fric_time > 1000)
      shoot_control.shoot_mode = SHOOT_FRIC_STOP;
  }  // 遥控器拨杆位于中档时, 可使用键盘控制
  else if (switch_is_mid(RC_LEVER_SHOOT)) {
    // 此处使用 Q 键开启摩擦轮
    if ((RC_KEY_HOLD() & SHOOT_ON_KEYBOARD) &&
        shoot_control.shoot_mode == SHOOT_STOP)
      shoot_control.shoot_mode = SHOOT_READY_FRIC;
    // 使用 E 键关闭摩擦轮
    else if ((RC_KEY_HOLD() & SHOOT_OFF_KEYBOARD) &&
             shoot_control.shoot_mode != SHOOT_STOP)
      shoot_control.shoot_mode = SHOOT_STOP;
  }

  ///! NOTE: 射击状态更新
  // 若摩擦轮准备就绪, 则进入准备弹药 SHOOT_READY_BULLET 状态
  if (shoot_control.shoot_mode == SHOOT_READY_FRIC &&
      abs(shoot_control.mot_fric_m->speed_rpm) > abs(FRIC_SPEED + 50) &&
      shoot_control.mot_fric_s->speed_rpm > -FRIC_SPEED - 50) {
    shoot_control.shoot_mode = SHOOT_READY_BULLET;
  }  // 判断是否装弹完毕
  else if (shoot_control.shoot_mode == SHOOT_READY_BULLET) {
    // 微动开关被按下(装弹完毕), 状态设置成准备射击
    if (shoot_control.trig_button_hold == SWITCH_TRIGGER_ON)
      shoot_control.shoot_mode = SHOOT_READY;
  }  // 发射准备就绪, 判断发射状态
  else if (shoot_control.shoot_mode == SHOOT_READY) {
    // 微动开关没有被按下, 状态设置成装弹状态
    if (shoot_control.trig_button_hold == SWITCH_TRIGGER_OFF) {
      shoot_control.shoot_mode = SHOOT_READY_BULLET;
    }  // 下拨一次或鼠标按下一次, 开始射击
    else if ((switch_is_down(RC_LEVER_SHOOT) && !switch_is_down(last_s)) ||
             (shoot_control.press_l && shoot_control.last_press_l == 0) ||
             (shoot_control.press_r && shoot_control.last_press_r == 0)) {
      shoot_control.shoot_mode = SHOOT_BULLET;  // 射击状态
    }
  }  // 发射完毕 SHOOT_DONE 状态, 转其他模式的唯一途径
  else if (shoot_control.shoot_mode == SHOOT_DONE) {
    if (shoot_control.trig_button_hold == SWITCH_TRIGGER_OFF) {  // 已经发射完毕
      if (++shoot_control.key_time > SHOOT_DONE_KEY_OFF_TIME_time) {
        shoot_control.key_time = 0;  // 防止误触发
        shoot_control.shoot_mode = SHOOT_READY_BULLET;
      }
    } else {  // 还未完成发射
      shoot_control.key_time = 0;
      shoot_control.shoot_mode = SHOOT_READY;
    }
  }

  // 一直长按shift关闭热量限制
  if (RC_KEY_HOLD() != KEY_PRESSED_OFFSET_SHIFT) {
    //! TODO: 正式比赛修改此处
    // get_shoot_heat0_limit_and_heat0(&shoot_control.heat_limit,
    //                                 &shoot_control.heat);

    if ((shoot_control.heat + SHOOT_HEAT_REMAIN_VALUE >
         shoot_control.heat_limit)) {
      if (shoot_control.shoot_mode == SHOOT_BULLET)
        shoot_control.shoot_mode = SHOOT_READY;
    }
  }

  //! TODO: 修改此处
  // 如果云台状态是 无力状态，就关闭射击
  // if (gimbal_cmd_to_shoot_stop()) {
  //   shoot_control.shoot_mode = SHOOT_STOP;
  // }

  last_s = RC_LEVER_SHOOT;  // 记录本次遥控器的拨杆状态
}

static void shoot_feedback_update(void) {
  static fp32 speed_fliter_1 = 0.f;
  static fp32 speed_fliter_2 = 0.f;
  static fp32 speed_fliter_3 = 0.f;

  shoot_control.last_angle = shoot_control.trig_angle;  // 记录角度
  if (shoot_control.last_shoot_mode != shoot_control.shoot_mode) can_fric_off();

  /// 对拨弹盘电机速度进行滤波
  speed_fliter_1 = speed_fliter_2;
  speed_fliter_2 = speed_fliter_3;
  speed_fliter_3 =  /// 二阶低通滤波
      speed_fliter_2 * fliter_num[0] + speed_fliter_1 * fliter_num[1] +
      (shoot_control.mot_trig->speed_rpm * fliter_num[2]);
  shoot_control.trig_speed = speed_fliter_3;

  // 电机圈数重置, 因为输出轴旋转一圈, 电机轴旋转19圈,
  // 将电机轴数据处理成输出轴数据, 用于控制输出轴角度
  // shoot_control.ecd_count 就是输出轴的数据
  int16_t deata_ecd =
      shoot_control.mot_trig->ecd - shoot_control.mot_trig->last_ecd;

  if (deata_ecd > HALF_ECD_RANGE) {
    shoot_control.trig_laps_sum--;
  } else if (deata_ecd < -HALF_ECD_RANGE) {
    shoot_control.trig_laps_sum++;
  }

  // 算出从起始位置到现在电机输出轴转过的角度计算输出轴角度
  // shoot_control.angle = shoot_control.ecd_Sum * MOTOR_ECD_TO_ANGLE_3508;
  shoot_control.trig_angle =
      (shoot_control.trig_laps_sum * ECD_RANGE + shoot_control.mot_trig->ecd) *
      MOTOR_ECD_TO_ANGLE_3508;

  // 控制器状态更新
  shoot_control.trig_button_hold = TRIG_BUTTEN_HOLD();  // 微动开关
  shoot_control.last_press_l = shoot_control.press_l;
  shoot_control.last_press_r = shoot_control.press_r;
  shoot_control.press_l = shoot_control.shoot_rc->mouse.press_l;
  shoot_control.press_r = shoot_control.shoot_rc->mouse.press_r;
}

/// 判断电机是否被卡弹
static void trig_motor_fallback(void) {
  if (fabsf(shoot_control.trig_speed) < 0.00000000001) {
    if (++shoot_control.return_back_flag > BAKE_TIME_OVER) {
      shoot_control.return_back_flag = BAKE_TIME_OVER;
      shoot_control.shoot_mode = SHOOT_BULLET_STOP;
    }
  }
}
