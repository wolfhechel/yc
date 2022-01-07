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


#include "usbd_ccid_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include <string.h>
#include <stdio.h>
#include "usb_dcd_int.h"

#define USB_CCID_DEMO

#ifdef USB_CCID_DEMO

USB_OTG_CORE_HANDLE  USB_OTG_dev;

void usb_reg_init()
{
    USB_OTG_WRITE_REG8(CORE_USB_CONFIG, 0x00);
    my_delay_ms(1);
    USB_OTG_WRITE_REG8(CORE_USB_TRIG, 0xc0);
    USB_OTG_WRITE_REG8(CORE_USB_CONFIG, 0x3c);
    USB_OTG_WRITE_REG8(CORE_USB_STATUS, 0xFF);
    USB_OTG_WRITE_REG8(CORE_USB_FIFO_EMPTY, 0xFF);
    USB_OTG_WRITE_REG8(CORE_USB_ADDR, 0x00);
    USB_OTG_WRITE_REG8(CORE_USB_INT_MASK(0), 0x70);
    USB_OTG_WRITE_REG8(CORE_USB_INT_MASK(1), 0xf0);
    USB_OTG_WRITE_REG8(CORE_USB_INT_MASK(2), 0xff);


}

void usb_main(void)
{
#if ((VERSIONS == EXIST_BT) || (VERSIONS == NO_BT))
    enable_clock(CLKCLS_BT);
    BT_CLKPLL_EN = 0xff;
    SYSCTRL_HCLK_CON |= 1 << 11;
#elif (VERSIONS == NO_XTAL)

#endif
    enable_clock(CLKCLS_USB);
    usb_reg_init();

    MyPrintf("YICHIP SCPU USB CCID Demo V1.0.\n");
    memset(&USB_OTG_dev, 0, sizeof(USB_OTG_dev));

    USBD_Init(&USB_OTG_dev,
              USB_OTG_FS_CORE_ID,
              &USR_desc,
              &USBD_CCID_cb,
              &USRD_cb);
    enable_intr(INTR_USB);


    while (1)
    {
        if (USB_OTG_dev.dev.device_status_new == USB_OTG_END)
        {
            CCID_IntMessage(&USB_OTG_dev);
        }
        resp_CCID_CMD(&USB_OTG_dev);
    }


}





void USB_IRQHandler(void)
{
    USBD_OTG_ISR_Handler(&USB_OTG_dev);
//   NVIC_ClearPendingIRQ(USB_IRQn);
//   MyPrintf("YICHIP SCPU USB CCID handler startV1.0.\n");
}


#endif









