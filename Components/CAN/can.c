#include "can.h"

#include "cmsis_os.h"
#include "type_def.h"
// #include "detect_task.h"

static CAN_HandleTypeDef hcan1;  // CHASSIS_CAN
static CAN_HandleTypeDef hcan2;  // GIMBAL_CAN
static void can_peri_init(void);
static void can_gpio_init(void);
static void can_filt_init(void);

void can_init(void) {
  can_peri_init();
  can_gpio_init();

  can_filt_init();
}

// motor data read
#define GET_MOT_MEASURE(ptr, data)                             \
  {                                                            \
    (ptr)->last_ecd = (ptr)->ecd;                              \
    (ptr)->ecd = (uint16_t)((data)[0] << 8 | (data)[1]);       \
    (ptr)->speed_rpm = (uint16_t)((data)[2] << 8 | (data)[3]); \
    (ptr)->current = (uint16_t)((data)[4] << 8 | (data)[5]);   \
    (ptr)->temperate = (data)[6];                              \
  }

/** 电机数据:
 *
 *  0: 底盘电机1 3508电机
 *  1: 底盘电机2 3508电机
 *  2: 底盘电机3 3508电机
 *  3: 底盘电机4 3508电机
 *
 *  4: yaw云台电机    6020电机
 *  5: pitch云台电机  6020电机
 *  6: 拨弹电机       2006电机
 *
 *  7: 发射电机 3508电机
 *  8: 发射电机 3508电机
 *
 */

static motor_measure_t motor_chassis[8] = {0};

static CAN_TxHeaderTypeDef gimbal_tx_message;
static CAN_TxHeaderTypeDef chassis_tx_message;
static CAN_TxHeaderTypeDef fric_tx_message;
static CAN_TxHeaderTypeDef capci_tx_message;

static uint8_t gimbal_can_send_data[8];
static uint8_t chassis_can_send_data[8];
static uint8_t fric_can_send_data[8];
static uint8_t capci_can_send_data[8];

/// 控制电机电流: 0x205, 0x206, 0x207 (CAN2)
/// yaw:    6020电机控制电流,   范围  [-30000, 30000]
/// pitch:  6020电机控制电流,   范围  [-30000, 30000]
/// trig:   3508电机控制电流,   范围  [-10000, 10000]
/// rev: 保留
///
/// yaw: 云台yaw轴
/// pitch: 云台pitch轴
/// trig: 拨弹盘电机
void can_gimbal_cmd(int16_t yaw, int16_t pitch, int16_t trig) {
  uint32_t send_mail_box = 0;

  gimbal_tx_message.StdId = CAN_GIMBAL_ALL_ID;
  gimbal_tx_message.IDE = CAN_ID_STD;
  gimbal_tx_message.RTR = CAN_RTR_DATA;
  gimbal_tx_message.DLC = 0x08;  // len

  gimbal_can_send_data[0] = (yaw >> 8);
  gimbal_can_send_data[1] = yaw;  /// ID: 5
  gimbal_can_send_data[2] = (pitch >> 8);
  gimbal_can_send_data[3] = pitch;  /// ID: 6
  gimbal_can_send_data[4] = (trig >> 8);
  gimbal_can_send_data[5] = trig;  /// ID: 7
  gimbal_can_send_data[6] = 0;
  gimbal_can_send_data[7] = 0;

  HAL_CAN_AddTxMessage(&hcan2, &gimbal_tx_message, gimbal_can_send_data,
                       &send_mail_box);
}

