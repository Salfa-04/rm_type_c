#include "can.h"

#include "cmsis_os.h"
#include "type_def.h"

// motor data read
#define GET_MOT_MEASURE(ptr, data)                             \
  {                                                            \
    (ptr)->last_ecd = (ptr)->ecd;                              \
    (ptr)->ecd = (uint16_t)((data)[0] << 8 | (data)[1]);       \
    (ptr)->speed_rpm = (uint16_t)((data)[2] << 8 | (data)[3]); \
    (ptr)->current = (uint16_t)((data)[4] << 8 | (data)[5]);   \
    (ptr)->temperate = (data)[6];                              \
  }

/** 电机数据: motor_chassis[9]
 *
 *  ################ CAN1 ################
 *  0: 底盘电机1    3508电机 0x200 1 0x201
 *  1: 底盘电机2    3508电机 0x200 2 0x202
 *  2: 底盘电机3    3508电机 0x200 3 0x203
 *  3: 底盘电机4    3508电机 0x200 4 0x204
 *  4: 旋转电机     6020电机 0x1FF 1 0x205
 *  5: 拨弹盘电机   3508电机 0x1FF 6 0x206
 *
 *  ################ CAN2 ################
 *  6: 抬头电机     6020电机 0x1FF 1 0x205
 *  7: 发射电机     3508电机 0x1FF 6 0x206
 *  8: 发射电机     3508电机 0x1FF 7 0x207
 *
 */

static motor_measure_t motor_datas[9] = {0};

static CAN_TxHeaderTypeDef tx_message_low;
static CAN_TxHeaderTypeDef tx_message_mid;
static CAN_TxHeaderTypeDef tx_message_high;
static CAN_TxHeaderTypeDef tx_message_cap;

static uint8_t can_send_data_low[8];
static uint8_t can_send_data_mid[8];
static uint8_t can_send_data_high[8];
static uint8_t can_send_data_cap[8];

/// 获取电机数据指针: 底盘电机 3508
/// id: 电机编号, 范围[0, 3]
const void *getp_mot_chas(uint8_t id) { return &motor_datas[(id & 0x03)]; }

/// 获取电机数据指针: 云台旋转电机 6020
const void *getp_mot_yaw(void) { return &motor_datas[4]; }

/// 获取电机数据指针: 拨弹盘电机 3508
const void *getp_mot_trig(void) { return &motor_datas[5]; }

/// 获取电机数据指针: 云台抬头电机 6020
const void *getp_mot_pitch(void) { return &motor_datas[6]; }

/// 获取电机数据指针: 发射电机 3508
/// id: 电机编号, 范围[0, 1]
const void *getp_mot_fric(uint8_t id) {
  return &motor_datas[((id & 0x01) + 7)];
}

/// 控制电机电流: 0x201, 0x202, 0x203, 0x204 (CAN1)
/// mot1:  3508电机控制电流,   范围  [-16384, 16384]
/// mot2:  3508电机控制电流,   范围  [-16384, 16384]
/// mot3:  3508电机控制电流,   范围  [-16384, 16384]
/// mot4:  3508电机控制电流,   范围  [-16384, 16384]
///
/// motX: 底盘电机
void can_low_cmd(int16_t mot1, int16_t mot2, int16_t mot3, int16_t mot4) {
  uint32_t send_mail_box = 0;

  tx_message_low.StdId = CAN_ADDR_LOW;
  tx_message_low.IDE = CAN_ID_STD;
  tx_message_low.RTR = CAN_RTR_DATA;
  tx_message_low.DLC = 0x08;  // len

  can_send_data_low[0] = (mot1 >> 8) & 0xFF;
  can_send_data_low[1] = mot1 & 0xFF;
  can_send_data_low[2] = (mot2 >> 8) & 0xFF;
  can_send_data_low[3] = mot2 & 0xFF;
  can_send_data_low[4] = (mot3 >> 8) & 0xFF;
  can_send_data_low[5] = mot3 & 0xFF;
  can_send_data_low[6] = (mot4 >> 8) & 0xFF;
  can_send_data_low[7] = mot4 & 0xFF;

  HAL_CAN_AddTxMessage((CAN_HandleTypeDef *)getp_can1(), &tx_message_low,
                       can_send_data_low, &send_mail_box);
}

