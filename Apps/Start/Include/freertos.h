#ifndef __FREERTOS_H
#define __FREERTOS_H

#include "cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

void freertos_init(void);

extern osThreadId loopTask;
extern osThreadId adcTask;
extern osThreadId blinkyTask;
extern osThreadId insTask;
extern osThreadId detectTask;
extern osThreadId refereeTask;

#ifdef __cplusplus
}
#endif

#endif /* __FREERTOS_H */
