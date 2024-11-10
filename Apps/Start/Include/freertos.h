#ifndef __FREERTOS_H
#define __FREERTOS_H

#include "cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

void freertos_init(void);

extern osThreadId blinkyTask;
extern osThreadId loopTask;
extern osThreadId deviceTask;
extern osThreadId insTask;
extern osThreadId detectTask;

#ifdef __cplusplus
}
#endif

#endif /* __FREERTOS_H */
