/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usbd_cdc_core.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : Peripheral Device Interface low layer.
 *****************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "usbd_usr.h"


/*********************************************
   CDC Device library callbacks
 *********************************************/
static uint8_t  usbd_cdc_Init(void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_cdc_DeInit(void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_cdc_Setup(void  *pdev, USB_SETUP_REQ *req);
static uint8_t  usbd_cdc_EP0_RxReady(void *pdev);
static uint8_t  usbd_cdc_DataIn(void *pdev, uint8_t epnum);
static uint8_t  usbd_cdc_DataOut(void *pdev, uint8_t epnum);
static uint8_t  usbd_cdc_SOF(void *pdev);

/*********************************************
   CDC specific management functions
 *********************************************/
static void Handle_USBAsynchXfer(void *pdev);
static uint8_t  *USBD_cdc_GetCfgDesc(uint8_t speed, uint16_t *length);
#ifdef USE_USB_OTG_HS
    static uint8_t  *USBD_cdc_GetOtherCfgDesc(uint8_t speed, uint16_t *length);
#endif
/**
  * @}
  */

/** @defgroup usbd_cdc_Private_Variables
  * @{
  */
extern CDC_IF_Prop_TypeDef  APP_FOPS;
extern uint8_t USBD_DeviceDesc   [USB_SIZ_DEVICE_DESC];

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
    #if defined ( __ICCARM__ ) /*!< IAR Compiler */
        #pragma data_alignment=4
    #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN uint8_t usbd_cdc_CfgDesc  [USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END ;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
    #if defined ( __ICCARM__ ) /*!< IAR Compiler */
        #pragma data_alignment=4
    #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN uint8_t usbd_cdc_OtherCfgDesc  [USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END ;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
    #if defined ( __ICCARM__ ) /*!< IAR Compiler */
        #pragma data_alignment=4
    #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN static uint32_t  usbd_cdc_AltSet  __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
    #if defined ( __ICCARM__ ) /*!< IAR Compiler */
        #pragma data_alignment=4
    #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN uint8_t USB_Rx_Buffer   [CDC_DATA_MAX_PACKET_SIZE] __ALIGN_END ;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
    #if defined ( __ICCARM__ ) /*!< IAR Compiler */
        #pragma data_alignment=4
    #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

#ifdef USER_SPECIFIED_DATA_SOURCE
    uint8_t *APP_Rx_Buffer = NULL;
#else
    //__ALIGN_BEGIN uint8_t APP_Rx_Buffer   [APP_RX_DATA_SIZE] __ALIGN_END ;
    struct APP_DATA_STRUCT_DEF APP_Gdata_param;
#endif

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
    #if defined ( __ICCARM__ ) /*!< IAR Compiler */
        #pragma data_alignment=4
    #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN uint8_t CmdBuff[CDC_CMD_PACKET_SZE] __ALIGN_END ;

//volatile uint32_t APP_Rx_ptr_in  = 0;
//volatile uint32_t APP_Rx_ptr_out = 0;
uint32_t APP_Rx_length  = 0;

uint8_t  USB_Tx_State = 0;

uint32_t cdcCmd = 0xFF;
uint32_t cdcLen = 0;

/* CDC interface class callbacks structure */
USBD_Class_cb_TypeDef  USBD_CDC_cb =
{
    usbd_cdc_Init,
    usbd_cdc_DeInit,
    usbd_cdc_Setup,
    NULL,                 /* EP0_TxSent, */
    usbd_cdc_EP0_RxReady,
    usbd_cdc_DataIn,
    usbd_cdc_DataOut,
    usbd_cdc_SOF,
    NULL,
    NULL,
    USBD_cdc_GetCfgDesc,
#ifdef USE_USB_OTG_HS
    USBD_cdc_GetOtherCfgDesc, /* use same cobfig as per FS */
#endif /* USE_USB_OTG_HS  */
};

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
    #if defined ( __ICCARM__ ) /*!< IAR Compiler */
        #pragma data_alignment=4
    #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t usbd_cdc_CfgDesc[USB_CDC_CONFIG_DESC_SIZ]  __ALIGN_END =
{
    /*Configuration Descriptor*/
    0x09,   /* bLength: Configuration Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,      /* bDescriptorType: Configuration */
    USB_CDC_CONFIG_DESC_SIZ,                /* wTotalLength:no of returned bytes */
    0x00,
    0x02,   /* bNumInterfaces: 2 interface */
    0x01,   /* bConfigurationValue: Configuration value */
    0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
    0xC0,   /* bmAttributes: self powered */
    0x32,   /* MaxPower 0 mA */

    /*---------------------------------------------------------------------------*/

    /*Interface Descriptor */
    0x09,   /* bLength: Interface Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: Interface */
    /* Interface descriptor type */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x01,   /* bNumEndpoints: One endpoints used */
    0x02,   /* bInterfaceClass: Communication Interface Class */
    0x02,   /* bInterfaceSubClass: Abstract Control Model */
    0x01,   /* bInterfaceProtocol: Common AT commands */
    0x00,   /* iInterface: */

    /*Header Functional Descriptor*/
    0x05,   /* bLength: Endpoint Descriptor size */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x00,   /* bDescriptorSubtype: Header Func Desc */
    0x10,   /* bcdCDC: spec release number */
    0x01,

    /*Call Management Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x01,   /* bDescriptorSubtype: Call Management Func Desc */
    0x00,   /* bmCapabilities: D0+D1 */
    0x00,   /* bDataInterface: 0 */
//  0x01,   /* bDataInterface: 1 */

    /*ACM Functional Descriptor*/
    0x04,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
    0x02,   /* bmCapabilities */

    /*Union Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x06,   /* bDescriptorSubtype: Union func desc */
    0x00,   /* bMasterInterface: Communication class interface */
    0x01,   /* bSlaveInterface0: Data Class Interface */

    /*Endpoint 2 Descriptor*/
    0x07,                           /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
    CDC_CMD_EP,                     /* bEndpointAddress */
    0x03,                           /* bmAttributes: Interrupt */
    LOBYTE(CDC_CMD_PACKET_SZE),     /* wMaxPacketSize: */
    HIBYTE(CDC_CMD_PACKET_SZE),
#ifdef USE_USB_OTG_HS
    0x10,                           /* bInterval: */
#else
    0xFF,                           /* bInterval: */
#endif /* USE_USB_OTG_HS */

    /*---------------------------------------------------------------------------*/

    /*Data class interface descriptor*/
    0x09,   /* bLength: Endpoint Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: */
    0x01,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x02,   /* bNumEndpoints: Two endpoints used */
    0x0A,   /* bInterfaceClass: CDC */
    0x00,   /* bInterfaceSubClass: */
    0x00,   /* bInterfaceProtocol: */
    0x00,   /* iInterface: */

    /*Endpoint OUT Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
    CDC_OUT_EP,                        /* bEndpointAddress */
    0x02,                              /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
    0x00,                              /* bInterval: ignore for Bulk transfer */

    /*Endpoint IN Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
    CDC_IN_EP,                         /* bEndpointAddress */
    0x02,                              /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
    0x00                               /* bInterval: ignore for Bulk transfer */
} ;


/**
  * @}
  */

/** @defgroup usbd_cdc_Private_Functions
  * @{
  */

/**
  * @brief  usbd_cdc_Init
  *         Initilaize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  usbd_cdc_Init(void  *pdev,
                              uint8_t cfgidx)
{
    uint8_t *pbuf;

    /* Open EP IN */
    DCD_EP_Open(pdev,
                CDC_IN_EP,
                CDC_DATA_IN_PACKET_SIZE,
                USB_OTG_EP_BULK);

    /* Open EP OUT */
    DCD_EP_Open(pdev,
                CDC_OUT_EP,
                CDC_DATA_OUT_PACKET_SIZE,
                USB_OTG_EP_BULK);

    /* Open Command IN EP */
    DCD_EP_Open(pdev,
                CDC_CMD_EP,
                CDC_CMD_PACKET_SZE,
                USB_OTG_EP_INT);

    pbuf = (uint8_t *)USBD_DeviceDesc;
    pbuf[4] = DEVICE_CLASS_CDC;
    pbuf[5] = DEVICE_SUBCLASS_CDC;

    /* Initialize the Interface physical components */
    APP_FOPS.pIf_Init();

    /* Prepare Out endpoint to receive next packet */
    DCD_EP_PrepareRx(pdev,
                     CDC_OUT_EP,
                     (uint8_t *)(USB_Rx_Buffer),
                     CDC_DATA_OUT_PACKET_SIZE);

    return USBD_OK;
}

/**
  * @brief  usbd_cdc_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  usbd_cdc_DeInit(void  *pdev,
                                uint8_t cfgidx)
{
    /* Open EP IN */
    DCD_EP_Close(pdev,
                 CDC_IN_EP);

    /* Open EP OUT */
    DCD_EP_Close(pdev,
                 CDC_OUT_EP);

    /* Open Command IN EP */
    DCD_EP_Close(pdev,
                 CDC_CMD_EP);

    /* Restore default state of the Interface physical components */
    APP_FOPS.pIf_DeInit();

    return USBD_OK;
}

