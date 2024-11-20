#include "referee_task.h"

#include "freertos.h"
#include "refuart.h"

void referee_task(void const* args) {
  (void)args;

  /* Infinite loop */
  for (;;) {
    vTaskDelay(10000);
  }
}
