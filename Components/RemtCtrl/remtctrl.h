#ifndef __REMTCTRL_H
#define __REMTCTRL_H

#include "stm32f4xx_hal.h"
#include "type_def.h"

#define SBUS_RX_BUF_NUM 36U
#define RC_FRAME_LENGTH 18U

#define RC_CH_VALUE_MIN ((uint16_t)364)
#define RC_CH_VALUE_OFFSET ((uint16_t)1024)
#define RC_CH_VALUE_MAX ((uint16_t)1684)

#define RC_SW_UP ((uint16_t)1)
#define RC_SW_MID ((uint16_t)3)
#define RC_SW_DOWN ((uint16_t)2)
#define switch_is_down(s) (s == RC_SW_DOWN)
#define switch_is_mid(s) (s == RC_SW_MID)

#define KEY_PRESSED_OFFSET_W ((uint16_t)1 << 0)
#define KEY_PRESSED_OFFSET_S ((uint16_t)1 << 1)
#define KEY_PRESSED_OFFSET_A ((uint16_t)1 << 2)
#define KEY_PRESSED_OFFSET_D ((uint16_t)1 << 3)
// 关闭热量限制（只有一直摁着才没有热量限制）
#define KEY_PRESSED_OFFSET_SHIFT ((uint16_t)1 << 4)
#define KEY_PRESSED_OFFSET_CTRL ((uint16_t)1 << 5)  // 嘲讽
#define KEY_PRESSED_OFFSET_Q ((uint16_t)1 << 6)     // 开启摩擦轮
#define KEY_PRESSED_OFFSET_E ((uint16_t)1 << 7)     // 关闭摩擦轮
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

typedef __packed struct {
  __packed struct {
    int16_t ch[5];
    char s[2];
  } rc;

  __packed struct {
    int16_t x;
    int16_t y;
    int16_t z;
    uint8_t press_l;
    uint8_t press_r;
  } mouse;

  __packed struct {
    uint16_t v;
  } key;

} RC_t;

void remtctrl_init(void);
const RC_t *get_remote_control_point(void);
bool_t RC_data_is_error(RC_t remtctrl);
void slove_RC_lost(void);
void slove_data_error(void);
void sbus_to_print(uint8_t *sbus);

#ifdef __cplusplus
}
#endif

#endif /* __REMTCTRL_H */
