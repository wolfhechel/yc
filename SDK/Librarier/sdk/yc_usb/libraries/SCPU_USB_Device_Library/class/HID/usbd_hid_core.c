/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usbd_usr.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : Peripheral Device Interface low layer.
 *****************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_dcd_int.h"
#include "usbd_hid_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"


/** @defgroup USBD_HID
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_HID_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_HID_Private_Defines
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_HID_Private_Macros
  * @{
  */
/**
  * @}
  */




/** @defgroup USBD_HID_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_HID_Init(void  *pdev,
                              uint8_t cfgidx);

static uint8_t  USBD_HID_DeInit(void  *pdev,
                                uint8_t cfgidx);

static uint8_t  USBD_HID_Setup(void  *pdev,
                               USB_SETUP_REQ *req);

static uint8_t  *USBD_HID_GetCfgDesc(uint8_t speed, uint16_t *length);

static uint8_t  USBD_HID_DataIn(void  *pdev, uint8_t epnum);
uint8_t USBD_HID_DataOut(void *pdev, uint8_t epnum);
/**
  * @}
  */

/** @defgroup USBD_HID_Private_Variables
  * @{
  */

USBD_Class_cb_TypeDef  USBD_HID_cb =
{
    USBD_HID_Init,
    USBD_HID_DeInit,
    USBD_HID_Setup,
    NULL, /*EP0_TxSent*/
    NULL, /*EP0_RxReady*/
    USBD_HID_DataIn, /*DataIn*/
    USBD_HID_DataOut, /*DataOut*/
    NULL, /*SOF */
    NULL,
    NULL,
    USBD_HID_GetCfgDesc,
#ifdef USB_OTG_HS_CORE
    USBD_HID_GetCfgDesc, /* use same config as per FS */
#endif
};

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
    #if defined ( __ICCARM__ ) /*!< IAR Compiler */
        #pragma data_alignment=4
    #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN static uint32_t  USBD_HID_AltSet  __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
    #if defined ( __ICCARM__ ) /*!< IAR Compiler */
        #pragma data_alignment=4
    #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN static uint32_t  USBD_HID_Protocol  __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
    #if defined ( __ICCARM__ ) /*!< IAR Compiler */
        #pragma data_alignment=4
    #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN static uint32_t  USBD_HID_IdleState __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
    #if defined ( __ICCARM__ ) /*!< IAR Compiler */
        #pragma data_alignment=4
    #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgDesc[USB_HID_CONFIG_DESC_SIZ] __ALIGN_END =
{

    /* 41 */
    0x09,                                 /* bLength: Configuration Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,    /* bDescriptorType: Configuration */
    USB_HID_CONFIG_DESC_SIZ, 0x00,        /* wTotalLength: Bytes returned */
    0x01,                                 /* bNumInterfaces: 1 interface*/
    0x01,                                 /* bConfigurationValue: Configuration value*/
    0x00,                                 /* iConfiguration: Index of string descriptor describing the configuration*/
    0x80,                                 /* bmAttributes: bus powered and Support Remote Wake-up */
    0x32,                                 /* MaxPower 100 mA: this current is used for detecting Vbus*/

    /************** Descriptor of HID interface ****************/
    0x09,                                 /* bLength: Interface Descriptor size*/
    USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType: Interface descriptor type*/
    0x00,                                 /* bInterfaceNumber: Number of Interface*/
    0x00,                                 /* bAlternateSetting: Alternate setting*/
    0x02,                                 /* bNumEndpoints*/
    0x03,                                 /* bInterfaceClass: HID*/
    0x00,                                 /* bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x00,                                 /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0x00,                                 /* iInterface: Index of string descriptor*/

    /******************** Descriptor of HID ********************/
    0x09,                                 /* bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE,                  /* bDescriptorType: HID*/
    0x11, 0x01,                           /* bcdHID: HID Class Spec release number*/
    0x00,                                 /* bCountryCode: Hardware target country*/
    0x01,                                 /* bNumDescriptors: Number of HID class descriptors to follow*/
    0x22,                                 /* bDescriptorType*/
    HID_REPORT_DESC_SIZE,           /* wItemLength: Total length of Report descriptor*/
    0x00,

    /******************** Descriptor of HID IN endpoint ********************/
    0x07,                                 /* bLength: Endpoint Descriptor size*/
    USB_ENDPOINT_DESCRIPTOR_TYPE,         /* bDescriptorType:*/
    HID_IN_EP,                            /* bEndpointAddress: Endpoint Address (IN)*/
    0x03,                                 /* bmAttributes: Interrupt endpoint*/
    HID_IN_PACKET, 0x00,                  /* wMaxPacketSize: 64 Byte max */
    0x01,                                 /* bInterval: Polling Interval (1 ms)*/

    /******************** Descriptor of HID OUT endpoint ********************/
    0x07,                                 /* bLength: Endpoint Descriptor size*/
    USB_ENDPOINT_DESCRIPTOR_TYPE,         /* bDescriptorType:*/
    HID_OUT_EP,                           /* bEndpointAddress: Endpoint Address (OUT)*/
    0x03,                                 /* bmAttributes: Interrupt endpoint*/
    HID_OUT_PACKET, 0x00,                 /* wMaxPacketSize: 64 Byte max */
    0x01,                                 /* bInterval: Polling Interval (1 ms)*/

} ;


