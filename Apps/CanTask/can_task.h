#ifndef __CAN_TASK_H
#define __CAN_TASK_H

#include "stm32f4xx_hal.h"

// 摩擦轮电机 PID
#define FRIC_MAIN_KP 10.f
#define FRIC_MAIN_KI 3.f
#define FRIC_MAIN_KD 0.f
#define FRIC_SUB_KP 10.f
#define FRIC_SUB_KI 3.f
#define FRIC_SUB_KD 0.f
#define FRIC_KS 80
#define FRIC_MAXO 16800.0f
#define FRIC_MAXI 16000.0f

// 拨弹轮电机 PID
#define TRIG_KP 30.0f
#define TRIG_KI 4.0f
#define TRIG_KD 1.0f
#define TRIG_MAXO 16000.0f
#define TRIG_MAXI 2000.0f

#ifdef __cplusplus
extern "C" {
#endif

void can_task(void const *);

void can_fric_forward(void);
void can_fric_reverse(void);
void can_fric_off(void);

void can_trig_on(void);
void can_trig_off(void);
void can_trig_hold(void);
void can_trig_free(void);
void can_trig_back(void);

#ifdef __cplusplus
}
#endif

#endif /* __CAN_TASK_H */
