#include "loop_task.h"

#include "freertos.h"
#include "remtctrl.h"
#include "type_def.h"

void loop_task(void const *args) {
  (void)args;

  remtctrl_t *remote = (remtctrl_t *)getp_remtctrl();

  /* Infinite loop */
  for (;;) {
    uprintf("ch1: %d, ch2: %d, ch3: %d, ch4: %d, ch5: %d, s1: %d, s2: %d\r\n",
            remote->rc.ch[0], remote->rc.ch[1], remote->rc.ch[2],
            remote->rc.ch[3], remote->rc.ch[4], remote->rc.s[0],
            remote->rc.s[1]);

    vTaskDelay(100);
  }
}
