#include "main.h"

#include "adc.h"
#include "bled.h"
#include "buzzer.h"
#include "can.h"
#include "freertos.h"
#include "laser.h"
#include "refuart.h"
#include "remtctrl.h"
#include "type_def.h"

int main(void) {
  system_init();

  remtctrl_init();  // 加载遥控器
  can_init();       // 初始化CAN、开启CAN中断接收
  adc_init();       // 加载温度、电池电压、硬件版本
  laser_init();     // 激光电源初始化
  buzzer_init();    // 蜂鸣器初始化
  bled_init();      // 板载 LED 初始化
  refuart_init();   // 裁判系统串口初始化

  uprintf("Hello, world!\r\n");
  adc_update_vref();  // 更新参考电压

  freertos_init();
  osKernelStart();

  for (;;); /* We'd never get here */
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM7) {
    HAL_IncTick();
  } /* @type tick = ms */
}
