 
#ifndef __halUSB_H__
#define __halUSB_H__
#include <string.h>
#include <stdio.h>
#include "usb_main.h"
#include "halBasics.h"


typedef enum
{
	USB_MODE_HID = 0,
}E_USB_COMM_MODE;





void  halUsbInit(E_USB_COMM_MODE  eMode);
void halUsbDeinit(void);

s32 halUsbSend(u8 * pucSendData,u32 uiSendLen);

s32 halUsbReceive( u8 *pucRecData,u8 uiRecLen);

u8 halUsbDataAck(void);












#endif /* __halUSB_H__ */



