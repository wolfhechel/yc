/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usb_dcd_int.h
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : Peripheral Device Interface Layer.
 *****************************************************************************/


#ifndef __USB_DCD_INT_H__
#define __USB_DCD_INT_H__


#ifdef __cplusplus
extern "C" {
#endif

/* Include ------------------------------------------------------------------*/
#include "usb_dcd.h"
/* Exported types -----------------------------------------------------------*/
/* Exported constants -------------------------------------------------------*/
/* Exported macro -----------------------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
/* Exported variables -------------------------------------------------------*/
/** @addtogroup USB_OTG_DRIVER
  * @{
  */

/** @defgroup USB_DCD_INT
  * @brief This file is the
  * @{
  */


/** @defgroup USB_DCD_INT_Exported_Defines
  * @{
  */
typedef struct
{
    unsigned char   Datainout;
    unsigned char   type;
    unsigned char   inf;
    unsigned char   len;
} USBD_SETUP_Repot_Typedef;
extern USBD_SETUP_Repot_Typedef  m_sReportReq;

typedef struct
{
    unsigned char    data[64];
} USB_Repot_buf_Typedef;
extern USB_Repot_buf_Typedef     m_sReportBuf;


typedef struct _USBD_DCD_INT
{
    uint8_t (* DataOutStage)(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum);
    uint8_t (* DataInStage)(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum);
    uint8_t (* SetupStage)(USB_OTG_CORE_HANDLE *pdev);
    uint8_t (* SOF)(USB_OTG_CORE_HANDLE *pdev);
    uint8_t (* Reset)(USB_OTG_CORE_HANDLE *pdev);
    uint8_t (* Suspend)(USB_OTG_CORE_HANDLE *pdev);
    uint8_t (* Resume)(USB_OTG_CORE_HANDLE *pdev);
    uint8_t (* IsoINIncomplete)(USB_OTG_CORE_HANDLE *pdev);
    uint8_t (* IsoOUTIncomplete)(USB_OTG_CORE_HANDLE *pdev);

    uint8_t (* DevConnected)(USB_OTG_CORE_HANDLE *pdev);
    uint8_t (* DevDisconnected)(USB_OTG_CORE_HANDLE *pdev);

} USBD_DCD_INT_cb_TypeDef;

extern USBD_DCD_INT_cb_TypeDef *USBD_DCD_INT_fops;
/**
  * @}
  */


/** @defgroup USB_DCD_INT_Exported_Types
  * @{
  */
/**
  * @}
  */

/** @defgroup USB_DCD_INT_Exported_Macros
  * @{
  */
//#define CLEAR_IN_EP_INTR(epnum,intr)
#define CLEAR_IN_EP_INTR(epnum,intr) \
  txcsrl.d8=0; \
  txcsrl.b.intr = 0; \
  USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[epnum]->TXCSRL,txcsrl.d8);

#define CLEAR_OUT_EP_INTR(epnum,intr) \
  rxcsrl.d8=0; \
  rxcsrl.b.intr = 0; \
  USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[epnum]->RXCSRL,rxcsrl.d8);

/**
  * @}
  */

/** @defgroup USB_DCD_INT_Exported_Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup USB_DCD_INT_Exported_FunctionsPrototype
  * @{
  */
uint32_t USBD_OTG_ISR_Handler(USB_OTG_CORE_HANDLE *pdev);

//uint32_t USBD_OTG_DMA_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);
uint32_t USBD_OTG_DMA_ISR_Handler(USB_OTG_CORE_HANDLE *pdev, uint8_t dma_intr_value);

void _delay_(uint32_t t);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USB_DCD_INT_H__ */

/************************ (C) COPYRIGHT 2014 YICHIP ****END OF FILE****/
