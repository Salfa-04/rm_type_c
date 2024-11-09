#ifndef __FREERTOS_H
#define __FREERTOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_os.h"

void freertos_init(void);

extern osThreadId blinkyTask;
extern osThreadId loopTask;
extern osThreadId deviceTask;

#ifdef __cplusplus
}
#endif

#endif /* __FREERTOS_H */