/// 控制电机电流: 0x201, 0x202, 0x203, 0x204 (CAN2)
/// fric1:  3508电机控制电流,   范围  [-16384, 16384]
/// fric2:  3508电机控制电流,   范围  [-16384, 16384]
/// rev1: 保留
/// rev2: 保留
///
/// fric: 发射电机
void can_fric_cmd(int16_t fric1, int16_t fric2) {
  uint32_t send_mail_box = 0;

  fric_tx_message.StdId = CAN_SHOOT_ALL_ID;
  fric_tx_message.IDE = CAN_ID_STD;
  fric_tx_message.RTR = CAN_RTR_DATA;
  fric_tx_message.DLC = 0x08;  // len

  fric_can_send_data[0] = (fric1 >> 8);
  fric_can_send_data[1] = fric1;  /// ID: 1
  fric_can_send_data[2] = (fric2 >> 8);
  fric_can_send_data[3] = fric2;  /// ID: 2
  fric_can_send_data[4] = 0;
  fric_can_send_data[5] = 0;
  fric_can_send_data[6] = 0;
  fric_can_send_data[7] = 0;

  HAL_CAN_AddTxMessage(&hcan2, &fric_tx_message, fric_can_send_data,
                       &send_mail_box);
}

/// 控制电机电流: 0x201, 0x202, 0x203, 0x204 (CAN1)
/// mot1:  3508电机控制电流,   范围  [-16384, 16384]
/// mot2:  3508电机控制电流,   范围  [-16384, 16384]
/// mot3:  3508电机控制电流,   范围  [-16384, 16384]
/// mot4:  3508电机控制电流,   范围  [-16384, 16384]
///
/// motX: 底盘电机
void can_chassis_cmd(int16_t mot1, int16_t mot2, int16_t mot3, int16_t mot4) {
  uint32_t send_mail_box = 0;

  chassis_tx_message.StdId = CAN_CHASSIS_ALL_ID;
  chassis_tx_message.IDE = CAN_ID_STD;
  chassis_tx_message.RTR = CAN_RTR_DATA;
  chassis_tx_message.DLC = 0x08;  // len

  chassis_can_send_data[0] = mot1 >> 8;
  chassis_can_send_data[1] = mot1;
  chassis_can_send_data[2] = mot2 >> 8;
  chassis_can_send_data[3] = mot2;
  chassis_can_send_data[4] = mot3 >> 8;
  chassis_can_send_data[5] = mot3;
  chassis_can_send_data[6] = mot4 >> 8;
  chassis_can_send_data[7] = mot4;

  HAL_CAN_AddTxMessage(&hcan1, &chassis_tx_message, chassis_can_send_data,
                       &send_mail_box);
}

/// 控制电容启动: 0x210 (CAN1)
/// start:    1: 启动, 0: 不起作用
/// restart:  1: 重启, 0: 不起作用
void can_capci_cmd(bool_t start, bool_t restart) {
  uint32_t send_mail_box = 0;

  capci_tx_message.StdId = CAN_CAPCI_ALL_ID;
  capci_tx_message.IDE = CAN_ID_STD;
  capci_tx_message.RTR = CAN_RTR_DATA;
  capci_tx_message.DLC = 0x08;

  capci_can_send_data[0] = start;
  capci_can_send_data[1] = restart;
  capci_can_send_data[2] = 0;
  capci_can_send_data[3] = 0;
  capci_can_send_data[4] = 0;
  capci_can_send_data[5] = 0;
  capci_can_send_data[6] = 0;
  capci_can_send_data[7] = 0;

  HAL_CAN_AddTxMessage(&hcan1, &capci_tx_message, capci_can_send_data,
                       &send_mail_box);
}

/// 获取电机数据指针: Yaw 6020电机
const motor_measure_t *getp_yaw_motor(void) { return &motor_chassis[4]; }

/// 获取电机数据指针: Pitch 6020电机
const motor_measure_t *getp_pitch_motor(void) { return &motor_chassis[5]; }

/// 获取电机数据指针: 拨弹电机 2006电机
const motor_measure_t *getp_trig_motor(void) { return &motor_chassis[6]; }

/// 获取电机数据指针: 发射电机 3508电机
/// id: 电机编号, 范围[0, 1]
const motor_measure_t *getp_fric_motor(uint8_t id) {
  return &motor_chassis[((id & 0x01) + 7)];
}

/// 获取电机数据指针: 底盘电机 3508电机
/// id: 电机编号, 范围[0, 3]
const motor_measure_t *getp_chassis_motor(uint8_t id) {
  return &motor_chassis[(id & 0x03)];
}

