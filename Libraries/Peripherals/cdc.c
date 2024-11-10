#include "cdc.h"

#include "type_def.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "usbd_core.h"
#include "usbd_def.h"
#include "usbd_desc.h"

USBD_HandleTypeDef hUsbDeviceFS;

void usb_cdc_init(void) {
  /* Init Device Library, add supported class and start the library. */

  if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK) {
    Error_Handler();
  }

  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK) {
    Error_Handler();
  }

  if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) !=
      USBD_OK) {
    Error_Handler();
  }

  if (USBD_Start(&hUsbDeviceFS) != USBD_OK) {
    Error_Handler();
  }
}

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
void OTG_FS_IRQHandler(void) {
  /* USB On The Go FS global interrupt handler. */
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}
