#include "bmi088.h"

#include "bmi088_reg.h"
#include "imu.h"
#include "type_def.h"

void bmi088_delay_long(void);
void bmi088_delay_short(void);
void imu_bmi088_read(uint8_t tx_data, uint8_t* rx_data);
void bmi088_write(uint8_t reg, uint8_t data);
void bmi088_read_multi(uint8_t reg, uint8_t* data, uint8_t len);
void bmi088_accel_write(uint8_t reg, uint8_t data);
void bmi088_gyro_write(uint8_t reg, uint8_t data);
void bmi088_accel_read(uint8_t reg, uint8_t* data);
void bmi088_gyro_read(uint8_t reg, uint8_t* data);
void bmi088_accel_read_multi(uint8_t reg, uint8_t* data, uint8_t len);
void bmi088_gyro_read_multi(uint8_t reg, uint8_t* data, uint8_t len);

#define BMI088_ACCEL_CMD_NUM 6
#define BMI088_GYRO_CMD_NUM 6

float BMI088_ACCEL_SEN = BMI088_ACCEL_3G_SEN;
float BMI088_GYRO_SEN = BMI088_GYRO_2000_SEN;

const uint8_t bmi088_aclel_init_cmd[BMI088_ACCEL_CMD_NUM][2] = {
    {BMI088_ACC_PWR_CTRL, BMI088_ACC_ENABLE_ACC_ON},
    {BMI088_ACC_PWR_CONF, BMI088_ACC_PWR_ACTIVE_MODE},
    {BMI088_ACC_CONF,
     BMI088_ACC_NORMAL | BMI088_ACC_800_HZ | BMI088_ACC_CONF_MUST_Set},
    {BMI088_ACC_RANGE, BMI088_ACC_RANGE_3G},
    {BMI088_INT1_IO_CTRL, BMI088_ACC_INT1_IO_ENABLE | BMI088_ACC_INT1_GPIO_PP |
                              BMI088_ACC_INT1_GPIO_LOW},
    {BMI088_INT_MAP_DATA, BMI088_ACC_INT1_DRDY_INTERRUPT},
};

const uint8_t bmi088_gyro_init_cmd[BMI088_GYRO_CMD_NUM][2] = {
    {BMI088_GYRO_RANGE, BMI088_GYRO_2000},
    {BMI088_GYRO_BANDWIDTH,
     BMI088_GYRO_1000_116_HZ | BMI088_GYRO_BANDWIDTH_MUST_Set},
    {BMI088_GYRO_LPM1, BMI088_GYRO_NORMAL_MODE},
    {BMI088_GYRO_CTRL, BMI088_DRDY_ON},
    {BMI088_GYRO_INT3_INT4_IO_CONF,
     BMI088_GYRO_INT3_GPIO_PP | BMI088_GYRO_INT3_GPIO_LOW},
    {BMI088_GYRO_INT3_INT4_IO_MAP, BMI088_GYRO_DRDY_IO_INT3},
};