/// 控制电机电流: 0x205, 0x206 (CAN1)
/// yaw :    6020电机控制电压,   范围  [-25000, 25000]
/// trig:    3508电机控制电流,   范围  [-16384, 16384]
///
/// yaw: 云台旋转电机, trig: 拨弹盘电机
void can_mid_cmd(int16_t yaw, int16_t trig) {
  uint32_t send_mail_box = 0;

  tx_message_mid.StdId = CAN_ADDR_HIGH;
  tx_message_mid.IDE = CAN_ID_STD;
  tx_message_mid.RTR = CAN_RTR_DATA;
  tx_message_mid.DLC = 0x08;  // len

  can_send_data_mid[0] = (yaw >> 8) & 0xFF;
  can_send_data_mid[1] = yaw & 0xFF;
  can_send_data_mid[2] = (trig >> 8) & 0xFF;
  can_send_data_mid[3] = trig & 0xFF;
  can_send_data_mid[4] = 0;
  can_send_data_mid[5] = 0;
  can_send_data_mid[6] = 0;
  can_send_data_mid[7] = 0;

  HAL_CAN_AddTxMessage((CAN_HandleTypeDef *)getp_can1(), &tx_message_mid,
                       can_send_data_mid, &send_mail_box);
}

/// 控制电机电流: 0x205, 0x206, 0x207 (CAN2)
/// pitch:      6020电机控制电压,   范围  [-25000, 25000]
/// fric_main:  3508电机控制电流,   范围  [-16384, 16384]
/// fric_sub:   3508电机控制电流,   范围  [-16384, 16384]
///
/// pitch:  云台pitch轴
/// fric_X: 发射电机
void can_high_cmd(int16_t pitch, int16_t fric_main, int16_t fric_sub) {
  uint32_t send_mail_box = 0;

  tx_message_high.StdId = CAN_ADDR_HIGH;
  tx_message_high.IDE = CAN_ID_STD;
  tx_message_high.RTR = CAN_RTR_DATA;
  tx_message_high.DLC = 0x08;  // len

  can_send_data_high[0] = (pitch >> 8) & 0xFF;
  can_send_data_high[1] = pitch & 0xFF;
  can_send_data_high[2] = (fric_main >> 8) & 0xFF;
  can_send_data_high[3] = fric_main & 0xFF;
  can_send_data_high[4] = (fric_sub >> 8) & 0xFF;
  can_send_data_high[5] = fric_sub & 0xFF;
  can_send_data_high[6] = 0;
  can_send_data_high[7] = 0;

  HAL_CAN_AddTxMessage((CAN_HandleTypeDef *)getp_can2(), &tx_message_high,
                       can_send_data_high, &send_mail_box);
}

/// 控制电容启动: 0x210 (CAN1)
/// start:    1: 启动, 0: 不起作用
/// restart:  1: 重启, 0: 不起作用
void can_capci_cmd(bool_t start, bool_t restart) {
  uint32_t send_mail_box = 0;

  tx_message_cap.StdId = CAN_ADDR_CAP;
  tx_message_cap.IDE = CAN_ID_STD;
  tx_message_cap.RTR = CAN_RTR_DATA;
  tx_message_cap.DLC = 0x08;

  can_send_data_cap[0] = start;
  can_send_data_cap[1] = restart;
  can_send_data_cap[2] = 0;
  can_send_data_cap[3] = 0;
  can_send_data_cap[4] = 0;
  can_send_data_cap[5] = 0;
  can_send_data_cap[6] = 0;
  can_send_data_cap[7] = 0;

  HAL_CAN_AddTxMessage((CAN_HandleTypeDef *)getp_can1(), &tx_message_cap,
                       can_send_data_cap, &send_mail_box);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  CAN_RxHeaderTypeDef rx_header = {0};
  uint8_t rx_data[8] = {0}, id = 0;
  HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data);

  if (hcan->Instance == CAN1) {
    switch (rx_header.StdId) {
      case CAN_ID_M1:
      case CAN_ID_M2:
      case CAN_ID_M3:
      case CAN_ID_M4:

      {  // 电机编号 -> 0, 1, 2, 3
        id = rx_header.StdId - CAN_ADDR_BASE;
        GET_MOT_MEASURE(&motor_datas[id], rx_data);
      } break;

      case CAN_ID_YAW:

      {  // 电机编号 -> 4
        id = rx_header.StdId - CAN_ADDR_BASE;
        GET_MOT_MEASURE(&motor_datas[id], rx_data);
      } break;

      case CAN_ID_TRIG:

      {  // 电机编号 -> 5
        id = rx_header.StdId - CAN_ADDR_BASE;
        GET_MOT_MEASURE(&motor_datas[id], rx_data);
      } break;

      default:
        break;
    }
  } else if (hcan->Instance == CAN2) {
    switch (rx_header.StdId) {
      case CAN_ID_PIT:

      {  // 电机编号 -> 6
        id = rx_header.StdId - CAN_ADDR_HIGH;
        GET_MOT_MEASURE(&motor_datas[id], rx_data);
      } break;

      case CAN_ID_FRIC_M:
      case CAN_ID_FRIC_S:

      {  // 电机编号 -> 7, 8
        id = rx_header.StdId - CAN_ADDR_HIGH;
        GET_MOT_MEASURE(&motor_datas[id + 7], rx_data);
      } break;
    }
  }
}
