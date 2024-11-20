#ifndef __ADC_TASK_H
#define __ADC_TASK_H

#include "stm32f4xx_hal.h"
#include "type_def.h"

#ifdef __cplusplus
extern "C" {
#endif

void adc_task(void const *);

fp32 get_mcu_temperature(void);
uint16_t get_battery_percentage(void);
uint8_t get_hardware_version(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_TASK_H */
