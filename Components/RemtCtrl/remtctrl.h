#ifndef __REMTCTRL_H
#define __REMTCTRL_H

#include "stm32f4xx_hal.h"
#include "type_def.h"

#define SBUS_RX_BUF_NUM 36U  // DBUS接收缓冲区大小
#define RC_FRAME_LENGTH 18U  // 遥控器数据帧长度

/// 遥控器通道偏移量
#define RC_CH_OFFSET ((uint16_t)1024)

/// 遥控器拨杆状态
#define RC_SW_UP ((uint16_t)1)
#define RC_SW_MID ((uint16_t)3)
#define RC_SW_DOWN ((uint16_t)2)
#define switch_is_down(s) (s == RC_SW_DOWN)
#define switch_is_mid(s) (s == RC_SW_MID)
#define switch_is_up(s) (s == RC_SW_UP)

/// 遥控器按键状态
#define KEY_PRESSED_OFFSET_W ((uint16_t)1 << 0)
#define KEY_PRESSED_OFFSET_S ((uint16_t)1 << 1)
#define KEY_PRESSED_OFFSET_A ((uint16_t)1 << 2)
#define KEY_PRESSED_OFFSET_D ((uint16_t)1 << 3)
#define KEY_PRESSED_OFFSET_SHIFT ((uint16_t)1 << 4)  // 关闭热量限制
#define KEY_PRESSED_OFFSET_CTRL ((uint16_t)1 << 5)   // 嘲讽
#define KEY_PRESSED_OFFSET_Q ((uint16_t)1 << 6)      // 开启摩擦轮
#define KEY_PRESSED_OFFSET_E ((uint16_t)1 << 7)      // 关闭摩擦轮
#define KEY_PRESSED_OFFSET_R ((uint16_t)1 << 8)
#define KEY_PRESSED_OFFSET_F ((uint16_t)1 << 9)
#define KEY_PRESSED_OFFSET_G ((uint16_t)1 << 10)
#define KEY_PRESSED_OFFSET_Z ((uint16_t)1 << 11)
#define KEY_PRESSED_OFFSET_X ((uint16_t)1 << 12)  // 正弦小陀螺
#define KEY_PRESSED_OFFSET_C ((uint16_t)1 << 13)  // 小陀螺
#define KEY_PRESSED_OFFSET_V ((uint16_t)1 << 14)  // 绝对角度控制
#define KEY_PRESSED_OFFSET_B ((uint16_t)1 << 15)  // 无力模式

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  __packed struct {
    /// 遥控器通道 (chX):
    /// 0: 右横轴, 1: 右纵轴;
    /// 2: 左横轴, 3: 左纵轴;
    /// 4: 左旋钮
    ///
    /// 平面直角坐标系:
    /// 右上为正, 左下为负, 逆时针为正
    /// 范围: [-660, 660], 原点为零
    ///
    int16_t ch[5];

    /// 遥控器拨杆 (sX):
    /// 0: 右拨杆, 1: 左拨杆
    ///
    /// 三个档位:
    /// UP: 1, MID: 3, DOWN: 2
    ///
    uint8_t s[2];
  } rc;

  __packed struct {
    /// 鼠标移动速度 (x, y, z):
    /// x: 水平方向, y: 竖直方向
    ///
    /// 范围: [-32768, 32767], 静止为零
    ///
    int16_t x, y, z;

    /// 鼠标按键状态 (pressX):
    /// 按下: 1, 未按下: 0
    ///
    uint8_t press_l;
    uint8_t press_r;
  } mouse;

  __packed struct {
    /// 键盘按键状态 (v):
    /// Bit0 -- W 键
    /// Bit1 -- S 键
    /// Bit2 -- A 键
    /// Bit3 -- D 键
    /// Bit4 -- Q 键
    /// Bit5 -- E 键
    /// Bit6 -- Shift 键
    /// Bit7 -- Ctrl 键
    ///
    /// 使用方式:
    /// if ((key.v & KEY_PRESSED_OFFSET_X) != 0)
    /// { /* X 被按下 */ }
    ///
    uint16_t v;
  } key;

} remtctrl_t;

void remtctrl_init(void);
const remtctrl_t *getp_remtctrl(void);

/// 遥控器数据重定向
void sbus_to_print(uint8_t *sbus);

#ifdef __cplusplus
}
#endif

#endif /* __REMTCTRL_H */
