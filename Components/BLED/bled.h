#ifndef __BLED_H
#define __BLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

void bled_init(void);
void bled_show(uint32_t aRGB);

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __BLED_H */
