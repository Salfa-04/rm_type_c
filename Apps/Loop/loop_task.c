#include "loop_task.h"

#include "freertos.h"

void loop_task(void const *args) {
  (void)args;

  /* Infinite loop */
  for (;;) {
    vTaskDelay(1000);
  }
}
