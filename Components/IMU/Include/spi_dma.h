#ifndef __SPI_DMA_H
#define __SPI_DMA_H

#include "type_def.h"

#ifdef __cplusplus
extern "C" {
#endif

void SPI_DMA_init(uint32_t tx_buf, uint32_t rx_buf, uint16_t num);
void SPI_DMA_enable(uint32_t tx_buf, uint32_t rx_buf, uint16_t ndtr);

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __SPI_DMA_H */
