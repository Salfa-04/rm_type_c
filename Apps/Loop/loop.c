#include "loop.h"

#include "arm_math.h"
#include "freertos.h"

void loop(void const *args) {
  (void)args;

  /* Infinite loop */
  for (;;) {
    vTaskDelay(300);
  }
}
