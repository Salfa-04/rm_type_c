#include "adc_task.h"

#include "adc.h"
#include "cmsis_os.h"

#define VOLTAGE_DROP 0.00f

static fp32 calc_battery_percentage(float voltage);
static fp32 battery_voltage = 0;
static fp32 electricity_percentage = 0;
static fp32 mcu_temperature = 0;
static uint8_t hardware_version = 0;

void adc_task(void const* args) {
  (void)args;

  hardware_version = adc_get_version();

  vTaskDelay(1000);

  for (;;) {
    battery_voltage = adc_get_voltage() + VOLTAGE_DROP;
    electricity_percentage = calc_battery_percentage(battery_voltage);
    mcu_temperature = adc_get_temprate();

    vTaskDelay(300);
  }
}

fp32 get_mcu_temperature(void) { return mcu_temperature; }
uint8_t get_hardware_version(void) { return hardware_version; }
uint16_t get_battery_percentage(void) {
  return (uint16_t)(electricity_percentage * 100.0f);
}

static fp32 calc_battery_percentage(fp32 voltage) {
  fp32 percentage;
  fp32 voltage_2 = voltage * voltage;
  fp32 voltage_3 = voltage_2 * voltage;

  if (voltage < 6.6f)
    percentage = 1.0f;
  else if (voltage < 19.5f)
    percentage = 0.0f;
  else if (voltage < 21.9f)
    percentage =
        0.005664f * voltage_3 - 0.3386f * voltage_2 + 6.765f * voltage - 45.17f;
  else if (voltage < 25.5f)
    percentage =
        0.02269f * voltage_3 - 1.654f * voltage_2 + 40.34f * voltage - 328.4f;
  else
    percentage = 1.0f;

  if (percentage < 0.0f)
    percentage = 0.0f;
  else if (percentage > 1.0f)
    percentage = 1.0f;

  return percentage;
}