/**
  * @brief  usbd_cdc_Setup
  *         Handle the CDC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  usbd_cdc_Setup(void  *pdev,
                               USB_SETUP_REQ *req)
{
    uint16_t len = USB_CDC_DESC_SIZ;
    uint8_t  *pbuf = usbd_cdc_CfgDesc + 9;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
    /* CDC Class Requests -------------------------------*/
    case USB_REQ_TYPE_CLASS :
        /* Check if the request is a data setup packet */
        if (req->wLength)
        {
            /* Check if the request is Device-to-Host */
            if (req->bmRequest & 0x80)
            {
                /* Get the data to be sent to Host from interface layer */
                APP_FOPS.pIf_Ctrl(req->bRequest, CmdBuff, req->wLength);

                /* Send the data to the host */
                USBD_CtlSendData(pdev,
                                 CmdBuff,
                                 req->wLength);
            }
            else /* Host-to-Device requeset */
            {
                /* Set the value of the current command to be processed */
                cdcCmd = req->bRequest;
                cdcLen = req->wLength;

                /* Prepare the reception of the buffer over EP0
                Next step: the received data will be managed in usbd_cdc_EP0_TxSent()
                function. */
            }
        }
        else /* No Data request */
        {
            /* Transfer the command to the interface layer */
            APP_FOPS.pIf_Ctrl(req->bRequest, NULL, 0);
        }

        return USBD_OK;

    default:
        USBD_CtlError(pdev, req);
        return USBD_FAIL;



    /* Standard Requests -------------------------------*/
    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
        case USB_REQ_GET_DESCRIPTOR:
            if ((req->wValue >> 8) == CDC_DESCRIPTOR_TYPE)
            {
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
                pbuf = usbd_cdc_Desc;
#else
                pbuf = usbd_cdc_CfgDesc + 9 + (9 * USBD_ITF_MAX_NUM);
#endif
                len = MIN(USB_CDC_DESC_SIZ, req->wLength);
            }

            USBD_CtlSendData(pdev,
                             pbuf,
                             len);
            break;

        case USB_REQ_GET_INTERFACE :
            USBD_CtlSendData(pdev,
                             (uint8_t *)&usbd_cdc_AltSet,
                             1);
            break;

        case USB_REQ_SET_INTERFACE :
            if ((uint8_t)(req->wValue) < USBD_ITF_MAX_NUM)
            {
                usbd_cdc_AltSet = (uint8_t)(req->wValue);
            }
            else
            {
                /* Call the error management function (command will be nacked */
                USBD_CtlError(pdev, req);
            }
            break;
        }
    }
    return USBD_OK;
}

