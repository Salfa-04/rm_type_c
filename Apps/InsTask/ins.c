#include "ins.h"

#include "AHRS.h"
#include "detect.h"
#include "freertos.h"
#include "imu.h"
#include "pid.h"
#include "spi_dma.h"
#include "type_def.h"

static TaskHandle_t INS_task_local_handler;
static bmi088_t bmi088_data;

extern SPI_HandleTypeDef hspi1;
static void imu_temp_control(fp32 temp);
static void imu_cmd_spi_dma(void);

static void imu_cali_slove(fp32 gyro[3], fp32 accel[3], bmi088_t *bmi088);

#define IMU_temp_PWM(pwm) imu_heat_set(pwm)

#define BMI088_BOARD_INSTALL_SPIN_MATRIX \
  {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }

uint8_t gyro_dma_rx_buf[SPI_DMA_GYRO_LENGHT];
uint8_t gyro_dma_tx_buf[SPI_DMA_GYRO_LENGHT] = {
    0x82, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

uint8_t accel_dma_rx_buf[SPI_DMA_ACCEL_LENGHT];
uint8_t accel_dma_tx_buf[SPI_DMA_ACCEL_LENGHT] = {
    0x92, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

uint8_t accel_temp_dma_rx_buf[SPI_DMA_ACCEL_TEMP_LENGHT];
uint8_t accel_temp_dma_tx_buf[SPI_DMA_ACCEL_TEMP_LENGHT] = {
    0xA2,
    0xFF,
    0xFF,
    0xFF,
};

volatile uint8_t gyro_update_flag = 0;
volatile uint8_t accel_update_flag = 0;
volatile uint8_t accel_temp_update_flag = 0;
volatile uint8_t imu_start_dma_flag = 0;

static fp32 gyro_scale_factor[3][3] = {BMI088_BOARD_INSTALL_SPIN_MATRIX};
static fp32 gyro_offset[3], gyro_cali_offset[3];

static fp32 accel_scale_factor[3][3] = {BMI088_BOARD_INSTALL_SPIN_MATRIX};
static fp32 accel_offset[3];
// static fp32 accel_cali_offset[3];

static uint8_t first_temperate = 0;
static fp32 imu_temp_PID_int[3];
static const fp32 imu_temp_PID[3] = {
    TEMPERATURE_PID_KP,
    TEMPERATURE_PID_KI,
    TEMPERATURE_PID_KD,
};

static pid_type_def imu_temp_pid;
// tast run time , unit s.任务运行的时间 单位 s
static const float timing_time = 0.001f;

// 加速度计低通滤波
static fp32 accel_fliter_1[3] = {0.0f, 0.0f, 0.0f};
static fp32 accel_fliter_2[3] = {0.0f, 0.0f, 0.0f};
static fp32 accel_fliter_3[3] = {0.0f, 0.0f, 0.0f};
static const fp32 fliter_num[3] = {
    1.929454039488895f,
    -0.93178349823448126f,
    0.002329458745586203f,
};

static fp32 INS_gyro[3] = {0.0f, 0.0f, 0.0f};
static fp32 INS_accel[3] = {0.0f, 0.0f, 0.0f};
static fp32 INS_mag[3] = {0.0f, 0.0f, 0.0f};
static fp32 INS_quat[4] = {0.0f, 0.0f, 0.0f, 0.0f};
static fp32 INS_angle[3] = {0.0f, 0.0f, 0.0f};

void ins(void const *args) {
  (void)args;

  // wait a time
  vTaskDelay(INS_TASK_INIT_TIME);

  imu_temp_PID_int[0] = imu_temp_PID[0];
  imu_temp_PID_int[1] = imu_temp_PID[1];
  imu_temp_PID_int[2] = imu_temp_PID[2];

  while (imu_init()) osDelay(300);
  bmi088_read_all(&bmi088_data);
  imu_cali_slove(INS_gyro, INS_accel, &bmi088_data);

  PID_init(&imu_temp_pid, PID_POSITION, imu_temp_PID_int /*imu_temp_PID*/,
           TEMPERATURE_PID_MAX_OUT, TEMPERATURE_PID_MAX_IOUT);
  AHRS_init(INS_quat, INS_accel, INS_mag);

  // 滤波初始化
  accel_fliter_1[0] = accel_fliter_2[0] = accel_fliter_3[0] = INS_accel[0];
  accel_fliter_1[1] = accel_fliter_2[1] = accel_fliter_3[1] = INS_accel[1];
  accel_fliter_1[2] = accel_fliter_2[2] = accel_fliter_3[2] = INS_accel[2];

  // 获取任务句柄
  INS_task_local_handler = xTaskGetHandle(pcTaskGetName(NULL));

  // 设置SPI频率
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  if (HAL_SPI_Init(&hspi1) != HAL_OK) Error_Handler();
  SPI_DMA_init((uint32_t)gyro_dma_tx_buf, (uint32_t)gyro_dma_rx_buf,
               SPI_DMA_GYRO_LENGHT);

  imu_start_dma_flag = 1;

  /* Infinite loop */
  for (;;) {
    // 等待SPI DMA传输
    while (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) != pdPASS);

    if (gyro_update_flag & (1 << IMU_UPDATE_SHFITS)) {
      gyro_update_flag &= ~(1 << IMU_UPDATE_SHFITS);
      bmi088_gyro_raw(gyro_dma_rx_buf + BMI088_GYRO_RX_BUF_DATA_OFFSET,
                      &bmi088_data.gyro);
    }

    if (accel_update_flag & (1 << IMU_UPDATE_SHFITS)) {
      accel_update_flag &= ~(1 << IMU_UPDATE_SHFITS);
      bmi088_accel_raw(accel_dma_rx_buf + BMI088_ACCEL_RX_BUF_DATA_OFFSET,
                       &bmi088_data.accel, &bmi088_data.time);
    }

    if (accel_temp_update_flag & (1 << IMU_UPDATE_SHFITS)) {
      accel_temp_update_flag &= ~(1 << IMU_UPDATE_SHFITS);
      bmi088_temp_raw(accel_temp_dma_rx_buf + BMI088_ACCEL_RX_BUF_DATA_OFFSET,
                      &bmi088_data.temp);
      imu_temp_control(bmi088_data.temp);

      // rotate and zero drift
      imu_cali_slove(INS_gyro, INS_accel, &bmi088_data);

      // 加速度计低通滤波
      accel_fliter_1[0] = accel_fliter_2[0];
      accel_fliter_2[0] = accel_fliter_3[0];
      accel_fliter_3[0] = accel_fliter_2[0] * fliter_num[0] +
                          accel_fliter_1[0] * fliter_num[1] +
                          INS_accel[0] * fliter_num[2];
      accel_fliter_1[1] = accel_fliter_2[1];
      accel_fliter_2[1] = accel_fliter_3[1];
      accel_fliter_3[1] = accel_fliter_2[1] * fliter_num[0] +
                          accel_fliter_1[1] * fliter_num[1] +
                          INS_accel[1] * fliter_num[2];
      accel_fliter_1[2] = accel_fliter_2[2];
      accel_fliter_2[2] = accel_fliter_3[2];
      accel_fliter_3[2] = accel_fliter_2[2] * fliter_num[0] +
                          accel_fliter_1[2] * fliter_num[1] +
                          INS_accel[2] * fliter_num[2];

      // 更新角度
      AHRS_update(INS_quat, timing_time, INS_gyro, accel_fliter_3, INS_mag);
      get_angle(INS_quat, INS_angle + INS_YAW_ADDRESS_OFFSET,
                INS_angle + INS_PITCH_ADDRESS_OFFSET,
                INS_angle + INS_ROLL_ADDRESS_OFFSET);
    }
  }
}

static void imu_cali_slove(fp32 gyro[3], fp32 accel[3], bmi088_t *bmi088) {
  for (uint8_t i = 0; i < 3; i++) {
    gyro[i] = bmi088->gyro.x * gyro_scale_factor[i][0] +
              bmi088->gyro.y * gyro_scale_factor[i][1] +
              bmi088->gyro.z * gyro_scale_factor[i][2] + gyro_offset[i];
    accel[i] = bmi088->accel.x * accel_scale_factor[i][0] +
               bmi088->accel.y * accel_scale_factor[i][1] +
               bmi088->accel.z * accel_scale_factor[i][2] + accel_offset[i];
  }
}

static void imu_temp_control(fp32 temp) {
  (void)temp, (void)first_temperate;
  // uint16_t tempPWM;
  // static uint8_t temp_constant_time = 0;
  // if (first_temperate) {
  //   PID_calc(&imu_temp_pid, temp, get_control_temperature());
  //   if (imu_temp_pid.out < 0.0f) imu_temp_pid.out = 0.0f;
  //   tempPWM = (uint16_t)imu_temp_pid.out;
  //   IMU_temp_PWM(tempPWM);
  // } else {
  //   if (temp > get_control_temperature()) {
  //     // 达到设置温度，将积分项设置为一半最大功率，加速收敛
  //     if (++temp_constant_time > 200) {
  //       first_temperate = 1;
  //       imu_temp_pid.Iout = MPU6500_TEMP_PWM_MAX / 2.0f;
  //     }
  //   }

  //   // 在没有达到设置的温度，一直最大功率加热
  //   IMU_temp_PWM(MPU6500_TEMP_PWM_MAX - 1);
  // }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == INT_Accel_Pin) {
    detect_hook(BOARD_ACCEL_TOE);
    accel_update_flag |= 1 << IMU_DR_SHFITS;
    accel_temp_update_flag |= 1 << IMU_DR_SHFITS;
    if (imu_start_dma_flag) imu_cmd_spi_dma();
  } else if (GPIO_Pin == INT_Gyro_Pin) {
    detect_hook(BOARD_GYRO_TOE);
    gyro_update_flag |= 1 << IMU_DR_SHFITS;
    if (imu_start_dma_flag) imu_cmd_spi_dma();
  } else if (GPIO_Pin == GPIO_PIN_0) {  // 唤醒任务
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
      static BaseType_t xHigherPriorityTaskWoken;
      vTaskNotifyGiveFromISR(INS_task_local_handler, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }
}

static void imu_cmd_spi_dma(void) {
  // 开启陀螺仪的DMA传输
  if ((gyro_update_flag & (1 << IMU_DR_SHFITS)) &&
      !(hspi1.hdmatx->Instance->CR & DMA_SxCR_EN) &&
      !(hspi1.hdmarx->Instance->CR & DMA_SxCR_EN) &&
      !(accel_update_flag & (1 << IMU_SPI_SHFITS)) &&
      !(accel_temp_update_flag & (1 << IMU_SPI_SHFITS))) {
    gyro_update_flag &= ~(1 << IMU_DR_SHFITS);
    gyro_update_flag |= (1 << IMU_SPI_SHFITS);
    HAL_GPIO_WritePin(CS_Gyro_GPIO_Port, CS_Gyro_Pin, GPIO_PIN_RESET);
    SPI_DMA_enable((uint32_t)gyro_dma_tx_buf, (uint32_t)gyro_dma_rx_buf,
                   SPI_DMA_GYRO_LENGHT);
    return;
  }

  // 开启加速度计的DMA传输
  if ((accel_update_flag & (1 << IMU_DR_SHFITS)) &&
      !(hspi1.hdmatx->Instance->CR & DMA_SxCR_EN) &&
      !(hspi1.hdmarx->Instance->CR & DMA_SxCR_EN) &&
      !(gyro_update_flag & (1 << IMU_SPI_SHFITS)) &&
      !(accel_temp_update_flag & (1 << IMU_SPI_SHFITS))) {
    accel_update_flag &= ~(1 << IMU_DR_SHFITS);
    accel_update_flag |= (1 << IMU_SPI_SHFITS);
    HAL_GPIO_WritePin(CS_Accel_GPIO_Port, CS_Accel_Pin, GPIO_PIN_RESET);
    SPI_DMA_enable((uint32_t)accel_dma_tx_buf, (uint32_t)accel_dma_rx_buf,
                   SPI_DMA_ACCEL_LENGHT);
    return;
  }

  if ((accel_temp_update_flag & (1 << IMU_DR_SHFITS)) &&
      !(hspi1.hdmatx->Instance->CR & DMA_SxCR_EN) &&
      !(hspi1.hdmarx->Instance->CR & DMA_SxCR_EN) &&
      !(gyro_update_flag & (1 << IMU_SPI_SHFITS)) &&
      !(accel_update_flag & (1 << IMU_SPI_SHFITS))) {
    accel_temp_update_flag &= ~(1 << IMU_DR_SHFITS);
    accel_temp_update_flag |= (1 << IMU_SPI_SHFITS);

    HAL_GPIO_WritePin(CS_Accel_GPIO_Port, CS_Accel_Pin, GPIO_PIN_RESET);
    SPI_DMA_enable((uint32_t)accel_temp_dma_tx_buf,
                   (uint32_t)accel_temp_dma_rx_buf, SPI_DMA_ACCEL_TEMP_LENGHT);
    return;
  }
}

void DMA2_Stream2_IRQHandler(void) {
  if (__HAL_DMA_GET_FLAG(hspi1.hdmarx,
                         __HAL_DMA_GET_TC_FLAG_INDEX(hspi1.hdmarx)) != RESET) {
    __HAL_DMA_CLEAR_FLAG(hspi1.hdmarx,
                         __HAL_DMA_GET_TC_FLAG_INDEX(hspi1.hdmarx));

    // 陀螺仪读取完毕
    if (gyro_update_flag & (1 << IMU_SPI_SHFITS)) {
      gyro_update_flag &= ~(1 << IMU_SPI_SHFITS);
      gyro_update_flag |= (1 << IMU_UPDATE_SHFITS);
      HAL_GPIO_WritePin(CS_Gyro_GPIO_Port, CS_Gyro_Pin, GPIO_PIN_SET);
    }

    // 加速度计读取完毕
    if (accel_update_flag & (1 << IMU_SPI_SHFITS)) {
      accel_update_flag &= ~(1 << IMU_SPI_SHFITS);
      accel_update_flag |= (1 << IMU_UPDATE_SHFITS);
      HAL_GPIO_WritePin(CS_Accel_GPIO_Port, CS_Accel_Pin, GPIO_PIN_SET);
    }

    // 温度读取完毕
    if (accel_temp_update_flag & (1 << IMU_SPI_SHFITS)) {
      accel_temp_update_flag &= ~(1 << IMU_SPI_SHFITS);
      accel_temp_update_flag |= (1 << IMU_UPDATE_SHFITS);
      HAL_GPIO_WritePin(CS_Accel_GPIO_Port, CS_Accel_Pin, GPIO_PIN_SET);
    }

    imu_cmd_spi_dma();

    if (gyro_update_flag & (1 << IMU_UPDATE_SHFITS)) {
      __HAL_GPIO_EXTI_GENERATE_SWIT(GPIO_PIN_0);
    }
  }
}

const fp32 *get_INS_quat_point(void) { return INS_quat; }
const fp32 *get_INS_angle_point(void) { return INS_angle; }
const fp32 *get_gyro_data_point(void) { return INS_gyro; }
const fp32 *get_accel_data_point(void) { return INS_accel; }

void gyro_offset_calc(fp32 gyro_offset[3], fp32 gyro[3],
                      uint16_t *offset_time_count) {
  if (gyro_offset == NULL || gyro == NULL || offset_time_count == NULL) {
    return;
  }

  gyro_offset[0] = gyro_offset[0] - 0.0003f * gyro[0];
  gyro_offset[1] = gyro_offset[1] - 0.0003f * gyro[1];
  gyro_offset[2] = gyro_offset[2] - 0.0003f * gyro[2];
  (*offset_time_count)++;
}

void INS_cali_gyro(fp32 cali_scale[3], fp32 cali_offset[3],
                   uint16_t *time_count) {
  if (*time_count == 0) {
    gyro_offset[0] = gyro_cali_offset[0];
    gyro_offset[1] = gyro_cali_offset[1];
    gyro_offset[2] = gyro_cali_offset[2];
  }

  gyro_offset_calc(gyro_offset, INS_gyro, time_count);
  cali_offset[0] = gyro_offset[0];
  cali_offset[1] = gyro_offset[1];
  cali_offset[2] = gyro_offset[2];
  cali_scale[0] = 1.0f;
  cali_scale[1] = 1.0f;
  cali_scale[2] = 1.0f;
}

void INS_set_cali_gyro(fp32 cali_scale[3], fp32 cali_offset[3]) {
  (void)cali_scale;
  gyro_cali_offset[0] = cali_offset[0];
  gyro_cali_offset[1] = cali_offset[1];
  gyro_cali_offset[2] = cali_offset[2];
  gyro_offset[0] = gyro_cali_offset[0];
  gyro_offset[1] = gyro_cali_offset[1];
  gyro_offset[2] = gyro_cali_offset[2];
}