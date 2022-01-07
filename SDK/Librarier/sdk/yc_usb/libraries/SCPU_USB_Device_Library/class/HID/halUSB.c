#include <string.h>
#include <stdio.h>
#include "usbd_hid_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_dcd_int.h"
#include "usb_main.h"
#include "halUSB.h"

extern USB_OTG_CORE_HANDLE  USB_OTG_dev;
extern uint8_t receive_data_flag;
extern uint8_t data_len;

void  halUsbInit(E_USB_COMM_MODE  eMode)
{

    if (eMode != USB_MODE_HID)
    {
        return ;
    }

    usb_main();

}


void halUsbDeinit(void)
{
    disable_clock(CLKCLS_USB);
    disable_intr(INTR_USB);

}



int32_t halUsbSend(uint8_t *pucSendData, uint32_t uiSendLen)
{
    if (USB_OTG_dev.dev.device_status_new == USB_OTG_END)
    {
        DCD_EP_Tx(&USB_OTG_dev, HID_IN_EP, pucSendData, uiSendLen);
        return uiSendLen;
    }
    else
    {
        return HAL_ERROR;

    }

}

int32_t halUsbReceive(uint8_t *pucRecData, uint8_t uiRecLen)
{

    if (uiRecLen > HID_EPOUT_SIZE)
    {
        return HAL_ERROR;
    }
    else
    {
        memcpy(pucRecData, HidOut_Data_Buff, data_len);
        //  pucRecData =  (uint8_t *)&HidOut_Data_Buff[0];
        return data_len ;
    }

}

uint8_t halUsbDataAck(void)
{
    if (receive_data_flag == 1)
    {
        receive_data_flag = 0;
        return HAVE_DATA;
    }
    else
    {

        return NO_DATA;
    }


}





















