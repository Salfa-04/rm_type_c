#include "bled.h"

TIM_HandleTypeDef htim5;
void bled_gpio_init(void);

static uint8_t alpha;
static uint16_t red, green, blue;

void bled_show(uint32_t aRGB) {
  alpha = (aRGB & 0xFF000000) >> 24;
  red = ((aRGB & 0x00FF0000) >> 16) * alpha;
  green = ((aRGB & 0x0000FF00) >> 8) * alpha;
  blue = (aRGB & 0x000000FF) * alpha;

  __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_1, blue);
  __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_2, green);
  __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_3, red);
}

void bled_init(void) {
  /* TIM5 clock enable */
  __HAL_RCC_TIM5_CLK_ENABLE();

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 0;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 8399;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK) {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim5) != HAL_OK) {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
    Error_Handler();
  }

  bled_gpio_init();
}

void bled_gpio_init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIOH clock enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /**TIM5 GPIO Configuration
  PH12     ------> TIM5_CH3
  PH11     ------> TIM5_CH2
  PH10     ------> TIM5_CH1
  */
  GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_11 | GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  if (HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1) != HAL_OK) {
    Error_Handler();
  };
  if (HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2) != HAL_OK) {
    Error_Handler();
  };
  if (HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_3) != HAL_OK) {
    Error_Handler();
  };
}
