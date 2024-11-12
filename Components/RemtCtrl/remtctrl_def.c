#include <stdlib.h>

// #include "detect_task.h"
#include "remtctrl.h"
#include "type_def.h"

static UART_HandleTypeDef huart3;
static DMA_HandleTypeDef hdma_usart3_rx;
// 接收原始数据，为18个字节，给了36个字节长度，防止DMA传输越界
static uint8_t sbus_rx_buf[2][SBUS_RX_BUF_NUM];

// 遥控器出错数据上限
#define RC_CHANNAL_ERROR_VALUE 700

void remtctrl_uart_init(void) {
  /* USART3 clock enable */
  __HAL_RCC_USART3_CLK_ENABLE();

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 100000;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_EVEN;
  huart3.Init.Mode = UART_MODE_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK) {
    Error_Handler();
  }

  /* USART3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART3_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(USART3_IRQn);
}

void remtctrl_gpio_init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIOC clock enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /**USART3 GPIO Configuration
  PC11     ------> USART3_RX
  */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void remtctrl_dma_init(void) {
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* USART3 RX DMA Init */
  hdma_usart3_rx.Instance = DMA1_Stream1;
  hdma_usart3_rx.Init.Channel = DMA_CHANNEL_4;
  hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart3_rx.Init.Mode = DMA_NORMAL;
  hdma_usart3_rx.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_usart3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  if (HAL_DMA_Init(&hdma_usart3_rx) != HAL_OK) {
    Error_Handler();
  }
  __HAL_LINKDMA(&huart3, hdmarx, hdma_usart3_rx);

  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
}

void remtctrl_start(void) {
  // enable the DMA transfer for the receiver request
  SET_BIT(huart3.Instance->CR3, USART_CR3_DMAR);

  // enalbe idle interrupt
  __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);

  // disable DMA
  __HAL_DMA_DISABLE(&hdma_usart3_rx);
  while (hdma_usart3_rx.Instance->CR & DMA_SxCR_EN) {
    __HAL_DMA_DISABLE(&hdma_usart3_rx);
  }

  hdma_usart3_rx.Instance->PAR = (uint32_t) & (USART3->DR);
  hdma_usart3_rx.Instance->M0AR = (uint32_t)(sbus_rx_buf[0]);  // buffer 1
  hdma_usart3_rx.Instance->M1AR = (uint32_t)(sbus_rx_buf[1]);  // buffer 2
  hdma_usart3_rx.Instance->NDTR = SBUS_RX_BUF_NUM;             // data length

  // enable double memory buffer and enable DMA
  SET_BIT(hdma_usart3_rx.Instance->CR, DMA_SxCR_DBM);
  __HAL_DMA_ENABLE(&hdma_usart3_rx);
}

// void remtctrl_stop(void) {
//   /* disable remote control */
//   __HAL_UART_DISABLE(&huart3);
// }

void remtctrl_restart(uint16_t dma_buf_num) {
  __HAL_UART_DISABLE(&huart3);
  __HAL_DMA_DISABLE(&hdma_usart3_rx);
  hdma_usart3_rx.Instance->NDTR = dma_buf_num;
  __HAL_DMA_ENABLE(&hdma_usart3_rx);
  __HAL_UART_ENABLE(&huart3);
}

bool_t remtctrl_data_is_err(RC_t remtctrl) {
  // 使用了go to语句 方便出错统一处理遥控器变量数据归零
  if (abs(remtctrl.rc.ch[0]) > RC_CHANNAL_ERROR_VALUE) goto error;
  if (abs(remtctrl.rc.ch[1]) > RC_CHANNAL_ERROR_VALUE) goto error;
  if (abs(remtctrl.rc.ch[2]) > RC_CHANNAL_ERROR_VALUE) goto error;
  if (abs(remtctrl.rc.ch[3]) > RC_CHANNAL_ERROR_VALUE) goto error;
  if (remtctrl.rc.s[0] == 0 || remtctrl.rc.s[1] == 0) goto error;
  return 0;

error:
  remtctrl.rc.ch[0] = 0;
  remtctrl.rc.ch[1] = 0;
  remtctrl.rc.ch[2] = 0;
  remtctrl.rc.ch[3] = 0;
  remtctrl.rc.ch[4] = 0;
  remtctrl.rc.s[0] = RC_SW_DOWN;
  remtctrl.rc.s[1] = RC_SW_DOWN;
  remtctrl.mouse.x = 0;
  remtctrl.mouse.y = 0;
  remtctrl.mouse.z = 0;
  remtctrl.mouse.press_l = 0;
  remtctrl.mouse.press_r = 0;
  remtctrl.key.v = 0;
  return 1;
}

