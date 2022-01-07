/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usbd_core.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : Peripheral Device Interface low layer.
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "usbd_core.h"
#include "usbd_req.h"
#include "usbd_ioreq.h"
#include "usb_dcd_int.h"
#include "usb_bsp.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Ptivate function prototypes ----------------------------------------------*/


/** @defgroup USBD_CORE_Private_FunctionPrototypes
* @{
*/
static uint8_t USBD_SetupStage(USB_OTG_CORE_HANDLE *pdev);
static uint8_t USBD_DataOutStage(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum);
static uint8_t USBD_DataInStage(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum);
static uint8_t USBD_SOF(USB_OTG_CORE_HANDLE  *pdev);
static uint8_t USBD_Reset(USB_OTG_CORE_HANDLE  *pdev);
static uint8_t USBD_Suspend(USB_OTG_CORE_HANDLE  *pdev);
static uint8_t USBD_Resume(USB_OTG_CORE_HANDLE  *pdev);
#ifdef VBUS_SENSING_ENABLED
static uint8_t USBD_DevConnected(USB_OTG_CORE_HANDLE  *pdev);
static uint8_t USBD_DevDisconnected(USB_OTG_CORE_HANDLE  *pdev);
#endif
static uint8_t USBD_IsoINIncomplete(USB_OTG_CORE_HANDLE  *pdev);
static uint8_t USBD_IsoOUTIncomplete(USB_OTG_CORE_HANDLE  *pdev);
//static uint8_t  USBD_RunTestMode(USB_OTG_CORE_HANDLE  *pdev) ;
/**
* @}
*/



USBD_DCD_INT_cb_TypeDef USBD_DCD_INT_cb =
{
    USBD_DataOutStage,
    USBD_DataInStage,
    USBD_SetupStage,
    USBD_SOF,
    USBD_Reset,
    USBD_Suspend,
    USBD_Resume,
    USBD_IsoINIncomplete,
    USBD_IsoOUTIncomplete,

    USBD_DevConnected,
    USBD_DevDisconnected,
};

USBD_DCD_INT_cb_TypeDef  *USBD_DCD_INT_fops = &USBD_DCD_INT_cb;


/**
* @brief  USBD_SetupStage
*         Handle the setup stage
* @param  pdev: device instance
* @retval status
*/
uint8_t out0_data_len = 0;
static uint8_t USBD_SetupStage(USB_OTG_CORE_HANDLE *pdev)
{
    /* deal with receive setup packet */
    USB_SETUP_REQ req;
    USBD_ParseSetupRequest(pdev, &req);

    switch (req.bmRequest & 0x1F)
    {
    case USB_REQ_RECIPIENT_DEVICE:
        USBD_StdDevReq(pdev, &req);
        break;

    case USB_REQ_RECIPIENT_INTERFACE:
        USBD_StdItfReq(pdev, &req);
        break;

    case USB_REQ_RECIPIENT_ENDPOINT:
        USBD_StdEPReq(pdev, &req);
        break;

    default:
        DCD_EP_Stall(pdev, req.bmRequest & 0x80);
        break;
    }
    return USBD_OK;
}


/**
* @brief  USBD_DataOutStage
*         Handle data out stage
* @param  pdev: device instance
* @param  epnum: endpoint index
* @retval status
*/
static uint8_t USBD_DataOutStage(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum)
{
    USB_OTG_EP *ep;

    if (epnum == 0)
    {
        ep = &pdev->dev.out_ep[0];
        if (pdev->dev.device_state == USB_OTG_EP0_DATA_OUT)
        {
            if (ep->rem_data_len > ep->maxpacket)
            {
                ep->rem_data_len -=  ep->maxpacket;

                if (pdev->cfg.dma_enable == 1)
                {
                    /* in slave mode this, is handled by the RxSTSQLvl ISR */
                    ep->xfer_buff += ep->maxpacket;
                }
                USBD_CtlContinueRx(pdev,
                                   ep->xfer_buff,
                                   MIN(ep->rem_data_len, ep->maxpacket));
            }
            else
            {
                if ((pdev->dev.class_cb->EP0_RxReady != NULL) &&
                        (pdev->dev.device_status == USB_OTG_CONFIGURED))
                {
                    pdev->dev.class_cb->EP0_RxReady(pdev);
                }
                USBD_CtlSendStatus(pdev);
            }
        }
    }
    else if ((pdev->dev.class_cb->DataOut != NULL) &&
             (pdev->dev.device_status == USB_OTG_CONFIGURED))
    {

        pdev->dev.class_cb->DataOut(pdev, epnum);
    }
    return USBD_OK;
}


/**
* @brief  USBD_DataInStage
*         Handle data in stage
* @param  pdev: device instance
* @param  epnum: endpoint index
* @retval status
*/
static uint8_t USBD_DataInStage(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum)
{
    if (epnum == 0)
    {

    }
    else if ((pdev->dev.class_cb->DataIn != NULL) &&
             (pdev->dev.device_status == USB_OTG_CONFIGURED))
    {
        pdev->dev.class_cb->DataIn(pdev, epnum);
    }
    return USBD_OK;
}



