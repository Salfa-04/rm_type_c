#ifndef __USBD_H
#define __USBD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "usbd_cdc_if.h"
#include "usbd_def.h"

void usb_device_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_H */
