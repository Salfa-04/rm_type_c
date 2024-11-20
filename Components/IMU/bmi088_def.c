#include "bmi088.h"
#include "imu.h"
#include "stm32f4xx_hal.h"

void imu_bmi088_read(uint8_t tx_data, uint8_t* rx_data);

void bmi088_write(uint8_t reg, uint8_t data) {
  uint8_t temp[] = {0};
  imu_bmi088_read(reg, temp);
  imu_bmi088_read(data, temp);
}

void bmi088_read_multi(uint8_t reg, uint8_t* data, uint8_t len) {
  imu_bmi088_read(reg | 0x80, data);
  for (; len > 0; len--) imu_bmi088_read(0x55, data++);
}

void bmi088_accel_write(uint8_t reg, uint8_t data) {
  HAL_GPIO_WritePin(CS_Accel_GPIO_Port, CS_Accel_Pin, GPIO_PIN_RESET);
  bmi088_write(reg, data);
  HAL_GPIO_WritePin(CS_Accel_GPIO_Port, CS_Accel_Pin, GPIO_PIN_SET);
}

void bmi088_gyro_write(uint8_t reg, uint8_t data) {
  HAL_GPIO_WritePin(CS_Gyro_GPIO_Port, CS_Gyro_Pin, GPIO_PIN_RESET);
  bmi088_write(reg, data);
  HAL_GPIO_WritePin(CS_Gyro_GPIO_Port, CS_Gyro_Pin, GPIO_PIN_SET);
}

void bmi088_accel_read(uint8_t reg, uint8_t* data) {
  HAL_GPIO_WritePin(CS_Accel_GPIO_Port, CS_Accel_Pin, GPIO_PIN_RESET);
  imu_bmi088_read(reg | 0x80, data);
  imu_bmi088_read(0x55, data);
  imu_bmi088_read(0x55, data);
  HAL_GPIO_WritePin(CS_Accel_GPIO_Port, CS_Accel_Pin, GPIO_PIN_SET);
}

void bmi088_gyro_read(uint8_t reg, uint8_t* data) {
  HAL_GPIO_WritePin(CS_Gyro_GPIO_Port, CS_Gyro_Pin, GPIO_PIN_RESET);
  imu_bmi088_read(reg | 0x80, data);
  imu_bmi088_read(0x55, data);
  HAL_GPIO_WritePin(CS_Gyro_GPIO_Port, CS_Gyro_Pin, GPIO_PIN_SET);
}

void bmi088_accel_read_multi(uint8_t reg, uint8_t* data, uint8_t len) {
  HAL_GPIO_WritePin(CS_Accel_GPIO_Port, CS_Accel_Pin, GPIO_PIN_RESET);
  imu_bmi088_read(reg | 0x80, data);
  bmi088_read_multi(reg, data, len);
  HAL_GPIO_WritePin(CS_Accel_GPIO_Port, CS_Accel_Pin, GPIO_PIN_SET);
}

void bmi088_gyro_read_multi(uint8_t reg, uint8_t* data, uint8_t len) {
  HAL_GPIO_WritePin(CS_Gyro_GPIO_Port, CS_Gyro_Pin, GPIO_PIN_RESET);
  bmi088_read_multi(reg, data, len);
  HAL_GPIO_WritePin(CS_Gyro_GPIO_Port, CS_Gyro_Pin, GPIO_PIN_SET);
}

void bmi088_delay_long(void) { HAL_Delay(80); }
void bmi088_delay_short(void) { HAL_Delay(1); }