__ALIGN_BEGIN static uint8_t HID_ReportDesc[HID_REPORT_DESC_SIZE] __ALIGN_END =
{

    0x05U, 0x81U,           /* Usage Page (Vendor defined)*/
    0x09U, 0x82U,           /* Usage (Vendor defined) */
    0xA1U, 0x01U,           /* Collection (Application) */
    0x09U, 0x83U,           /* Usage (Vendor defined) */
                            
    0x09U, 0x84U,           /* Usage (Vendor defined) */
    0x15U, 0x80U,           /* logical Minimum (-128) */
    0x25U, 0x7FU,           /* logical Maximum (127) */
    0x75U, 0x08U,           /* Report Size (8U) */
    0x95U, HID_IN_PACKET,   /* Report Count (8U) */
    0x81U, 0x02U,           /* Input(Data, Variable, Absolute) */

    0x09U, 0x84U,           /* Usage (Vendor defined) */
    0x15U, 0x80U,           /* logical Minimum (-128) */
    0x25U, 0x7FU,           /* logical Maximum (127) */
    0x75U, 0x08U,           /* Report Size (8U) */
    0x95U, HID_OUT_PACKET,  /* Report Count (8U) */
    0x91U, 0x02U,           /* Input(Data, Variable, Absolute) */
    0xC0U,                  /* end collection */
};
/**
  * @}
  */

/** @defgroup USBD_HID_Private_Functions
  * @{
  */

