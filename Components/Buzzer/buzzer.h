#ifndef __BUZZER_H
#define __BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

void buzzer_init(void);
void buzzer_on(uint16_t psc, uint16_t pwm);
void buzzer_off(void);

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __BUZZER_H */