static void bmi088_gpio_init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_Accel_GPIO_Port, CS_Accel_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(CS_Gyro_GPIO_Port, CS_Gyro_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = CS_Accel_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(CS_Accel_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PCPin PCPin */
  GPIO_InitStruct.Pin = INT_Accel_Pin | INT_Gyro_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = CS_Gyro_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(CS_Gyro_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

static bool_t bmi088_accel_init(void) {
  uint8_t res = {0};

  // check commiunication
  bmi088_accel_read(BMI088_ACC_CHIP_ID, &res);
  bmi088_delay_short();
  bmi088_accel_read(BMI088_ACC_CHIP_ID, &res);
  bmi088_delay_short();

  // accel software reset
  bmi088_accel_write(BMI088_ACC_SOFTRESET, BMI088_ACC_SOFTRESET_VALUE);
  bmi088_delay_long();

  // check commiunication is normal after reset
  bmi088_accel_read(BMI088_ACC_CHIP_ID, &res);
  bmi088_delay_short();
  bmi088_accel_read(BMI088_ACC_CHIP_ID, &res);
  bmi088_delay_short();

  // check the "who am I"
  if (res != BMI088_ACC_CHIP_ID_VALUE) PANIC();

  // set accel sonsor config and check
  for (uint8_t writeNum = 0; writeNum < BMI088_ACCEL_CMD_NUM; writeNum++) {
    bmi088_accel_write(bmi088_aclel_init_cmd[writeNum][0],
                       bmi088_aclel_init_cmd[writeNum][1]);
    bmi088_delay_short();

    bmi088_accel_read(bmi088_aclel_init_cmd[writeNum][0], &res);
    bmi088_delay_short();

    if (res != bmi088_aclel_init_cmd[writeNum][1]) PANIC();
  }

  return 0;
}

static bool_t bmi088_gyro_init(void) {
  uint8_t res = {0};

  // check commiunication
  bmi088_gyro_read(BMI088_GYRO_CHIP_ID, &res);
  bmi088_delay_short();
  bmi088_gyro_read(BMI088_GYRO_CHIP_ID, &res);
  bmi088_delay_short();

  // reset the gyro sensor
  bmi088_gyro_write(BMI088_GYRO_SOFTRESET, BMI088_GYRO_SOFTRESET_VALUE);
  bmi088_delay_long();

  // check commiunication is normal after reset
  bmi088_gyro_read(BMI088_GYRO_CHIP_ID, &res);
  bmi088_delay_short();
  bmi088_gyro_read(BMI088_GYRO_CHIP_ID, &res);
  bmi088_delay_short();

  // check the "who am I"
  if (res != BMI088_GYRO_CHIP_ID_VALUE) PANIC();

  // set gyro sonsor config and check
  for (uint8_t writeNum = 0; writeNum < BMI088_GYRO_CMD_NUM; writeNum++) {
    bmi088_gyro_write(bmi088_gyro_init_cmd[writeNum][0],
                      bmi088_gyro_init_cmd[writeNum][1]);
    bmi088_delay_short();

    bmi088_gyro_read(bmi088_gyro_init_cmd[writeNum][0], &res);
    bmi088_delay_short();

    if (res != bmi088_gyro_init_cmd[writeNum][1]) PANIC();
  }

  return 0;
}

bool_t bmi088_init(void) {
  bmi088_gpio_init();

  if (bmi088_accel_init() || bmi088_gyro_init()) {
    return 1;
  }

  return 0;
}

void bmi088_read_accel(accel_t* accel) {
  int16_t temp_raw;
  uint8_t buf[8] = {0};

  bmi088_accel_read_multi(BMI088_ACCEL_XOUT_L, buf, 6);
  temp_raw = (int16_t)((buf[1]) << 8) | buf[0];
  accel->x = temp_raw * BMI088_ACCEL_SEN;
  temp_raw = (int16_t)((buf[3]) << 8) | buf[2];
  accel->y = temp_raw * BMI088_ACCEL_SEN;
  temp_raw = (int16_t)((buf[5]) << 8) | buf[4];
  accel->z = temp_raw * BMI088_ACCEL_SEN;
}

void bmi088_read_gyro(gyro_t* gyro) {
  int16_t temp_raw;
  uint8_t buf[8] = {0};

  bmi088_gyro_read_multi(BMI088_GYRO_CHIP_ID, buf, 8);
  temp_raw = (int16_t)((buf[3]) << 8) | buf[2];
  gyro->x = temp_raw * BMI088_GYRO_SEN;
  temp_raw = (int16_t)((buf[5]) << 8) | buf[4];
  gyro->y = temp_raw * BMI088_GYRO_SEN;
  temp_raw = (int16_t)((buf[7]) << 8) | buf[6];
  gyro->z = temp_raw * BMI088_GYRO_SEN;
}

void bmi088_read_temp(fp32* temp) {
  int16_t temp_raw;
  uint8_t buf[8] = {0};

  bmi088_accel_read_multi(BMI088_TEMP_M, buf, 2);
  temp_raw = (int16_t)((buf[0] << 3) | (buf[1] >> 5));
  if (temp_raw > 1023) temp_raw -= 2048;
  *temp = temp_raw * BMI088_TEMP_FACTOR + BMI088_TEMP_OFFSET;
}

void bmi088_read_all(bmi088_t* bmi088) {
  bmi088_read_accel(&bmi088->accel);
  bmi088_read_gyro(&bmi088->gyro);
  bmi088_read_temp(&bmi088->temp);
}

void bmi088_accel_raw(uint8_t* rx_buf, accel_t* accel, fp32* time) {
  int16_t bmi088_raw_temp;
  uint32_t sensor_time;
  bmi088_raw_temp = (int16_t)((rx_buf[1]) << 8) | rx_buf[0];
  accel->x = bmi088_raw_temp * BMI088_ACCEL_SEN;
  bmi088_raw_temp = (int16_t)((rx_buf[3]) << 8) | rx_buf[2];
  accel->y = bmi088_raw_temp * BMI088_ACCEL_SEN;
  bmi088_raw_temp = (int16_t)((rx_buf[5]) << 8) | rx_buf[4];
  accel->z = bmi088_raw_temp * BMI088_ACCEL_SEN;
  sensor_time = (uint32_t)((rx_buf[8] << 16) | (rx_buf[7] << 8) | rx_buf[6]);
  *time = sensor_time * 39.0625f;
}

void bmi088_gyro_raw(uint8_t* rx_buf, gyro_t* gyro) {
  int16_t bmi088_raw_temp;
  bmi088_raw_temp = (int16_t)((rx_buf[1]) << 8) | rx_buf[0];
  gyro->x = bmi088_raw_temp * BMI088_GYRO_SEN;
  bmi088_raw_temp = (int16_t)((rx_buf[3]) << 8) | rx_buf[2];
  gyro->y = bmi088_raw_temp * BMI088_GYRO_SEN;
  bmi088_raw_temp = (int16_t)((rx_buf[5]) << 8) | rx_buf[4];
  gyro->z = bmi088_raw_temp * BMI088_GYRO_SEN;
}

void bmi088_temp_raw(uint8_t* rx_buf, fp32* temperate) {
  int16_t bmi088_raw_temp;
  bmi088_raw_temp = (int16_t)((rx_buf[0] << 3) | (rx_buf[1] >> 5));
  if (bmi088_raw_temp > 1023) bmi088_raw_temp -= 2048;
  *temperate = bmi088_raw_temp * BMI088_TEMP_FACTOR + BMI088_TEMP_OFFSET;
}

void EXTI4_IRQHandler(void) {
  /* EXTI line4 interrupt Handler */
  HAL_GPIO_EXTI_IRQHandler(INT_Accel_Pin);
}

void EXTI9_5_IRQHandler(void) {
  /* EXTI line[9:5] interrupt Handler */
  HAL_GPIO_EXTI_IRQHandler(INT_Gyro_Pin);
}
