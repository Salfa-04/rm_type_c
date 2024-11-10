#ifndef __INS_H
#define __INS_H

#include "type_def.h"

#ifdef __cplusplus
extern "C" {
#endif

void ins(void const *);
const fp32 *get_INS_quat_point(void);    /// len 4
const fp32 *get_INS_angle_point(void);   /// len 3
const fp32 *get_gyro_data_point(void);   /// len 3
const fp32 *get_accel_data_point(void);  /// len 3
const fp32 *get_mag_data_point(void);    /// len 3

void gyro_offset_calc(fp32 gyro_offset[3], fp32 gyro[3],
                      uint16_t *offset_time_count);
void INS_cali_gyro(fp32 cali_scale[3], fp32 cali_offset[3],
                   uint16_t *time_count);
void INS_set_cali_gyro(fp32 cali_scale[3], fp32 cali_offset[3]);

#define SPI_DMA_GYRO_LENGHT 8
#define SPI_DMA_ACCEL_LENGHT 9
#define SPI_DMA_ACCEL_TEMP_LENGHT 4

#define IMU_DR_SHFITS 0
#define IMU_SPI_SHFITS 1
#define IMU_UPDATE_SHFITS 2

#define BMI088_GYRO_RX_BUF_DATA_OFFSET 1
#define BMI088_ACCEL_RX_BUF_DATA_OFFSET 2

// ist83100原始数据在缓冲区buf的位置
#define IST8310_RX_BUF_DATA_OFFSET 16

#define TEMPERATURE_PID_KP 1600.0f  // 1800.0//1600.0f //温度控制PID的kp
#define TEMPERATURE_PID_KI 0.1f     // 1.0F//0.2f    //温度控制PID的ki
#define TEMPERATURE_PID_KD 0.05f    // 温度控制PID的kd
#define TEMPERATURE_PID_MAX_OUT 4500.0f   // 温度控制PID的max_out
#define TEMPERATURE_PID_MAX_IOUT 4400.0f  // 温度控制PID的max_iout

// mpu6500控制温度的设置TIM的重载值，即给PWM最大为 MPU6500_TEMP_PWM_MAX-1
#define MPU6500_TEMP_PWM_MAX 5000

#define INS_TASK_INIT_TIME 7  // 任务开始初期 delay 一段时间

#define INS_YAW_ADDRESS_OFFSET 0
#define INS_PITCH_ADDRESS_OFFSET 1
#define INS_ROLL_ADDRESS_OFFSET 2

#define INS_GYRO_X_ADDRESS_OFFSET 0
#define INS_GYRO_Y_ADDRESS_OFFSET 1
#define INS_GYRO_Z_ADDRESS_OFFSET 2

#define INS_ACCEL_X_ADDRESS_OFFSET 0
#define INS_ACCEL_Y_ADDRESS_OFFSET 1
#define INS_ACCEL_Z_ADDRESS_OFFSET 2

#define INS_MAG_X_ADDRESS_OFFSET 0
#define INS_MAG_Y_ADDRESS_OFFSET 1
#define INS_MAG_Z_ADDRESS_OFFSET 2

#ifdef __cplusplus
}
#endif

#endif /* __INS_H */
