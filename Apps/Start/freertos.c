#include "freertos.h"

#include "cmsis_os.h"
#include "task.h"

osThreadId loopTask;
void loop(void const *);

osThreadId blinkyTask;
void blinky(void const *);

osThreadId deviceTask;
void device(void const *);

osThreadId insTask;
void ins(void const *);

osThreadId detectTask;
void detect(void const *);

void freertos_init(void) {
  /* add mutexes, ... */

  /* add semaphores, ... */

  /* start timers, add new ones, ... */

  /* add queues, ... */

  /* add threads, ... */

  /* definition and creation of loopTask */
  osThreadDef(LoopTask, loop, osPriorityNormal, 0, 128);
  blinkyTask = osThreadCreate(osThread(LoopTask), NULL);

  /* definition and creation of blinkyTask */
  osThreadDef(BlinkyTask, blinky, osPriorityNormal, 0, 128);
  blinkyTask = osThreadCreate(osThread(BlinkyTask), NULL);

  /* definition and creation of deviceTask */
  osThreadDef(DeviceTask, device, osPriorityNormal, 0, 256);
  blinkyTask = osThreadCreate(osThread(DeviceTask), NULL);

  /* definition and creation of InsTask */
  osThreadDef(InsTask, ins, osPriorityNormal, 0, 512);
  blinkyTask = osThreadCreate(osThread(InsTask), NULL);

  /* definition and creation of DetectTask */
  osThreadDef(DetectTask, detect, osPriorityNormal, 0, 256);
  blinkyTask = osThreadCreate(osThread(DetectTask), NULL);
}
