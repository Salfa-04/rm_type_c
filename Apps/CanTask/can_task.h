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

#define FRIC_PID_MAXO 16800.0f
#define FRIC_PID_MAXI 16000.0f

// 拨弹轮电机 PID
#define TRIG_SPEED_KP 30.0f
#define TRIG_SPEED_KI 4.0f
#define TRIG_SPEED_KD 1.0f
#define TRIG_ANGLE_KP 2500.0f
#define TRIG_ANGLE_KI 16.0f
#define TRIG_ANGLE_KD 9.0f

#define TRIG_SPEED_PID_MAXO 16000.0f
#define TRIG_SPEED_PID_MAXI 2000.0f
#define TRIG_ANGLE_PID_MAXO 80000.0f
#define TRIG_ANGLE_PID_MAXI 10000.0f

#ifdef __cplusplus
extern "C" {
#endif

void can_task(void const *);

void can_fric_forward(void);
void can_fric_reverse(void);
void can_fric_off(void);

#ifdef __cplusplus
}
#endif

#endif /* __CAN_TASK_H */
