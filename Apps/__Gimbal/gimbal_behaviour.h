#ifndef __GIMBAL_BEHAVIOUR_H
#define __GIMBAL_BEHAVIOUR_H

#include "gimbal_task.h"
#include "type_def.h"

typedef enum {
  GIMBAL_ZERO_FORCE = 0,
  GIMBAL_INIT,
  GIMBAL_CALI,
  GIMBAL_ABSOLUTE_ANGLE,
  GIMBAL_RELATIVE_ANGLE,
  GIMBAL_MOTIONLESS,
} gimbal_behaviour_e;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 * 被gimbal_set_mode函数调用在gimbal_task.c,云台行为状态机以及电机状态机设置
 * @param[out]     gimbal_mode_set: 云台数据指针
 * @retval         none
 */
void gimbal_behaviour_mode_set(gimbal_control_t *gimbal_mode_set);

/**
 * @brief          云台行为控制，根据不同行为采用不同控制函数
 * @param[out]     add_yaw:设置的yaw角度增加值，单位 rad
 * @param[out]     add_pitch:设置的pitch角度增加值，单位 rad
 * @param[in]      gimbal_mode_set:云台数据指针
 * @retval         none
 */
void gimbal_behaviour_control_set(fp32 *add_yaw, fp32 *add_pitch,
                                  gimbal_control_t *gimbal_control_set);

/**
 * @brief          in some gimbal mode, need chassis keep no move
 * @param[in]      none
 * @retval         1: no move 0:normal
 */
/**
 * @brief          云台在某些行为下，需要底盘不动
 * @param[in]      none
 * @retval         1: no move 0:normal
 */

bool_t gimbal_cmd_to_chassis_stop(void);

/**
 * @brief          in some gimbal mode, need shoot keep no move
 * @param[in]      none
 * @retval         1: no move 0:normal
 */
/**
 * @brief          云台在某些行为下，需要射击停止
 * @param[in]      none
 * @retval         1: no move 0:normal
 */
bool_t gimbal_cmd_to_shoot_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* __GIMBAL_BEHAVIOUR_H */
