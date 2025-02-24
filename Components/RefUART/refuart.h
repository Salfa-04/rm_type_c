#ifndef __REFUART_H
#define __REFUART_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/// USART6
// extern UART_HandleTypeDef referee_uart_v;

void refuart_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __REFUART_H */
