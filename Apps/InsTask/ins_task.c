#include "ins_task.h"

#include "AHRS.h"
#include "adc_task.h"
#include "freertos.h"
#include "imu.h"
#include "pid.h"
#include "spi_dma.h"
#include "type_def.h"

static TaskHandle_t INS_task_local_handler;
static bmi088_t bmi088_data;

extern SPI_HandleTypeDef imu_spi_v;
static void imu_temp_control(fp32 temp);
static void imu_cmd_spi_dma(void);
static void imu_cali_slove(fp32 gyro[3], fp32 accel[3], bmi088_t *bmi088);

#define IMU_temp_PWM(pwm) imu_heat_set(pwm)
// max control temperature of gyro
#define GYRO_CONST_MAX_TEMP 45.0f

#define BMI088_BOARD_INSTALL_SPIN_MATRIX \
  {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 1.0f }

static uint8_t gyro_dma_rx_buf[SPI_DMA_GYRO_LENGHT] = {0};
static uint8_t gyro_dma_tx_buf[SPI_DMA_GYRO_LENGHT] = {
    0x82, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static uint8_t accel_dma_rx_buf[SPI_DMA_ACCEL_LENGHT] = {0};
static uint8_t accel_dma_tx_buf[SPI_DMA_ACCEL_LENGHT] = {
    0x92, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

static uint8_t accel_temp_dma_rx_buf[SPI_DMA_ACCEL_TEMP_LENGHT] = {0};
static uint8_t accel_temp_dma_tx_buf[SPI_DMA_ACCEL_TEMP_LENGHT] = {
    0xA2,
    0xFF,
    0xFF,
    0xFF,
};

static uint8_t gyro_update_flag = 0;
static uint8_t accel_update_flag = 0;
static uint8_t accel_temp_update_flag = 0;
static uint8_t imu_start_dma_flag = 0;

static fp32 gyro_scale_factor[3][3] = {BMI088_BOARD_INSTALL_SPIN_MATRIX};
static fp32 gyro_offset[3] = {0};

static fp32 accel_scale_factor[3][3] = {BMI088_BOARD_INSTALL_SPIN_MATRIX};
static fp32 accel_offset[3] = {0};

static uint8_t first_temperate = 0;
static fp32 imu_temp_PID_int[3] = {0};
static const fp32 imu_temp_PID[3] = {
    TEMPERATURE_PID_KP,
    TEMPERATURE_PID_KI,
    TEMPERATURE_PID_KD,
};

static pid_t imu_temp_pid = {0};
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

const fp32 *getp_angle_data(void) { return INS_angle; }
const fp32 *getp_gyro_data(void) { return INS_gyro; }
const fp32 *getp_accel_data(void) { return INS_accel; }

void ins_task(void const *args) {
  (void)args;

  // wait a time
  vTaskDelay(INS_TASK_INIT_TIME);

  imu_temp_PID_int[0] = imu_temp_PID[0];
  imu_temp_PID_int[1] = imu_temp_PID[1];
  imu_temp_PID_int[2] = imu_temp_PID[2];

  bmi088_read_all(&bmi088_data);
  imu_cali_slove(INS_gyro, INS_accel, &bmi088_data);

  /// 初始化 IMU 温度控制 PID, 有关于陀螺仪零飘
  pid_init(&imu_temp_pid, TEMPERATURE_PID_KP, TEMPERATURE_PID_KI,
           TEMPERATURE_PID_KD, 0, TEMPERATURE_PID_MAX_OUT,
           TEMPERATURE_PID_MAX_IOUT);

  AHRS_init(INS_quat, INS_accel, INS_mag);

  // 滤波初始化
  accel_fliter_1[0] = accel_fliter_2[0] = accel_fliter_3[0] = INS_accel[0];
  accel_fliter_1[1] = accel_fliter_2[1] = accel_fliter_3[1] = INS_accel[1];
  accel_fliter_1[2] = accel_fliter_2[2] = accel_fliter_3[2] = INS_accel[2];

  // 获取任务句柄
  INS_task_local_handler = xTaskGetHandle(pcTaskGetName(NULL));

  // 设置SPI频率
  imu_spi_v.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  if (HAL_SPI_Init(&imu_spi_v) != HAL_OK) Error_Handler();
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

static void imu_temp_control(fp32 imu_temperature) {
  static fp32 temperature = 0;
  static uint8_t temp_constant_time = 0;

  temperature = get_mcu_temperature() + 10.f;
  if (temperature > GYRO_CONST_MAX_TEMP)  // 限制最大温度
    temperature = GYRO_CONST_MAX_TEMP;
  imu_temp_pid.target = temperature;

  if (first_temperate) {
    pid_update(&imu_temp_pid, imu_temperature);
    if (imu_temp_pid.output < 0.0f) imu_temp_pid.output = 0.0f;
    IMU_temp_PWM((uint16_t)imu_temp_pid.output);
  } else {
    if (imu_temperature > temperature) {
      // 达到设置温度，将积分项设置为一半最大功率，加速收敛
      if (++temp_constant_time > 200) {
        first_temperate = 1;
        imu_temp_pid.integral = MPU6500_TEMP_PWM_MAX / 2.0f;
      }
    }

    // 在没有达到设置的温度，一直最大功率加热
    IMU_temp_PWM(MPU6500_TEMP_PWM_MAX - 1);
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == INT_Accel_Pin) {
    // detect_hook(BOARD_ACCEL_TOE);
    accel_update_flag |= 1 << IMU_DR_SHFITS;
    accel_temp_update_flag |= 1 << IMU_DR_SHFITS;
    if (imu_start_dma_flag) imu_cmd_spi_dma();
  } else if (GPIO_Pin == INT_Gyro_Pin) {
    // detect_hook(BOARD_GYRO_TOE);
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
      !(imu_spi_v.hdmatx->Instance->CR & DMA_SxCR_EN) &&
      !(imu_spi_v.hdmarx->Instance->CR & DMA_SxCR_EN) &&
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
      !(imu_spi_v.hdmatx->Instance->CR & DMA_SxCR_EN) &&
      !(imu_spi_v.hdmarx->Instance->CR & DMA_SxCR_EN) &&
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
      !(imu_spi_v.hdmatx->Instance->CR & DMA_SxCR_EN) &&
      !(imu_spi_v.hdmarx->Instance->CR & DMA_SxCR_EN) &&
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
  if (__HAL_DMA_GET_FLAG(imu_spi_v.hdmarx, __HAL_DMA_GET_TC_FLAG_INDEX(
                                               imu_spi_v.hdmarx)) != RESET) {
    __HAL_DMA_CLEAR_FLAG(imu_spi_v.hdmarx,
                         __HAL_DMA_GET_TC_FLAG_INDEX(imu_spi_v.hdmarx));

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
