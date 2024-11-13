#ifndef __REFUART_H
#define __REFUART_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/// USART6
extern UART_HandleTypeDef referee_uart_v;

void refuart_init(void);
void refuart_initer(uint8_t *rx1_buf, uint8_t *rx2_buf, uint16_t dma_buf_num);
void refuart_starter(uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __REFUART_H */
