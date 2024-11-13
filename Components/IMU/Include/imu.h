#ifndef __IMU_H
#define __IMU_H

#include "bmi088.h"
#include "main.h"
#include "type_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/// SPI1
extern SPI_HandleTypeDef imu_spi_v;

typedef struct {
  float x, y, z;
} accel_t;

typedef struct {
  float x, y, z;
} gyro_t;

typedef struct {
  accel_t accel;
  gyro_t gyro;
  fp32 temp, time;
} bmi088_t;

bool_t imu_init(void);
void imu_heat_set(uint16_t pwm);
void imu_delay_us(uint16_t nus);

void bmi088_read_accel(accel_t* accel);
void bmi088_read_gyro(gyro_t* gyro);
void bmi088_read_temp(fp32* temp);
void bmi088_read_all(bmi088_t* bmi088);

void bmi088_accel_raw(uint8_t* rx_buf, accel_t* accel, fp32* time);
void bmi088_gyro_raw(uint8_t* rx_buf, gyro_t* gyro);
void bmi088_temp_raw(uint8_t* rx_buf, fp32* temperate);

/// 若陀螺仪有数据返回将会调用该函数
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#define PANIC()                                        \
  do {                                                 \
    uprintf("Paniced at %s:%d\n", __FILE__, __LINE__); \
    return 1;                                          \
  } while (0)

#define CS_Accel_Pin GPIO_PIN_4
#define CS_Accel_GPIO_Port GPIOA
#define CS_Gyro_Pin GPIO_PIN_0
#define CS_Gyro_GPIO_Port GPIOB
#define RST_IST8310_Pin GPIO_PIN_6

#define INT_Accel_Pin GPIO_PIN_4
#define INT_Accel_GPIO_Port GPIOC
#define INT_Accel_EXTI_IRQn EXTI4_IRQn
#define INT_Gyro_Pin GPIO_PIN_5
#define INT_Gyro_GPIO_Port GPIOC
#define INT_Gyro_EXTI_IRQn EXTI9_5_IRQn
#define INT_IST8310_Pin GPIO_PIN_3
#define IST8310_EXTI_IRQn EXTI3_IRQn

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __IMU_H */
