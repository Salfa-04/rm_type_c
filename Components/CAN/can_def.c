#include "can.h"
#include "cmsis_os.h"
#include "type_def.h"

static CAN_HandleTypeDef hcan1;
static CAN_HandleTypeDef hcan2;
static void can_peri_init(void);
static void can_gpio_init(void);
static void can_filt_init(void);

void can_init(void) {
  can_peri_init();
  can_gpio_init();

  can_filt_init();
}

const void* getp_can1(void) { return &hcan1; }
const void* getp_can2(void) { return &hcan2; }

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
  hcan1.Init.AutoRetransmission = ENABLE;
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
  hcan2.Init.AutoRetransmission = ENABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK) {
    Error_Handler();
  }

  /* CAN1 interrupt Init */
  HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
  /* CAN2 interrupt Init */
  HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 2, 0);
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