/**
  * @brief  usbd_cdc_EP0_RxReady
  *         Data received on control endpoint
  * @param  pdev: device device instance
  * @retval status
  */
static uint8_t  usbd_cdc_EP0_RxReady(void  *pdev)
{
    if (cdcCmd != NO_CMD)
    {
        /* Process the data */
        APP_FOPS.pIf_Ctrl(cdcCmd, CmdBuff, cdcLen);

        /* Reset the command variable to default value */
        cdcCmd = NO_CMD;
    }

    return USBD_OK;
}

/**
  * @brief  usbd_audio_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  usbd_cdc_DataIn(void *pdev, uint8_t epnum)
{
    uint16_t USB_Tx_ptr;
    uint16_t USB_Tx_length = 0;

    if (USB_Tx_State == 1)
    {
        if (APP_Rx_length == 0)
        {
            USB_Tx_State = 0;
        }
        else
        {
            if (APP_Rx_length > CDC_DATA_IN_PACKET_SIZE)
            {
                USB_Tx_ptr = APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
                if (APP_Gdata_param.rx_structure.APP_Rx_ptr_out + CDC_DATA_IN_PACKET_SIZE >= APP_RX_DATA_SIZE)
                {
                    USB_Tx_length = APP_RX_DATA_SIZE - APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
                    APP_Gdata_param.rx_structure.APP_Rx_ptr_out = 0;
                    APP_Rx_length -= USB_Tx_length;
                    APP_Gdata_param.rx_structure.Rx_counter -= USB_Tx_length;
                }
                else
                {
                    USB_Tx_length = CDC_DATA_IN_PACKET_SIZE;
                    APP_Gdata_param.rx_structure.APP_Rx_ptr_out += CDC_DATA_IN_PACKET_SIZE;
                    APP_Rx_length -= CDC_DATA_IN_PACKET_SIZE;
                    APP_Gdata_param.rx_structure.Rx_counter -= CDC_DATA_IN_PACKET_SIZE;
                }
            }
            else
            {
                USB_Tx_ptr = APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
                if (APP_Gdata_param.rx_structure.APP_Rx_ptr_out + APP_Rx_length >= APP_RX_DATA_SIZE)
                {
                    USB_Tx_length = APP_RX_DATA_SIZE - APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
                    APP_Gdata_param.rx_structure.APP_Rx_ptr_out = 0;
                    APP_Rx_length -= USB_Tx_length;
                    APP_Gdata_param.rx_structure.Rx_counter -= USB_Tx_length;
                }
                else
                {
                    USB_Tx_length = APP_Rx_length;
                    APP_Gdata_param.rx_structure.APP_Rx_ptr_out += APP_Rx_length;
                    APP_Gdata_param.rx_structure.Rx_counter -= APP_Rx_length;
                    APP_Rx_length = 0;
                }
            }
        }
        /* Prepare the available data buffer to be sent on IN endpoint */
        DCD_EP_Tx(pdev,
                  CDC_IN_EP,
                  (uint8_t *)&APP_Gdata_param.rx_structure.APP_Rx_Buffer[USB_Tx_ptr],
                  USB_Tx_length);
    }

    return USBD_OK;
}

