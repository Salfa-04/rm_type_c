#include "main.h"

#include "adc.h"
#include "cdc.h"
#include "freertos.h"
#include "remtctrl.h"
#include "type_def.h"

int main(void) {
  system_init();

  // 加载遥控器
  remtctrl_init();
  // 温度、电池电压、硬件版本
  adc_init();
  // 初始化虚拟串口
  usb_cdc_init();
  uprintf("Hello, world!\r\n");

  HAL_Delay(300);
  // 更新参考电压
  adc_update_vref();

  freertos_init();
  osKernelStart();

  for (;;) {
    /* We should never get here */
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM7) {
    HAL_IncTick();
  } /* @type tick = ms */
}
