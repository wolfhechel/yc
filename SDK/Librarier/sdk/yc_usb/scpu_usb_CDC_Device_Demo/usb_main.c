/**
  ******************************************************************************
  * @file    main.c
  * @author  yichip
  * @version V1.0.0
  * @date    7-9-2018
  * @brief   This file provides all the Application firmware functions.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/




#include <string.h>
#include <stdio.h>
#include "usb_dcd_int.h"
#include "usbd_cdc_core.h"
#include "usbd_cdc_vcp.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "misc.h"


USB_OTG_CORE_HANDLE  USB_OTG_dev;
volatile unsigned char APP_Tx_ptr_in_count = 0, APP_Tx_ptr_out_count = 0;
volatile uint8_t usbFlowCtrl = 0;
extern struct APP_DATA_STRUCT_DEF APP_Gdata_param;
extern CDC_IF_Prop_TypeDef  APP_FOPS;


void usb_reg_init()
{
    USB_OTG_WRITE_REG8(CORE_USB_CONFIG, 0x00);
    USB_OTG_WRITE_REG8(CORE_USB_TRIG, 0xc0);
    USB_OTG_WRITE_REG8(CORE_USB_CONFIG, 0x3c);
    USB_OTG_WRITE_REG8(CORE_USB_STATUS, 0xFF);
    USB_OTG_WRITE_REG8(CORE_USB_FIFO_EMPTY, 0xFF);
    USB_OTG_WRITE_REG8(CORE_USB_ADDR, 0x00);
    USB_OTG_WRITE_REG8(CORE_USB_INT_MASK(0), 0x70);
    USB_OTG_WRITE_REG8(CORE_USB_INT_MASK(1), 0xf0);
    USB_OTG_WRITE_REG8(CORE_USB_INT_MASK(2), 0xff);
}

uint8_t Rxbuf[1024 * 2] = {0};
uint32_t total_receive_len = 0;
void usb_main(void)
{
    int rxlen = 0;
#if ((VERSIONS == EXIST_BT) || (VERSIONS == NO_BT))
    enable_clock(CLKCLS_BT);
    BT_CLKPLL_EN = 0xff;
    SYSCTRL_HCLK_CON |= 1 << 11;
#elif (VERSIONS == NO_XTAL)

#endif
    enable_clock(CLKCLS_USB);
    usb_reg_init();

    memset(&USB_OTG_dev, 0, sizeof(USB_OTG_dev));

    USBD_Init(&USB_OTG_dev,
              USB_OTG_FS_CORE_ID,
              &USR_desc,
              &USBD_CDC_cb,
              &USRD_cb);

    enable_intr(INTR_USB);

    while (1)
    {

        rxlen = VCP_GetRxBuflen();
        if (rxlen > 0)
        {
            total_receive_len = total_receive_len + rxlen;
            MyPrintf("rxlen =%d total_receive_len = %d \n\r", rxlen, total_receive_len);
            for (int i = 0; i < rxlen; i++)
            {
                Rxbuf[i] = VCP_GetRxChar();
            }
        }

        if (UART_IsRXFIFONotEmpty(UART0))
        {
            uint16_t receiveDataLen = 0;
            uint8_t  receiveBuffer[512] = {0};
            receiveDataLen = UART_ReceiveDataLen(UART0);
            UART_RecvBuf(UART0, receiveBuffer, receiveDataLen);
            DCD_EP_Tx(&USB_OTG_dev, CDC_IN_EP, receiveBuffer, receiveDataLen);
        }
    }
}







void USB_IRQHandler(void)
{
    USBD_OTG_ISR_Handler(&USB_OTG_dev);
}


