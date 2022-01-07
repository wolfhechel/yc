/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usbd_usr.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : Peripheral Device Interface low layer.
 *****************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "usbd_usr.h"
#include "usbd_ioreq.h"

#include "usb_conf.h"


/** @defgroup USBD_USR
* @brief    This file includes the user application layer
* @{
*/

/** @defgroup USBD_USR_Private_TypesDefinitions
* @{
*/
/**
* @}
*/


/** @defgroup USBD_USR_Private_Defines
* @{
*/
/**
* @}
*/


/** @defgroup USBD_USR_Private_Macros
* @{
*/
/**
* @}
*/


/** @defgroup USBD_USR_Private_Variables
* @{
*/

USBD_Usr_cb_TypeDef USRD_cb =
{
    USBD_USR_Init,
    USBD_USR_DeviceReset,
    USBD_USR_DeviceConfigured,
    USBD_USR_DeviceSuspended,
    USBD_USR_DeviceResumed,

    USBD_USR_DeviceConnected,
    USBD_USR_DeviceDisconnected,


};



/**
* @}
*/

/** @defgroup USBD_USR_Private_Constants
* @{
*/

/**
* @}
*/



/** @defgroup USBD_USR_Private_FunctionPrototypes
* @{
*/
/**
* @}
*/


/** @defgroup USBD_USR_Private_Functions
* @{
*/

#define USER_INFORMATION1      "[Key]:RemoteWakeup"
#define USER_INFORMATION2      "[Joystick]:Mouse emulation"


/**
* @brief  USBD_USR_Init
*         Displays the message on LCD for host lib initialization
* @param  None
* @retval None
*/
void USBD_USR_Init(void)
{

}

/**
* @brief  USBD_USR_DeviceReset
*         Displays the message on LCD on device Reset Event
* @param  speed : device speed
* @retval None
*/
extern USB_OTG_CORE_HANDLE  USB_OTG_dev;
extern USBD_DEVICE USR_desc;
extern USBD_Class_cb_TypeDef  USBD_CCID_cb;
void USBD_USR_DeviceReset(uint8_t speed)
{

    USB_OTG_dev.dev.zero_replay_flag = 0;
    memset((uint8_t *)&USB_OTG_dev, 0, sizeof(USB_OTG_dev));
    USB_OTG_dev.dev.device_status = USB_OTG_DEFAULT;

    USBD_Init(&USB_OTG_dev,
#ifdef USE_USB_OTG_HS
              USB_OTG_HS_CORE_ID,
#else
              USB_OTG_FS_CORE_ID,
#endif
              &USR_desc,
              &USBD_CCID_cb,
              &USRD_cb);

    USB_OTG_WRITE_REG8(CORE_USB_CLEAR, 0x40);
    USB_OTG_WRITE_REG8(USB_SOFCNT, 0);
    USB_OTG_WRITE_REG8(USB_SOFCNTHI, 0);

    USB_OTG_WRITE_REG8(CORE_USB_STATUS, 0xFF);
    USB_OTG_WRITE_REG8(CORE_USB_FIFO_EMPTY, 0xFF);
    USB_OTG_WRITE_REG8(CORE_USB_ADDR, 0x00);
    USB_OTG_WRITE_REG8(CORE_USB_INT_MASK(0), 0x70);
    USB_OTG_WRITE_REG8(CORE_USB_INT_MASK(1), 0xf0);
    USB_OTG_WRITE_REG8(CORE_USB_INT_MASK(2), 0xff);
}


/**
* @brief  USBD_USR_DeviceConfigured
*         Displays the message on LCD on device configuration Event
* @param  None
* @retval Staus
*/
void USBD_USR_DeviceConfigured(void)
{
//    MyPrintf("> HID Interface started.\n");
}


/**
* @brief  USBD_USR_DeviceConnected
*         Displays the message on LCD on device connection Event
* @param  None
* @retval Staus
*/
void USBD_USR_DeviceConnected(void)
{
//    MyPrintf("> USB Device Connected.\n");
}


/**
* @brief  USBD_USR_DeviceDisonnected
*         Displays the message on LCD on device disconnection Event
* @param  None
* @retval Staus
*/
void USBD_USR_DeviceDisconnected(void)
{
//    MyPrintf("> USB Device Disconnected.\n");
}

/**
* @brief  USBD_USR_DeviceSuspended
*         Displays the message on LCD on device suspend Event
* @param  None
* @retval None
*/
void USBD_USR_DeviceSuspended(void)
{
//    MyPrintf("\n> USB Device in Suspend Mode.\n");
    /* Users can do their application actions here for the USB-Reset */
}


/**
* @brief  USBD_USR_DeviceResumed
*         Displays the message on LCD on device resume Event
* @param  None
* @retval None
*/
void USBD_USR_DeviceResumed(void)
{
//    MyPrintf("\n> USB Device in Idle Mode.\n");
    /* Users can do their application actions here for the USB-Reset */
}


void my_delay_ms(int ms)
{
    int delay_num;
    while (ms-- > 0)
    {
        delay_num = (CPU_MHZ / 1000) / 8 + 400;
        while (delay_num-- > 0)
        {
            __asm__("nop");
        }
    }
}


/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
