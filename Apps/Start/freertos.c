#include "freertos.h"

#include "cmsis_os.h"
#include "task.h"

osThreadId blinkyTask;
void blinky(void const *);

void freertos_init(void) {
  /* add mutexes, ... */

  /* add semaphores, ... */

  /* start timers, add new ones, ... */

  /* add queues, ... */

  /* add threads, ... */

  /* definition and creation of blinkyTask */
  osThreadDef(BlinkyTask, blinky, osPriorityNormal, 0, 256);
  blinkyTask = osThreadCreate(osThread(BlinkyTask), NULL);
}
