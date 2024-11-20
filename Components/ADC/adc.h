#ifndef __ADC_H
#define __ADC_H

#include "stm32f4xx_hal.h"
#include "type_def.h"

#ifdef __cplusplus
extern "C" {
#endif

void adc_init(void);
void adc_update_vref(void);
fp32 adc_get_temprate(void);
fp32 adc_get_voltage(void);
uint8_t adc_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H */