static void sbus_to_rc(volatile const uint8_t *sbus_buf, RC_t *remt_ctrl) {
  if (sbus_buf == NULL || remt_ctrl == NULL) {
    return;
  }

  //!< Channel 0, 1, 2, 3
  remt_ctrl->rc.ch[0] = (sbus_buf[0] | (sbus_buf[1] << 8)) & 0x07ff;
  remt_ctrl->rc.ch[1] = ((sbus_buf[1] >> 3) | (sbus_buf[2] << 5)) & 0x07ff;
  remt_ctrl->rc.ch[2] =
      ((sbus_buf[2] >> 6) | (sbus_buf[3] << 2) | (sbus_buf[4] << 10)) & 0x07ff;
  remt_ctrl->rc.ch[3] = ((sbus_buf[4] >> 1) | (sbus_buf[5] << 7)) & 0x07ff;

  //!< Switch left, right
  remt_ctrl->rc.s[0] = ((sbus_buf[5] >> 4) & 0x0003);
  remt_ctrl->rc.s[1] = ((sbus_buf[5] >> 4) & 0x000C) >> 2;

  //!< Mouse X axis, Y axis, Z axis
  remt_ctrl->mouse.x = sbus_buf[6] | (sbus_buf[7] << 8);
  remt_ctrl->mouse.y = sbus_buf[8] | (sbus_buf[9] << 8);
  remt_ctrl->mouse.z = sbus_buf[10] | (sbus_buf[11] << 8);

  //!< Mouse IS Left Press, IS Right Press
  remt_ctrl->mouse.press_l = sbus_buf[12];
  remt_ctrl->mouse.press_r = sbus_buf[13];

  //!< KeyBoard value and NULL
  remt_ctrl->key.v = sbus_buf[14] | (sbus_buf[15] << 8);
  remt_ctrl->rc.ch[4] = sbus_buf[16] | (sbus_buf[17] << 8);

  //! set the offset value of the remote control channel
  remt_ctrl->rc.ch[0] -= RC_CH_VALUE_OFFSET;
  remt_ctrl->rc.ch[1] -= RC_CH_VALUE_OFFSET;
  remt_ctrl->rc.ch[2] -= RC_CH_VALUE_OFFSET;
  remt_ctrl->rc.ch[3] -= RC_CH_VALUE_OFFSET;
  remt_ctrl->rc.ch[4] -= RC_CH_VALUE_OFFSET;
}

// 串口中断
void USART3_IRQHandler(void) {
  if (huart3.Instance->SR & UART_FLAG_RXNE)  // 接收到数据
  {
    __HAL_UART_CLEAR_PEFLAG(&huart3);
  } else if (USART3->SR & UART_FLAG_IDLE) {
    static uint16_t this_time_rx_len = 0;
    __HAL_UART_CLEAR_PEFLAG(&huart3);
    RC_t *remt_ctrl = (RC_t *)get_remote_control_point();

    if ((hdma_usart3_rx.Instance->CR & DMA_SxCR_CT) == RESET) {
      /* Current memory buffer used is Memory 0 */
      __HAL_DMA_DISABLE(&hdma_usart3_rx);
      // get receive data length, length = set_data_length - remain_length
      this_time_rx_len = SBUS_RX_BUF_NUM - hdma_usart3_rx.Instance->NDTR;
      // reset set_data_lenght
      hdma_usart3_rx.Instance->NDTR = SBUS_RX_BUF_NUM;
      // set memory buffer 1
      hdma_usart3_rx.Instance->CR |= DMA_SxCR_CT;
      __HAL_DMA_ENABLE(&hdma_usart3_rx);

      if (this_time_rx_len == RC_FRAME_LENGTH) {
        sbus_to_rc(sbus_rx_buf[0], remt_ctrl);
        // detect_hook(DBUS_TOE);  // 记录数据接收时间
        sbus_to_print(sbus_rx_buf[0]);
      }
    } else {
      /* Current memory buffer used is Memory 1 */
      __HAL_DMA_DISABLE(&hdma_usart3_rx);
      // get receive data length, length = set_data_length - remain_length
      this_time_rx_len = SBUS_RX_BUF_NUM - hdma_usart3_rx.Instance->NDTR;
      // reset set_data_lenght
      hdma_usart3_rx.Instance->NDTR = SBUS_RX_BUF_NUM;
      // set memory buffer 0
      DMA1_Stream1->CR &= ~(DMA_SxCR_CT);
      __HAL_DMA_ENABLE(&hdma_usart3_rx);

      if (this_time_rx_len == RC_FRAME_LENGTH) {
        sbus_to_rc(sbus_rx_buf[1], remt_ctrl);
        // detect_hook(DBUS_TOE);  // 记录数据接收时间
        sbus_to_print(sbus_rx_buf[1]);
      }
    }
  }
}

void DMA1_Stream1_IRQHandler(void) {
  /* DMA1 stream1 global interrupt handler */
  HAL_DMA_IRQHandler(&hdma_usart3_rx);
}
