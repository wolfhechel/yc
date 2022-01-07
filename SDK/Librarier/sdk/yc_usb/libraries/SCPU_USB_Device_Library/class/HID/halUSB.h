
#ifndef __halUSB_H__
#define __halUSB_H__
#include <string.h>
#include <stdio.h>
#include "usb_main.h"


#define  NO_DATA         0
#define  HAVE_DATA       1
#define  HAL_OK          0
#define  HAL_ERROR       -1

typedef enum
{
    USB_MODE_HID = 0,
} E_USB_COMM_MODE;





void  halUsbInit(E_USB_COMM_MODE  eMode);
void halUsbDeinit(void);

int32_t halUsbSend(uint8_t *pucSendData, uint32_t uiSendLen);

int32_t halUsbReceive(uint8_t *pucRecData, uint8_t uiRecLen);

uint8_t halUsbDataAck(void);












#endif /* __halUSB_H__ */