/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_Init(void  *pdev,
                              uint8_t cfgidx)
{

    /* Open EP IN */
    DCD_EP_Open(pdev,
                HID_IN_EP,
                HID_IN_PACKET,
                USB_OTG_EP_INT);

    /* Open EP OUT */
    DCD_EP_Open(pdev,
                HID_OUT_EP,
                HID_OUT_PACKET,
                USB_OTG_EP_INT);

    return USBD_OK;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_DeInit(void  *pdev,
                                uint8_t cfgidx)
{
    /* Close HID EPs */
    DCD_EP_Close(pdev, HID_IN_EP);
    DCD_EP_Close(pdev, HID_OUT_EP);
    return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */

extern USB_OTG_CORE_HANDLE  USB_OTG_dev;
extern uint8_t hid_core_rcv;
static uint8_t  USBD_HID_Setup(void  *pdev,
                               USB_SETUP_REQ *req)
{
    uint16_t len = 0;
    uint8_t  *pbuf = NULL;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
    case USB_REQ_TYPE_CLASS :
        switch (req->bRequest)
        {


        case HID_REQ_SET_PROTOCOL:
            USBD_HID_Protocol = (uint8_t)(req->wValue);
            break;

        case HID_REQ_GET_PROTOCOL:
            USBD_CtlSendData(pdev,
                             (uint8_t *)&USBD_HID_Protocol,
                             1);
            break;

        case HID_REQ_SET_IDLE:
            USBD_HID_IdleState = (uint8_t)(req->wValue >> 8);
            USB_OTG_dev.dev.device_status_new = USB_OTG_END;
            break;

        case HID_REQ_GET_IDLE:
            USBD_CtlSendData(pdev,
                             (uint8_t *)&USBD_HID_IdleState,
                             1);
            break;
        case HID_REQ_SET_REPORT:
            m_sReportReq.Datainout = 1; //data out
            m_sReportReq.type = (uint8_t)((req->wValue >> 8) & 0xFF);
            m_sReportReq.inf  = (uint8_t)(req->wValue & 0xFF);
            m_sReportReq.len  = (uint8_t)(req->wLength);
            if (hid_core_rcv == 1)
            {
                hid_core_rcv = 0;
                USB_OTG_WRITE_REG8(CORE_USB_STATUS, 0x01);
                m_sReportReq.len = USB_OTG_READ_REG8(CORE_USB_EP_LEN(0));
                if (m_sReportReq.Datainout == 1) //data out
                {
                    m_sReportReq.Datainout = 0;
                    switch (m_sReportReq.type)
                    {
                    case 2: //out data
                        for (int n = 0 ; n < m_sReportReq.len ; n++)
                        {
                            m_sReportBuf.data[n] = USB_OTG_READ_REG8(CORE_USB_EP(0));
                        }
                        USB_OTG_EPReply_Zerolen(pdev, 0);

                        break;
                    case 3: //feature out data//
                        for (int n = 0 ; n < m_sReportReq.len ; n++)
                        {
                            m_sReportBuf.data[n] = USB_OTG_READ_REG8(CORE_USB_EP(0));
                        }
                        USB_OTG_EPReply_Zerolen(pdev, 0);
                        break;
                    }
                }
            }
            break;

        case HID_REQ_GET_REPORT:
            USBD_CtlSendData(pdev,
                             (uint8_t *)&USBD_HID_IdleState,
                             1);
            break;
        default:
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
        }
        break;

    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
        case USB_REQ_GET_DESCRIPTOR:
            if (req->wValue >> 8 == HID_REPORT_DESC)
            {
                len = MIN(HID_REPORT_DESC_SIZE, req->wLength);
                pbuf = HID_ReportDesc;
            }
            else if (req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
            {

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
                pbuf = USBD_HID_Desc;
#else
                pbuf = USBD_HID_CfgDesc + 0x12;
#endif
                len = MIN(USB_HID_DESC_SIZ, req->wLength);
            }

            USBD_CtlSendData(pdev,
                             pbuf,
                             len);

            break;

        case USB_REQ_GET_INTERFACE :
            USBD_CtlSendData(pdev,
                             (uint8_t *)&USBD_HID_AltSet,
                             1);
            break;

        case USB_REQ_SET_INTERFACE :
            USBD_HID_AltSet = (uint8_t)(req->wValue);
            break;
        }
    }
    return USBD_OK;
}

/**
  * @brief  USBD_HID_SendReport
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_HID_SendReport(USB_OTG_CORE_HANDLE  *pdev,
                            uint8_t *report,
                            uint16_t len)
{
    if (pdev->dev.device_status == USB_OTG_CONFIGURED)
    {
        DCD_EP_Tx(pdev, HID_IN_EP, report, len);
    }
    return USBD_OK;
}

/**
  * @brief  USBD_HID_GetCfgDesc
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_HID_GetCfgDesc(uint8_t speed, uint16_t *length)
{
    *length = sizeof(USBD_HID_CfgDesc);
    return USBD_HID_CfgDesc;
}

/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_HID_DataIn(void  *pdev,
                                uint8_t epnum)
{

    /* Ensure that the FIFO is empty before a new transfer, this condition could
    be caused by  a new transfer before the end of the previous transfer */

    return USBD_OK;
}

/**
  * @}
  */

uint8_t receive_data_flag;
uint8_t HidOut_Data_Buff[HID_MAX_PACKET_SIZE];

uint8_t USBD_HID_DataOut(void *pdev,
                         uint8_t epnum)
{
    DCD_EP_PrepareRx(pdev,
                     HID_OUT_EP,
                     (uint8_t *)&HidOut_Data_Buff[0],
                     HID_EPOUT_SIZE);
    receive_data_flag = 1;
    return USBD_OK;
}


/**
  * @}
  */


/**
  * @}
  */

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/

