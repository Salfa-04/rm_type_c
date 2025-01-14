#ifndef __CAN_TASK_H
#define __CAN_TASK_H

#include "stm32f4xx_hal.h"

// 摩擦轮电机 PID
#define FRIC_KP 10U
#define FRIC_KI 3U
#define FRIC_KS 80U
#define FRIC_MX 16800U

// 拨弹轮电机 PID
#define TRIG_KP_V 30.0f
#define TRIG_KI_V 4.0f
#define TRIG_KD_V 1.0f
#define TRIG_KP_A 30.0f
#define TRIG_KI_A 4.0f
#define TRIG_KD_A 1.0f
#define TRIG_KS 0
#define TRIG_MX 16800.0f

#ifdef __cplusplus
extern "C" {
#endif

void can_task(void const *);

void can_fric_forward(void);
void can_fric_reverse(void);
void can_fric_off(void);

void can_trig_on(void);
void can_trig_hold(void);
void can_trig_free(void);
void can_trig_back(void);

#ifdef __cplusplus
}
#endif

#endif /* __CAN_TASK_H */
