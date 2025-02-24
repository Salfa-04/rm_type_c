#include "loop_task.h"

#include "can.h"
#include "freertos.h"
#include "type_def.h"

static bool_t rcv_ok = false;

void loop_task(void const *args) {
  (void)args;

  // const remtctrl_t *ctrl = getp_remtctrl();
  // const motor_measure_t *mot[4] = {
  //     getp_mot_chas(0),
  //     getp_mot_chas(1),
  //     getp_mot_chas(2),
  //     getp_mot_chas(3),
  // };

  /* Infinite loop */
  for (;;) {
    TickType_t nowtick = xTaskGetTickCount();

    static uint8_t data[6] = {true, false, 30, 50, false, false};

    if (data[4] || data[5]) {
      can_capci_cmd(data[0], data[1], data[2], data[3]);
      data[4] = false;
    }

    // uprintf("s1=%d,s2=%d,s3=%d,s4=%d,", mot[0]->speed_rpm, mot[1]->speed_rpm,
    //         mot[2]->speed_rpm, mot[3]->speed_rpm);

    vTaskDelayUntil(&nowtick, 100);
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) {
    rcv_ok = true;
  }
}
