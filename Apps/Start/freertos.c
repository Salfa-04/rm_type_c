#include "freertos.h"

#include "adc_task.h"
#include "blinky_task.h"
#include "cmsis_os.h"
#include "loop_task.h"
#include "task.h"

osThreadId loopTask;
osThreadId adcTask;
osThreadId blinkyTask;

void freertos_init(void) {
  /* disable interrupts */
  __set_PRIMASK(1);

  /* add mutexes, ... */

  /* add semaphores, ... */

  /* start timers, add new ones, ... */

  /* add queues, ... */

  /* add threads, ... */

  /* definition and creation of loopTask */
  osThreadDef(LoopTask, loop_task, osPriorityNormal, 0, 512);
  loopTask = osThreadCreate(osThread(LoopTask), NULL);

  /* definition and creation of VoltageTask */
  osThreadDef(AdcTask, adc_task, osPriorityNormal, 0, 128);
  adcTask = osThreadCreate(osThread(AdcTask), NULL);

  /* definition and creation of BlinkyTask */
  osThreadDef(BlinkyTask, blinky_task, osPriorityNormal, 0, 128);
  blinkyTask = osThreadCreate(osThread(BlinkyTask), NULL);
}
