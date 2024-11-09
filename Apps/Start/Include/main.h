#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

void system_init(void);
void Error_Handler(void);

void uprint(uint8_t *data, uint8_t len);
void uprintf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
