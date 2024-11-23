#include "freertos.h"

#include "adc_task.h"
#include "blinky_task.h"
#include "can_task.h"
#include "cmsis_os.h"
#include "ins_task.h"
#include "loop_task.h"
#include "referee_task.h"
#include "task.h"

osThreadId loopTask;
osThreadId adcTask;
osThreadId canTask;
osThreadId blinkyTask;
osThreadId insTask;
osThreadId detectTask;
osThreadId refereeTask;

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

  /* definition and creation of InsTask */
  osThreadDef(InsTask, ins_task, osPriorityNormal, 0, 512);
  insTask = osThreadCreate(osThread(InsTask), NULL);

  /* definition and creation of RefereeTask */
  osThreadDef(RefereeTask, referee_task, osPriorityNormal, 0, 128);
  refereeTask = osThreadCreate(osThread(RefereeTask), NULL);

  /* definition and creation of RefereeTask */
  osThreadDef(CanTask, can_task, osPriorityNormal, 0, 128);
  canTask = osThreadCreate(osThread(CanTask), NULL);
}
