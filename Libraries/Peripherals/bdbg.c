#include "stm32f4xx_hal.h"
#include "type_def.h"

static UART_HandleTypeDef bdbg_v;
static DMA_HandleTypeDef bdbg_dma_tx;
static DMA_HandleTypeDef bdbg_dma_rx;
static void bdbg_uart_init(void);
static void bdbg_gpio_init(void);
static void bdbg_dma_init(void);

void bdbg_init(void) {
  bdbg_uart_init();
  bdbg_gpio_init();
  bdbg_dma_init();
}

const void* getp_bdbg(void) { return &bdbg_v; }

static void bdbg_uart_init(void) {
  /* USART1 clock enable */
  __HAL_RCC_USART1_CLK_ENABLE();

  bdbg_v.Instance = USART1;
  bdbg_v.Init.BaudRate = 115200;
  bdbg_v.Init.WordLength = UART_WORDLENGTH_8B;
  bdbg_v.Init.StopBits = UART_STOPBITS_1;
  bdbg_v.Init.Parity = UART_PARITY_NONE;
  bdbg_v.Init.Mode = UART_MODE_TX_RX;
  bdbg_v.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  bdbg_v.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&bdbg_v) != HAL_OK) {
    Error_Handler();
  }

  /* USART1 interrupt Init */
  HAL_NVIC_SetPriority(USART1_IRQn, 8, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

static void bdbg_gpio_init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  /**USART1 GPIO Configuration
  PB7     ------> USART1_RX
  PA9     ------> USART1_TX
  */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void bdbg_dma_init(void) {
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* USART1_TX DMA Init */
  bdbg_dma_tx.Instance = DMA2_Stream7;
  bdbg_dma_tx.Init.Channel = DMA_CHANNEL_4;
  bdbg_dma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  bdbg_dma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  bdbg_dma_tx.Init.MemInc = DMA_MINC_ENABLE;
  bdbg_dma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  bdbg_dma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  bdbg_dma_tx.Init.Mode = DMA_NORMAL;
  bdbg_dma_tx.Init.Priority = DMA_PRIORITY_LOW;
  bdbg_dma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  if (HAL_DMA_Init(&bdbg_dma_tx) != HAL_OK) {
    Error_Handler();
  }

  /* USART1_RX DMA Init */
  bdbg_dma_rx.Instance = DMA2_Stream5;
  bdbg_dma_rx.Init.Channel = DMA_CHANNEL_4;
  bdbg_dma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
  bdbg_dma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
  bdbg_dma_rx.Init.MemInc = DMA_MINC_ENABLE;
  bdbg_dma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  bdbg_dma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  bdbg_dma_rx.Init.Mode = DMA_NORMAL;
  bdbg_dma_rx.Init.Priority = DMA_PRIORITY_LOW;
  bdbg_dma_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  if (HAL_DMA_Init(&bdbg_dma_rx) != HAL_OK) {
    Error_Handler();
  }

  /* USART1 DMA Linker */
  __HAL_LINKDMA(&bdbg_v, hdmatx, bdbg_dma_tx);
  __HAL_LINKDMA(&bdbg_v, hdmarx, bdbg_dma_rx);

  /* DMA2_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
  /* DMA2_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
}

void USART1_IRQHandler(void) {
  /* USART1 global interrupt handler */
  HAL_UART_IRQHandler(&bdbg_v);
}

void DMA2_Stream7_IRQHandler(void) {
  /* DMA2 stream7 global interrupt handler */
  HAL_DMA_IRQHandler(&bdbg_dma_tx);
}

void DMA2_Stream5_IRQHandler(void) {
  /* DMA2 stream5 global interrupt handler */
  HAL_DMA_IRQHandler(&bdbg_dma_rx);
}
