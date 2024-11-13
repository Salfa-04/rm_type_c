#include "refuart.h"

#include "type_def.h"

UART_HandleTypeDef referee_uart_v;  // USART6
static DMA_HandleTypeDef hdma_usart6_rx;
static DMA_HandleTypeDef hdma_usart6_tx;

static void refuart_uart_init(void);
static void refuart_gpio_init(void);
static void refuart_dma_init(void);

void refuart_init(void) {
  refuart_uart_init();
  refuart_gpio_init();
  refuart_dma_init();
}

void refuart_initer(uint8_t *rx1_buf, uint8_t *rx2_buf, uint16_t dma_buf_num) {
  // enable the DMA transfer for the receiver and tramsmit request
  SET_BIT(referee_uart_v.Instance->CR3, USART_CR3_DMAR);
  SET_BIT(referee_uart_v.Instance->CR3, USART_CR3_DMAT);

  // enalbe idle interrupt
  __HAL_UART_ENABLE_IT(&referee_uart_v, UART_IT_IDLE);
  __HAL_DMA_DISABLE(&hdma_usart6_rx);
  while (hdma_usart6_rx.Instance->CR & DMA_SxCR_EN) {
    __HAL_DMA_DISABLE(&hdma_usart6_rx);
  }

  __HAL_DMA_CLEAR_FLAG(&hdma_usart6_rx, DMA_LISR_TCIF1);
  hdma_usart6_rx.Instance->PAR = (uint32_t) & (USART6->DR);
  hdma_usart6_rx.Instance->M0AR = (uint32_t)(rx1_buf);  // memory buffer 1
  hdma_usart6_rx.Instance->M1AR = (uint32_t)(rx2_buf);  // memory buffer 2
  __HAL_DMA_SET_COUNTER(&hdma_usart6_rx, dma_buf_num);  // data length

  // enable double memory buffer
  SET_BIT(hdma_usart6_rx.Instance->CR, DMA_SxCR_DBM);
  __HAL_DMA_ENABLE(&hdma_usart6_rx);
  __HAL_DMA_DISABLE(&hdma_usart6_tx);

  while (hdma_usart6_tx.Instance->CR & DMA_SxCR_EN) {
    __HAL_DMA_DISABLE(&hdma_usart6_tx);
  }

  hdma_usart6_tx.Instance->PAR = (uint32_t) & (USART6->DR);
}

void refuart_starter(uint8_t *data, uint16_t len) {
  __HAL_DMA_DISABLE(&hdma_usart6_tx);
  while (hdma_usart6_tx.Instance->CR & DMA_SxCR_EN) {
    __HAL_DMA_DISABLE(&hdma_usart6_tx);
  }

  __HAL_DMA_CLEAR_FLAG(&hdma_usart6_tx, DMA_HISR_TCIF6);
  hdma_usart6_tx.Instance->M0AR = (uint32_t)(data);
  __HAL_DMA_SET_COUNTER(&hdma_usart6_tx, len);
  __HAL_DMA_ENABLE(&hdma_usart6_tx);
}

static void refuart_uart_init(void) {
  /* USART6 clock enable */
  __HAL_RCC_USART6_CLK_ENABLE();

  referee_uart_v.Instance = USART6;
  referee_uart_v.Init.BaudRate = 115200;
  referee_uart_v.Init.WordLength = UART_WORDLENGTH_8B;
  referee_uart_v.Init.StopBits = UART_STOPBITS_1;
  referee_uart_v.Init.Parity = UART_PARITY_NONE;
  referee_uart_v.Init.Mode = UART_MODE_TX_RX;
  referee_uart_v.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  referee_uart_v.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&referee_uart_v) != HAL_OK) {
    Error_Handler();
  }

  /* USART6 interrupt Init */
  HAL_NVIC_SetPriority(USART6_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(USART6_IRQn);
}

static void refuart_gpio_init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO PortG Clock Enable */
  __HAL_RCC_GPIOG_CLK_ENABLE();
  /**USART6 GPIO Configuration
  PG14     ------> USART6_TX
  PG9     ------> USART6_RX
  */
  GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

static void refuart_dma_init(void) {
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* USART6_RX DMA Init */
  hdma_usart6_rx.Instance = DMA2_Stream1;
  hdma_usart6_rx.Init.Channel = DMA_CHANNEL_5;
  hdma_usart6_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_usart6_rx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart6_rx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart6_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart6_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart6_rx.Init.Mode = DMA_NORMAL;
  hdma_usart6_rx.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_usart6_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  if (HAL_DMA_Init(&hdma_usart6_rx) != HAL_OK) {
    Error_Handler();
  }

  /* USART6_TX DMA Init */
  hdma_usart6_tx.Instance = DMA2_Stream6;
  hdma_usart6_tx.Init.Channel = DMA_CHANNEL_5;
  hdma_usart6_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_usart6_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart6_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart6_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart6_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart6_tx.Init.Mode = DMA_NORMAL;
  hdma_usart6_tx.Init.Priority = DMA_PRIORITY_MEDIUM;
  hdma_usart6_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  if (HAL_DMA_Init(&hdma_usart6_tx) != HAL_OK) {
    Error_Handler();
  }

  /* USART6 DMA Linker */
  __HAL_LINKDMA(&referee_uart_v, hdmarx, hdma_usart6_rx);
  __HAL_LINKDMA(&referee_uart_v, hdmatx, hdma_usart6_tx);

  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
}

void DMA2_Stream1_IRQHandler(void) {
  /* DMA2 stream1 global interrupt handler */
  HAL_DMA_IRQHandler(&hdma_usart6_rx);
}

void DMA2_Stream6_IRQHandler(void) {
  /* DMA2 stream6 global interrupt handler */
  HAL_DMA_IRQHandler(&hdma_usart6_tx);
}

/// 被裁判系统任务复用, 故为虚函数
__weak void USART6_IRQHandler(void) {
  /* USART6 global interrupt handler */
  HAL_UART_IRQHandler(&referee_uart_v);
}
