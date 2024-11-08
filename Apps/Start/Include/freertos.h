#ifndef __FREERTOS_H
#define __FREERTOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_os.h"

extern osThreadId blinkyTask;

void freertos_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __FREERTOS_H */
