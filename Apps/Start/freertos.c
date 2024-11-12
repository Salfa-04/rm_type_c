#include "freertos.h"

#include "blinky_task.h"
#include "cmsis_os.h"
// #include "detect_task.h"
#include "ins_task.h"
#include "loop_task.h"
#include "task.h"
#include "voltage_task.h"

osThreadId loopTask;
osThreadId voltageTask;
osThreadId blinkyTask;
osThreadId insTask;
osThreadId detectTask;

void freertos_init(void) {
  /* add mutexes, ... */

  /* add semaphores, ... */

  /* start timers, add new ones, ... */

  /* add queues, ... */

  /* add threads, ... */

  /* definition and creation of loopTask */
  osThreadDef(LoopTask, loop_task, osPriorityNormal, 0, 128);
  blinkyTask = osThreadCreate(osThread(LoopTask), NULL);

  /* definition and creation of VoltageTask */
  osThreadDef(VoltageTask, voltage_task, osPriorityNormal, 0, 128);
  blinkyTask = osThreadCreate(osThread(VoltageTask), NULL);

  /* definition and creation of blinkyTask */
  osThreadDef(BlinkyTask, blinky_task, osPriorityNormal, 0, 128);
  blinkyTask = osThreadCreate(osThread(BlinkyTask), NULL);

  /* definition and creation of InsTask */
  osThreadDef(InsTask, ins_task, osPriorityNormal, 0, 512);
  blinkyTask = osThreadCreate(osThread(InsTask), NULL);

  // /* definition and creation of DetectTask */
  // osThreadDef(DetectTask, detect_task, osPriorityNormal, 0, 256);
  // blinkyTask = osThreadCreate(osThread(DetectTask), NULL);
}
