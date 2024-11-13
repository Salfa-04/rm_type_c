#include "spi_dma.h"

#include "stm32f4xx_hal.h"

SPI_HandleTypeDef imu_spi_v;  // SPI1
static DMA_HandleTypeDef hdma_spi1_rx;
static DMA_HandleTypeDef hdma_spi1_tx;

void imu_dma_init(void) {
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* SPI1 DMA Init */
  hdma_spi1_rx.Instance = DMA2_Stream2;
  hdma_spi1_rx.Init.Channel = DMA_CHANNEL_3;
  hdma_spi1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_spi1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_spi1_rx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_spi1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_spi1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_spi1_rx.Init.Mode = DMA_NORMAL;
  hdma_spi1_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
  hdma_spi1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  if (HAL_DMA_Init(&hdma_spi1_rx) != HAL_OK) {
    Error_Handler();
  }

  hdma_spi1_tx.Instance = DMA2_Stream3;
  hdma_spi1_tx.Init.Channel = DMA_CHANNEL_3;
  hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_spi1_tx.Init.Mode = DMA_NORMAL;
  hdma_spi1_tx.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_spi1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK) {
    Error_Handler();
  }

  __HAL_LINKDMA(&imu_spi_v, hdmarx, hdma_spi1_rx);
  __HAL_LINKDMA(&imu_spi_v, hdmatx, hdma_spi1_tx);

  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
}

void SPI_DMA_init(uint32_t tx_buf, uint32_t rx_buf, uint16_t num) {
  SET_BIT(imu_spi_v.Instance->CR2, SPI_CR2_TXDMAEN);
  SET_BIT(imu_spi_v.Instance->CR2, SPI_CR2_RXDMAEN);

  __HAL_SPI_ENABLE(&imu_spi_v);

  // disable DMA
  __HAL_DMA_DISABLE(&hdma_spi1_rx);

  while (hdma_spi1_rx.Instance->CR & DMA_SxCR_EN)
    __HAL_DMA_DISABLE(&hdma_spi1_rx);
  __HAL_DMA_CLEAR_FLAG(&hdma_spi1_rx, DMA_LISR_TCIF2);

  hdma_spi1_rx.Instance->PAR = (uint32_t) & (SPI1->DR);
  // memory buffer 1
  hdma_spi1_rx.Instance->M0AR = (uint32_t)(rx_buf);
  // data length
  __HAL_DMA_SET_COUNTER(&hdma_spi1_rx, num);

  __HAL_DMA_ENABLE_IT(&hdma_spi1_rx, DMA_IT_TC);

  // disable DMA
  __HAL_DMA_DISABLE(&hdma_spi1_tx);

  while (hdma_spi1_tx.Instance->CR & DMA_SxCR_EN)
    __HAL_DMA_DISABLE(&hdma_spi1_tx);
  __HAL_DMA_CLEAR_FLAG(&hdma_spi1_tx, DMA_LISR_TCIF3);

  hdma_spi1_tx.Instance->PAR = (uint32_t) & (SPI1->DR);
  // memory buffer 1
  hdma_spi1_tx.Instance->M0AR = (uint32_t)(tx_buf);
  // data length
  __HAL_DMA_SET_COUNTER(&hdma_spi1_tx, num);
}

void SPI_DMA_enable(uint32_t tx_buf, uint32_t rx_buf, uint16_t ndtr) {
  __HAL_DMA_DISABLE(&hdma_spi1_rx);
  __HAL_DMA_DISABLE(&hdma_spi1_tx);

  while (hdma_spi1_rx.Instance->CR & DMA_SxCR_EN) {
    __HAL_DMA_DISABLE(&hdma_spi1_rx);
  }
  while (hdma_spi1_tx.Instance->CR & DMA_SxCR_EN) {
    __HAL_DMA_DISABLE(&hdma_spi1_tx);
  }

  __HAL_DMA_CLEAR_FLAG(imu_spi_v.hdmarx,
                       __HAL_DMA_GET_TC_FLAG_INDEX(imu_spi_v.hdmarx));
  __HAL_DMA_CLEAR_FLAG(imu_spi_v.hdmarx,
                       __HAL_DMA_GET_HT_FLAG_INDEX(imu_spi_v.hdmarx));
  __HAL_DMA_CLEAR_FLAG(imu_spi_v.hdmarx,
                       __HAL_DMA_GET_TE_FLAG_INDEX(imu_spi_v.hdmarx));
  __HAL_DMA_CLEAR_FLAG(imu_spi_v.hdmarx,
                       __HAL_DMA_GET_DME_FLAG_INDEX(imu_spi_v.hdmarx));
  __HAL_DMA_CLEAR_FLAG(imu_spi_v.hdmarx,
                       __HAL_DMA_GET_FE_FLAG_INDEX(imu_spi_v.hdmarx));

  __HAL_DMA_CLEAR_FLAG(imu_spi_v.hdmatx,
                       __HAL_DMA_GET_TC_FLAG_INDEX(imu_spi_v.hdmatx));
  __HAL_DMA_CLEAR_FLAG(imu_spi_v.hdmatx,
                       __HAL_DMA_GET_HT_FLAG_INDEX(imu_spi_v.hdmatx));
  __HAL_DMA_CLEAR_FLAG(imu_spi_v.hdmatx,
                       __HAL_DMA_GET_TE_FLAG_INDEX(imu_spi_v.hdmatx));
  __HAL_DMA_CLEAR_FLAG(imu_spi_v.hdmatx,
                       __HAL_DMA_GET_DME_FLAG_INDEX(imu_spi_v.hdmatx));
  __HAL_DMA_CLEAR_FLAG(imu_spi_v.hdmatx,
                       __HAL_DMA_GET_FE_FLAG_INDEX(imu_spi_v.hdmatx));

  hdma_spi1_rx.Instance->M0AR = rx_buf;
  hdma_spi1_tx.Instance->M0AR = tx_buf;

  __HAL_DMA_SET_COUNTER(&hdma_spi1_rx, ndtr);
  __HAL_DMA_SET_COUNTER(&hdma_spi1_tx, ndtr);

  __HAL_DMA_ENABLE(&hdma_spi1_rx);
  __HAL_DMA_ENABLE(&hdma_spi1_tx);
}

/// 后续会被其他文件复写, 故为虚函数
__weak void DMA2_Stream2_IRQHandler(void) {
  /* DMA2_Stream2 global interrupt handler */
  HAL_DMA_IRQHandler(&hdma_spi1_rx);
}

void DMA2_Stream3_IRQHandler(void) {
  /* DMA2_Stream3 global interrupt handler */
  HAL_DMA_IRQHandler(&hdma_spi1_tx);
}
