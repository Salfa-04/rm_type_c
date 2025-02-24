#ifndef __FSETTING_H
#define __FSETTING_H

#include "stm32f4xx_hal.h"
#include "type_def.h"

#ifdef __cplusplus
extern "C" {
#endif

void fset_save(void *data, int8_t len);
void fset_read(void **ptr);

#ifdef __cplusplus
}
#endif

#endif /* __FSETTING_H */
