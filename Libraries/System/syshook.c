#include "cdc.h"
#include "stm32f4xx_hal.h"

void SystemClock_Config(void);

/************************** Init Handler **************************/

void system_init(void) {
  HAL_Init();
  SystemClock_Config();
}

/************************** Error Handler **************************/

void Error_Handler(void) {
  __disable_irq();
  for (;;);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
  /* Wrong parameters value: file on line */
  (void)file, (void)line;
}
#endif /* USE_FULL_ASSERT */

/************************** Print Redirection **************************/

#define __va(x) __builtin_va_##x

// #define Serial huart1
// extern UART_HandleTypeDef huart1;
extern USBD_HandleTypeDef hUsbDeviceFS;

void uprint(uint8_t *data, uint8_t len) {
  // if (Serial.hdmatx->Lock) return;
  // HAL_UART_Transmit_DMA(&Serial, data, len);

  CDC_Transmit_FS(data, len);
}

void uprintf(const char *format, ...) {
  // if (Serial.hdmatx->Lock) return;
  if (((USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData)->TxState != 0) return;

  __va(list) arg;
  __va(start)(arg, format);
  static uint8_t buffer[0xFF] = {0};
  int vsnprintf(char *, unsigned int, const char *, __va(list));
  uint8_t len = vsnprintf((char *)buffer, sizeof(buffer), format, arg);
  __va(end)(arg);

  uprint(buffer, len);
}

/************************** System Clock Config  **************************/

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 6;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
    Error_Handler();
  }
}
