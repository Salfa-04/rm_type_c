#include "remtctrl.h"

#include <string.h>

#include "type_def.h"

void remtctrl_uart_init(void);
void remtctrl_gpio_init(void);
void remtctrl_dma_init(void);
void remtctrl_start(void);
void remtctrl_restart(uint16_t dma_buf_num);

// remote control data
static RC_t remt_ctrl = {0};

void remtctrl_init(void) {
  remtctrl_uart_init();
  remtctrl_gpio_init();
  remtctrl_dma_init();
  remtctrl_start();
}

const RC_t *get_remote_control_point(void) { return &remt_ctrl; }
void slove_RC_lost(void) { remtctrl_restart(SBUS_RX_BUF_NUM); }
void slove_data_error(void) { remtctrl_restart(SBUS_RX_BUF_NUM); }

/// 数据重定向
void sbus_to_print(uint8_t *sbus) {
  static uint8_t usart_tx_buf[20];
  static uint8_t i = 0;
  usart_tx_buf[0] = 0xA6;
  memcpy(usart_tx_buf + 1, sbus, 18);
  for (i = 0, usart_tx_buf[19] = 0; i < 19; i++)
    usart_tx_buf[19] += usart_tx_buf[i];

  bool_t warn;

  // usart1_tx_dma_enable(usart_tx_buf, 20);
  //   Error_Handler();

  // uprint(usart_tx_buf, 20);
}