static void can_peri_init(void) {
  /* CANx clock enable */
  __HAL_RCC_CAN1_CLK_ENABLE();
  __HAL_RCC_CAN2_CLK_ENABLE();

  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 3;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_10TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK) {
    Error_Handler();
  }

  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 3;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_10TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK) {
    Error_Handler();
  }

  /* CAN1 interrupt Init */
  HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
  /* CAN2 interrupt Init */
  HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
}

static void can_gpio_init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIOx clock enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /**CAN1 GPIO Configuration
  PD0     ------> CAN1_RX
  PD1     ------> CAN1_TX
  */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /**CAN2 GPIO Configuration
  PB5     ------> CAN2_RX
  PB6     ------> CAN2_TX
  */
  GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void can_filt_init(void) {
  CAN_FilterTypeDef CAN_FilterStruct = {0};

  CAN_FilterStruct.FilterActivation = ENABLE;
  CAN_FilterStruct.FilterMode = CAN_FILTERMODE_IDMASK;
  CAN_FilterStruct.FilterScale = CAN_FILTERSCALE_32BIT;
  CAN_FilterStruct.FilterIdHigh = 0x0000;
  CAN_FilterStruct.FilterIdLow = 0x0000;
  CAN_FilterStruct.FilterMaskIdHigh = 0x0000;
  CAN_FilterStruct.FilterMaskIdLow = 0x0000;
  CAN_FilterStruct.FilterBank = 0;
  CAN_FilterStruct.FilterFIFOAssignment = CAN_RX_FIFO0;

  if (HAL_CAN_ConfigFilter(&hcan1, &CAN_FilterStruct) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_CAN_Start(&hcan1) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) !=
      HAL_OK) {
    Error_Handler();
  }

  CAN_FilterStruct.SlaveStartFilterBank = 14;
  CAN_FilterStruct.FilterBank = 14;

  if (HAL_CAN_ConfigFilter(&hcan2, &CAN_FilterStruct) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_CAN_Start(&hcan2) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING) !=
      HAL_OK) {
    Error_Handler();
  }
}

void CAN1_RX0_IRQHandler(void) {
  /* CAN1 RX0 interrupts handler */
  HAL_CAN_IRQHandler(&hcan1);
}

void CAN2_RX0_IRQHandler(void) {
  /* CAN2 RX0 interrupts handler */
  HAL_CAN_IRQHandler(&hcan2);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  CAN_RxHeaderTypeDef rx_header = {0};
  uint8_t rx_data[8] = {0};
  HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data);

  if (hcan->Instance == CAN1) {  /// 底盘电机
    switch (rx_header.StdId) {
      case CAN_3508_M1_ID:
      case CAN_3508_M2_ID:
      case CAN_3508_M3_ID:
      case CAN_3508_M4_ID:

      {  /// 底盘电机
        static uint8_t id = 0;
        id = rx_header.StdId - CAN_3508_M1_ID;
        GET_MOT_MEASURE(&motor_chassis[id], rx_data);
        // detect_hook(CHASSIS_MOTOR1_TOE + id);
      } break;

      default:
        break;
    }
  } else if (hcan->Instance == CAN2) {
    switch (rx_header.StdId) {
      case CAN_YAW_MOTOR_ID:
      case CAN_PIT_MOTOR_ID:
      case CAN_TRIG_MOTOR_ID:

      {  /// 云台电机
        static uint8_t id = 0;
        id = rx_header.StdId - CAN_3508_M1_ID;
        GET_MOT_MEASURE(&motor_chassis[id], rx_data);
        // detect_hook(CHASSIS_MOTOR1_TOE + id);
      } break;

      case CAN_FRIC_M1_ID:
      case CAN_FRIC_M2_ID:

      {  /// 发射电机
        static uint8_t id = 0;
        id = rx_header.StdId - CAN_3508_M1_ID;
        GET_MOT_MEASURE(&motor_chassis[id + 7], rx_data);
        // detect_hook(CHASSIS_MOTOR1_TOE + id);
      } break;

      default:
        break;
    }
  }
}
