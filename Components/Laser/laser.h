#ifndef __LASER_H
#define __LASER_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BUTTON_TRIG_Pin GPIO_PIN_7
#define BUTTON_TRIG_GPIO_Port GPIOI

void laser_init(void);
void laser_on(void);
void laser_off(void);

#ifdef __cplusplus
}
#endif

#endif /* __LASER_H */
