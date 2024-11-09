#include "freertos.h"

#include "cmsis_os.h"
#include "task.h"

osThreadId blinkyTask;
void blinky(void const *);

osThreadId loopTask;
void loop(void const *);

osThreadId deviceTask;
void device(void const *);

void freertos_init(void) {
  /* add mutexes, ... */

  /* add semaphores, ... */

  /* start timers, add new ones, ... */

  /* add queues, ... */

  /* add threads, ... */

  /* definition and creation of blinkyTask */
  osThreadDef(BlinkyTask, blinky, osPriorityNormal, 0, 256);
  blinkyTask = osThreadCreate(osThread(BlinkyTask), NULL);

  /* definition and creation of loopTask */
  osThreadDef(LoopTask, loop, osPriorityNormal, 0, 256);
  blinkyTask = osThreadCreate(osThread(LoopTask), NULL);

  /* definition and creation of deviceTask */
  osThreadDef(DeviceTask, device, osPriorityNormal, 0, 256);
  blinkyTask = osThreadCreate(osThread(DeviceTask), NULL);
}