/**
  * @brief  usbd_cdc_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  usbd_cdc_DataOut(void *pdev, uint8_t epnum)
{
    uint16_t USB_Rx_Cnt;

    /* Get the received data buffer and update the counter */

    /* USB data will be immediately processed, this allow next USB traffic being
       NAKed till the end of the application Xfer */
    /* Prepare Out endpoint to receive next packet */
    DCD_EP_PrepareRx(pdev,
                     CDC_OUT_EP,
                     (uint8_t *)(USB_Rx_Buffer),
                     CDC_DATA_OUT_PACKET_SIZE);

    USB_Rx_Cnt = ((USB_OTG_CORE_HANDLE *)pdev)->dev.out_ep[epnum].xfer_count;

    APP_FOPS.pIf_DataRx(USB_Rx_Buffer, USB_Rx_Cnt);

    return USBD_OK;
}

/**
  * @brief  usbd_audio_SOF
  *         Start Of Frame event management
  * @param  pdev: instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  usbd_cdc_SOF(void *pdev)
{
    static uint32_t FrameCount = 0;

    if (FrameCount++ == CDC_IN_FRAME_INTERVAL)
    {
        /* Reset the frame counter */
        FrameCount = 0;

        /* Check the data to be sent through IN pipe */
        Handle_USBAsynchXfer(pdev);
    }

    return USBD_OK;
}

