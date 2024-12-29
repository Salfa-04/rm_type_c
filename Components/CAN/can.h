#ifndef __CAN_H
#define __CAN_H

#include "stm32f4xx_hal.h"
#include "type_def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint16_t ecd;       // 电机编码器值
  int16_t speed_rpm;  // 电机转速
  int16_t current;    // 电机给定电流
  uint8_t temperate;  // 电机温度
  int16_t last_ecd;   // 上一次的编码器值
} motor_measure_t;

void can_init(void);
const void *getp_can1(void);
const void *getp_can2(void);

void can_low_cmd(int16_t mot1, int16_t mot2, int16_t mot3, int16_t mot4);
void can_mid_cmd(int16_t yaw, int16_t trig);
void can_high_cmd(int16_t pitch, int16_t fric_main, int16_t fric_sub);
void can_capci_cmd(bool_t start, bool_t restart);

const void *getp_mot_chas(uint8_t id);
const void *getp_mot_yaw(void);
const void *getp_mot_trig(void);
const void *getp_mot_pitch(void);
const void *getp_mot_fric(uint8_t id);

typedef enum {
  CAN_ADDR_BASE = 0x201,
  CAN_ADDR_HIGH = 0x1FF,
  CAN_ADDR_LOW = 0x200,
  CAN_ADDR_CAP = 0x000,

  /*** CAN: 1 */

  /// 底盘电机 3508    // TxID ID RxID  n
  CAN_ID_M1 = 0x201,  // 0x200 1 0x201 0
  CAN_ID_M2 = 0x202,  // 0x200 2 0x202 1
  CAN_ID_M3 = 0x203,  // 0x200 3 0x203 2
  CAN_ID_M4 = 0x204,  // 0x200 4 0x204 3

  /// 云台旋转电机 6020 // TxID ID RxID  n
  CAN_ID_YAW = 0x205,  // 0x1FF 5 0x205 4

  /// 拨弹盘电机 3508    // TxID ID RxID  n
  CAN_ID_TRIG = 0x206,  // 0x1FF 1 0x205 5

  /// 超级电容          // TxID ID RxID  n
  CAN_ID_CAP = 0x000,  // xxxx xx xxxx  x

  /*** CAN: 2 */

  /// 云台抬头电机 6020 // TxID ID RxID  n
  CAN_ID_PIT = 0x205,  // 0x1FF 6 0x206 6

  /// 发射电机 3508        // TxID ID RxID  n
  CAN_ID_FRIC_M = 0x206,  // 0x1FF 6 0x206 7
  CAN_ID_FRIC_S = 0x207,  // 0x1FF 7 0x207 8

} can_msg_id_e;

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H */
