#ifndef __VOLTAGE_TASK_H
#define __VOLTAGE_TASK_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void voltage_task(void const *);
uint16_t get_battery_percentage(void);

#ifdef __cplusplus
}
#endif

#endif /* __VOLTAGE_TASK_H */