void USBD_Init(USB_OTG_CORE_HANDLE *pdev,
               USB_OTG_CORE_ID_TypeDef coreID,
               USBD_DEVICE *pDevice,
               USBD_Class_cb_TypeDef *class_cb,
               USBD_Usr_cb_TypeDef *usr_cb)
{

    /* Hardware Init */
    USBD_DeInit(pdev);

    /*Register class and user callbacks */
    pdev->dev.class_cb = class_cb;
    pdev->dev.usr_cb = usr_cb;
    pdev->dev.usr_device = pDevice;

    /* set USB OTG core params */
    DCD_Init(pdev, coreID);
}



/**
* @brief  USBD_DeInit
*         Re-Initialize th device library
* @param  pdev: device instance
* @retval status: status
*/
USBD_Status USBD_DeInit(USB_OTG_CORE_HANDLE *pdev)
{
    /* Software Init */

    return USBD_OK;
}


static uint8_t USBD_Reset(USB_OTG_CORE_HANDLE  *pdev)
{
    /* Open EP0 OUT */
    DCD_EP_Open(pdev,
                0x00,
                USB_OTG_MAX_EP0_SIZE,
                EP_TYPE_CTRL);

    /* Open EP0 IN */
    DCD_EP_Open(pdev,
                0x80,
                USB_OTG_MAX_EP0_SIZE,
                EP_TYPE_CTRL);

    /* Upon Reset call usr call back */
    pdev->dev.device_status = USB_OTG_DEFAULT;
    pdev->dev.usr_cb->DeviceReset(pdev->cfg.speed);

    return USBD_OK;
}

/**
* @brief  USBD_Resume
*         Handle Resume event
* @param  pdev: device instance
* @retval status
*/

static uint8_t USBD_Resume(USB_OTG_CORE_HANDLE  *pdev)
{
    /* Upon Resume call usr call back */
    pdev->dev.usr_cb->DeviceResumed();
    pdev->dev.device_status = pdev->dev.device_old_status;
    pdev->dev.device_status = USB_OTG_CONFIGURED;
    return USBD_OK;
}


/**
* @brief  USBD_Suspend
*         Handle Suspend event
* @param  pdev: device instance
* @retval status
*/

static uint8_t USBD_Suspend(USB_OTG_CORE_HANDLE  *pdev)
{
    pdev->dev.device_old_status = pdev->dev.device_status;
    pdev->dev.device_status  = USB_OTG_SUSPENDED;
    /* Upon Resume call usr call back */
    pdev->dev.usr_cb->DeviceSuspended();
    return USBD_OK;
}


/**
* @brief  USBD_SOF
*         Handle SOF event
* @param  pdev: device instance
* @retval status
*/

static uint8_t USBD_SOF(USB_OTG_CORE_HANDLE  *pdev)
{
    if (pdev->dev.class_cb->SOF)
    {
        pdev->dev.class_cb->SOF(pdev);
    }
    return USBD_OK;
}
/**
* @brief  USBD_SetCfg
*        Configure device and start the interface
* @param  pdev: device instance
* @param  cfgidx: configuration index
* @retval status
*/

USBD_Status USBD_SetCfg(USB_OTG_CORE_HANDLE  *pdev, uint8_t cfgidx)
{
    pdev->dev.class_cb->Init(pdev, cfgidx);

    /* Upon set config call usr call back */
    pdev->dev.usr_cb->DeviceConfigured();
    return USBD_OK;
}

/**
* @brief  USBD_ClrCfg
*         Clear current configuration
* @param  pdev: device instance
* @param  cfgidx: configuration index
* @retval status: USBD_Status
*/
USBD_Status USBD_ClrCfg(USB_OTG_CORE_HANDLE  *pdev, uint8_t cfgidx)
{
    pdev->dev.class_cb->DeInit(pdev, cfgidx);
    return USBD_OK;
}

/**
* @brief  USBD_IsoINIncomplete
*         Handle iso in incomplete event
* @param  pdev: device instance
* @retval status
*/
static uint8_t USBD_IsoINIncomplete(USB_OTG_CORE_HANDLE  *pdev)
{
    pdev->dev.class_cb->IsoINIncomplete(pdev);
    return USBD_OK;
}

/**
* @brief  USBD_IsoOUTIncomplete
*         Handle iso out incomplete event
* @param  pdev: device instance
* @retval status
*/
static uint8_t USBD_IsoOUTIncomplete(USB_OTG_CORE_HANDLE  *pdev)
{
    pdev->dev.class_cb->IsoOUTIncomplete(pdev);
    return USBD_OK;
}

/**
* @brief  USBD_DevConnected
*         Handle device connection event
* @param  pdev: device instance
* @retval status
*/
static uint8_t USBD_DevConnected(USB_OTG_CORE_HANDLE  *pdev)
{
    pdev->dev.usr_cb->DeviceConnected();
    pdev->dev.connection_status = 1;
    return USBD_OK;
}

/**
* @brief  USBD_DevDisconnected
*         Handle device disconnection event
* @param  pdev: device instance
* @retval status
*/
static uint8_t USBD_DevDisconnected(USB_OTG_CORE_HANDLE  *pdev)
{
    pdev->dev.usr_cb->DeviceDisconnected();
    pdev->dev.class_cb->DeInit(pdev, 0);
    pdev->dev.connection_status = 0;
    return USBD_OK;
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
