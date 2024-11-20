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

void can_gimbal_cmd(int16_t yaw, int16_t pitch, int16_t shoot);
void can_fric_cmd(int16_t fric_main, int16_t fric_sub);
void can_chassis_cmd(int16_t mot1, int16_t mot2, int16_t mot3, int16_t mot4);
void can_capci_cmd(bool_t start, bool_t restart);

const motor_measure_t *getp_yaw_motor(void);
const motor_measure_t *getp_pitch_motor(void);
const motor_measure_t *getp_trig_motor(void);
const motor_measure_t *getp_fric_motor(uint8_t i);
const motor_measure_t *getp_chassis_motor(uint8_t i);

typedef enum {
  CAN_ADDR_BASE = 0x201,

  /// 电机接收的报文ID
  /// 0x200: ID: [1, 2, 3, 4]
  /// 0x1FF: ID: [5, 6, 7, 8]
  CAN_CHASSIS_BASE = 0x200,
  CAN_SHOOT_BASE = 0x200,
  CAN_FRIC_BASE = 0x200,
  CAN_GIMBAL_BASE = 0x1FF,
  CAN_CAPCI_BASE = 0x210,

  /*** CAN: 1 */

  /// 底盘电机 3508
  CAN_3508_M1_ID = 0x201,  // 0x200 1
  CAN_3508_M2_ID = 0x202,  // 0x200 2
  CAN_3508_M3_ID = 0x203,  // 0x200 3
  CAN_3508_M4_ID = 0x204,  // 0x200 4

  /// 超级电容
  CAN_CAPCI_ID = 0x211,  // 0x210 1

  /*** CAN: 2 */

  /// 发射电机 3508
  CAN_FRIC_MAIN_ID = 0x201,  // 0x200 1
  CAN_FRIC_SUB_ID = 0x202,   // 0x200 2

  /// 云台电机 6020
  CAN_YAW_MOTOR_ID = 0x205,  // 0x1FF 5
  CAN_PIT_MOTOR_ID = 0x206,  // 0x1FF 6

  /// 拨弹电机 3508
  CAN_TRIG_MOTOR_ID = 0x207,  // 0x1FF 7

} can_msg_id_e;

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H */
