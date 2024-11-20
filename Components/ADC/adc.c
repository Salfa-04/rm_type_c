#include "adc.h"

static ADC_HandleTypeDef hadc1;  // MCU temperature
static ADC_HandleTypeDef hadc3;  // battery voltage

#define ADC_BAT_Pin GPIO_PIN_10
#define ADC_BAT_GPIO_Port GPIOF

#define HW_GPIO_Port GPIOC
#define HW0_Pin GPIO_PIN_0
#define HW1_Pin GPIO_PIN_1
#define HW2_Pin GPIO_PIN_2

static volatile fp32 voltage_vrefint_proportion =
    8.0586080586080586080586080586081e-4f;

static void adc_temperature_init(void);
static void adc_battery_init(void);

void adc_init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /*Configure GPIO pins : HW0_Pin HW1_Pin HW2_Pin */
  GPIO_InitStruct.Pin = HW0_Pin | HW1_Pin | HW2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(HW_GPIO_Port, &GPIO_InitStruct);

  adc_temperature_init();
  adc_battery_init();
}

static void adc_ch_change(ADC_HandleTypeDef *ADCx, uint32_t ch) {
  static ADC_ChannelConfTypeDef sConfig = {0};
  sConfig.Channel = ch;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;

  if (HAL_ADC_Stop(ADCx) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_ADC_ConfigChannel(ADCx, &sConfig) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_ADC_Start(ADCx) != HAL_OK) {
    Error_Handler();
  }

  HAL_ADC_PollForConversion(&hadc1, 10);
}

void adc_update_vref(void) {
  adc_ch_change(&hadc1, ADC_CHANNEL_VREFINT);

  uint32_t total_adc = 0;
  for (uint8_t i = 0; i < 200; i++) {
    total_adc += (uint16_t)HAL_ADC_GetValue(&hadc1);
  }

  voltage_vrefint_proportion = 200 * 1.2f / total_adc;
  adc_ch_change(&hadc1, ADC_CHANNEL_TEMPSENSOR);
}

fp32 adc_get_temprate(void) {
  fp32 temperate;
  uint16_t adcx = 0;

  adcx = (uint16_t)HAL_ADC_GetValue(&hadc1);
  temperate = (fp32)adcx * voltage_vrefint_proportion;
  temperate = (temperate - 0.76f) * 400.0f + 25.0f;

  return temperate;
}

fp32 adc_get_voltage(void) {
  fp32 voltage;
  uint16_t adcx = 0;

  adcx = (uint16_t)HAL_ADC_GetValue(&hadc3);
  voltage = (fp32)adcx * voltage_vrefint_proportion *
            10.090909090909090909090909090909f;

  return voltage;
}

uint8_t adc_get_version(void) {
  uint8_t hardware_version;

  hardware_version = HAL_GPIO_ReadPin(HW_GPIO_Port, HW0_Pin) |
                     (HAL_GPIO_ReadPin(HW_GPIO_Port, HW1_Pin) << 1) |
                     (HAL_GPIO_ReadPin(HW_GPIO_Port, HW2_Pin) << 2);

  return hardware_version;
}

static void adc_temperature_init(void) {
  ADC_ChannelConfTypeDef sConfig = {0};

  /* ADC1 clock enable */
  __HAL_RCC_ADC1_CLK_ENABLE();

  /** Configure the global features of the ADC (Clock, Resolution, Data
   * Alignment and number of conversion)
   */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV6;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in
   * the sequencer and its sample time.
   */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
  }
}

static void adc_battery_init(void) {
  ADC_ChannelConfTypeDef sConfig = {0};
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* ADC3 clock enable */
  __HAL_RCC_ADC3_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /**ADC3 GPIO Configuration
  PF10     ------> ADC3_IN8
  */
  GPIO_InitStruct.Pin = ADC_BAT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADC_BAT_GPIO_Port, &GPIO_InitStruct);

  /** Configure the global features of the ADC (Clock, Resolution, Data
   * Alignment and number of conversion)
   */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV6;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc3) != HAL_OK) {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in
   * the sequencer and its sample time.
   */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) {
    Error_Handler();
  }
}
