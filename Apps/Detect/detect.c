#include "detect.h"

#include "cmsis_os.h"

static void detect_init(uint32_t time);

error_t error_list[ERROR_LIST_LENGHT + 1];

#if INCLUDE_uxTaskGetStackHighWaterMark
uint32_t detect_task_stack;
#endif

void detect(void const *args) {
  (void)args;

  static uint32_t system_time;
  system_time = xTaskGetTickCount();
  detect_init(system_time);

  // wait a time.
  vTaskDelay(DETECT_TASK_INIT_TIME);

  while (1) {
    static uint8_t error_num_display = 0;
    system_time = xTaskGetTickCount();

    error_num_display = ERROR_LIST_LENGHT;
    error_list[ERROR_LIST_LENGHT].is_lost = 0;
    error_list[ERROR_LIST_LENGHT].error_exist = 0;

    for (int i = 0; i < ERROR_LIST_LENGHT; i++) {
      // 未使能，跳过
      if (error_list[i].enable == 0) {
        continue;
      }

      // 判断掉线
      if (system_time - error_list[i].new_time >
          error_list[i].set_offline_time) {
        if (error_list[i].error_exist == 0) {
          // 记录错误以及掉线时间
          error_list[i].is_lost = 1;
          error_list[i].error_exist = 1;
          error_list[i].lost_time = system_time;
        }

        // 判断错误优先级， 保存优先级最高的错误码
        if (error_list[i].priority > error_list[error_num_display].priority) {
          error_num_display = i;
        }

        error_list[ERROR_LIST_LENGHT].is_lost = 1;
        error_list[ERROR_LIST_LENGHT].error_exist = 1;

        // 如果提供解决函数，运行解决函数
        if (error_list[i].solve_lost_fun != NULL) {
          error_list[i].solve_lost_fun();
        }
      } else if (system_time - error_list[i].work_time <
                 error_list[i].set_online_time) {
        // 刚刚上线，可能存在数据不稳定，只记录不丢失，
        error_list[i].is_lost = 0;
        error_list[i].error_exist = 1;
      } else {
        error_list[i].is_lost = 0;
        // 判断是否存在数据错误
        if (error_list[i].data_is_error != 0) {
          error_list[i].error_exist = 1;
        } else {
          error_list[i].error_exist = 0;
        }

        // 计算频率
        if (error_list[i].new_time > error_list[i].last_time) {
          error_list[i].frequency =
              configTICK_RATE_HZ /
              (fp32)(error_list[i].new_time - error_list[i].last_time);
        }
      }
    }

    vTaskDelay(DETECT_CONTROL_TIME);
#if INCLUDE_uxTaskGetStackHighWaterMark
    detect_task_stack = uxTaskGetStackHighWaterMark(NULL);
#endif
  }
}

bool_t toe_is_error(uint8_t toe) { return (error_list[toe].error_exist == 1); }

void detect_hook(uint8_t toe) {
  error_list[toe].last_time = error_list[toe].new_time;
  error_list[toe].new_time = xTaskGetTickCount();

  if (error_list[toe].is_lost) {
    error_list[toe].is_lost = 0;
    error_list[toe].work_time = error_list[toe].new_time;
  }

  if (error_list[toe].data_is_error_fun != NULL) {
    if (error_list[toe].data_is_error_fun()) {
      error_list[toe].error_exist = 1;
      error_list[toe].data_is_error = 1;

      if (error_list[toe].solve_data_error_fun != NULL) {
        error_list[toe].solve_data_error_fun();
      }
    } else {
      error_list[toe].data_is_error = 0;
    }
  } else {
    error_list[toe].data_is_error = 0;
  }
}

const error_t *get_error_list_point(void) { return error_list; }

extern void OLED_com_reset(void) {}
static void detect_init(uint32_t time) {
  // 设置离线时间，上线稳定工作时间，优先级 offlineTime onlinetime priority
  uint16_t set_item[ERROR_LIST_LENGHT][3] = {
      {30, 40, 15},   // SBUS
      {10, 10, 11},   // motor1
      {10, 10, 10},   // motor2
      {10, 10, 9},    // motor3
      {10, 10, 8},    // motor4
      {2, 3, 14},     // yaw
      {2, 3, 13},     // pitch
      {10, 10, 12},   // trigger
      {2, 3, 7},      // board gyro
      {5, 5, 7},      // board accel
      {40, 200, 7},   // board mag
      {100, 100, 5},  // referee
      {10, 10, 7},    // rm imu
      {100, 100, 1},  // oled
  };

  for (uint8_t i = 0; i < ERROR_LIST_LENGHT; i++) {
    error_list[i].set_offline_time = set_item[i][0];
    error_list[i].set_online_time = set_item[i][1];
    error_list[i].priority = set_item[i][2];
    error_list[i].data_is_error_fun = NULL;
    error_list[i].solve_lost_fun = NULL;
    error_list[i].solve_data_error_fun = NULL;

    error_list[i].enable = 1;
    error_list[i].error_exist = 1;
    error_list[i].is_lost = 1;
    error_list[i].data_is_error = 1;
    error_list[i].frequency = 0.0f;
    error_list[i].new_time = time;
    error_list[i].last_time = time;
    error_list[i].lost_time = time;
    error_list[i].work_time = time;
  }

  error_list[OLED_TOE].data_is_error_fun = NULL;
  error_list[OLED_TOE].solve_lost_fun = OLED_com_reset;
  error_list[OLED_TOE].solve_data_error_fun = NULL;

  //    error_list[DBUSTOE].dataIsErrorFun = RC_data_is_error;
  //    error_list[DBUSTOE].solveLostFun = slove_RC_lost;
  //    error_list[DBUSTOE].solveDataErrorFun = slove_data_error;
}
