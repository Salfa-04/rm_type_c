#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern UART_HandleTypeDef huart1;

void uart1_init(void);
void uprintf(const char *fmt, ...);

void USART1_IRQHandler(void);
void DMA2_Stream7_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H */
