#include "loop_task.h"

#include "freertos.h"
#include "ins_task.h"
// #include "remtctrl.h"
#include "type_def.h"

#define PROP 180 * 0.31830988618379067154

static bool_t rcv_ok = false;

void loop_task(void const *args) {
  (void)args;

  // const remtctrl_t *ctrl = getp_remtctrl();

  /* Infinite loop */
  for (;;) {
    TickType_t nowtick = xTaskGetTickCount();

    {
      const fp32 *data = getp_angle_data();
      // uprintf("a=%f,b=%f,c=%f,", data[0] * PROP, data[1] * PROP,
      //         data[2] * PROP);
    }
    vTaskDelayUntil(&nowtick, 100);
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) {
    rcv_ok = true;
  }
}