/**
  * @brief  Handle_USBAsynchXfer
  *         Send data to USB
  * @param  pdev: instance
  * @retval None
  */
static void Handle_USBAsynchXfer(void *pdev)
{
    uint16_t USB_Tx_ptr;
    uint16_t USB_Tx_length;

    if (USB_Tx_State != 1)
    {
        if (APP_Gdata_param.rx_structure.APP_Rx_ptr_out == CDC_APP_RX_DATA_SIZE)
        {
            APP_Gdata_param.rx_structure.APP_Rx_ptr_out = 0;
        }
#if 1
        if (!APP_Gdata_param.rx_structure.Rx_counter)
        {
            USB_Tx_State = 0;
            return;
        }
        APP_Rx_length = APP_Gdata_param.rx_structure.Rx_counter;
#else
        /* */
        if (APP_Gdata_param.rx_structure.APP_Rx_ptr_out == \
                APP_Gdata_param.rx_structure.APP_Rx_ptr_in)
        {
            USB_Tx_State = 0;
            return;
        }


        if (APP_Gdata_param.rx_structure.APP_Rx_ptr_out > APP_Gdata_param.rx_structure.APP_Rx_ptr_in) /* rollback */
        {
            APP_Rx_length = APP_RX_DATA_SIZE - APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
        }
        else
        {
            APP_Rx_length = APP_Gdata_param.rx_structure.APP_Rx_ptr_in - APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
        }
#endif
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
        APP_Rx_length &= ~0x03;
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

        if (APP_Rx_length > CDC_DATA_IN_PACKET_SIZE)
        {
            USB_Tx_ptr = APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
            if (APP_Gdata_param.rx_structure.APP_Rx_ptr_out + CDC_DATA_IN_PACKET_SIZE >= CDC_APP_RX_DATA_SIZE)
            {
                USB_Tx_length = APP_RX_DATA_SIZE - APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
                APP_Gdata_param.rx_structure.APP_Rx_ptr_out = 0;
                APP_Rx_length -= USB_Tx_length;
                APP_Gdata_param.rx_structure.Rx_counter = APP_Rx_length;
            }
            else
            {
                USB_Tx_length = CDC_DATA_IN_PACKET_SIZE;
                APP_Gdata_param.rx_structure.APP_Rx_ptr_out += CDC_DATA_IN_PACKET_SIZE;
                APP_Rx_length -= CDC_DATA_IN_PACKET_SIZE;
                APP_Gdata_param.rx_structure.Rx_counter = APP_Rx_length;
            }
        }
        else
        {
            USB_Tx_ptr = APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
            if (APP_Gdata_param.rx_structure.APP_Rx_ptr_out + APP_Rx_length >= CDC_APP_RX_DATA_SIZE)
            {
                USB_Tx_length = APP_RX_DATA_SIZE - APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
                APP_Gdata_param.rx_structure.APP_Rx_ptr_out = 0;
                APP_Rx_length -= USB_Tx_length;
                APP_Gdata_param.rx_structure.Rx_counter = APP_Rx_length;
            }
            else
            {
                USB_Tx_length = APP_Rx_length;
                APP_Gdata_param.rx_structure.APP_Rx_ptr_out += APP_Rx_length;
                APP_Rx_length = 0;
                APP_Gdata_param.rx_structure.Rx_counter = APP_Rx_length;
            }
        }
        USB_Tx_State = 1;

        DCD_EP_Tx(pdev,
                  CDC_IN_EP,
                  (uint8_t *)&APP_Gdata_param.rx_structure.APP_Rx_Buffer[USB_Tx_ptr],
                  USB_Tx_length);
    }

}

/**
  * @brief  USBD_cdc_GetCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_cdc_GetCfgDesc(uint8_t speed, uint16_t *length)
{
    *length = sizeof(usbd_cdc_CfgDesc);
    return usbd_cdc_CfgDesc;
}



/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
