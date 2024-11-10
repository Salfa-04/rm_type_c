#include "loop.h"

#include "freertos.h"

void loop(void const *args) {
  (void)args;

  /* Infinite loop */
  for (;;) {
    vTaskDelay(3000);
  }
}
