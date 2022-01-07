/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usb_dcd_int.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : Peripheral Device interrupt subroutines.
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "usb_dcd_int.h"
#include "usb_defines.h"
#include "usbd_desc.h"
#include "yc_timer.h"
#include "usbd_usr.h"

/** @addtogroup USB_OTG_DRIVER
* @{
*/

/** @defgroup USB_DCD_INT
* @brief This file contains the interrupt subroutines for the Device mode.
* @{
*/


/** @defgroup USB_DCD_INT_Private_Defines
* @{
*/
/**
* @}
*/

/* Interrupt Handlers */
static uint32_t DCD_HandleInEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint16_t ep_intr);
static uint32_t DCD_HandleOutEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint16_t ep_intr);


void _delay_(uint32_t t)
{
    ((void(*)())(0xc6 + 1))(t);
}

/**
* @brief  USBD_OTG_ISR_Handler
*         handles all USB Interrupts
* @param  pdev: device instance
* @retval status
*/

extern USBD_DCD_INT_cb_TypeDef USBD_DCD_INT_cb;
uint8_t setup_cnt = 0;
uint8_t SetAddress_Flag = 0;
uint8_t Address_Value = 0;
extern volatile uint8_t usb_configration_flag;
uint32_t USBD_OTG_ISR_Handler(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_IRQ1_TypeDef gintr_status;
    USB_OTG_IRQ2_TypeDef fifo_empty_irq;

    uint32_t retval = 0;

    gintr_status.d8 = USB_OTG_READ_REG8(&(pdev ->regs.STATUSEGS ->STATUS));
    fifo_empty_irq.d8 = USB_OTG_READ_REG8(&pdev ->regs.STATUSEGS ->EMPTY);

    if ((!gintr_status.d8) && (!fifo_empty_irq.d8))
        return 0;

    if (gintr_status.b.reset)
    {
        retval |= USBD_DCD_INT_fops->Reset(pdev);
    }

    if (gintr_status.b.suspend)
    {
        retval |= USBD_DCD_INT_fops->Suspend(pdev);
    }
    USB_OTG_WRITE_REG8(&pdev->regs.STATUSEGS ->STATUS, 0xE0);

    if (gintr_status.d8 & 0x0E)
    {
        USB_OTG_WRITE_REG8(&pdev->regs.STATUSEGS ->STATUS, gintr_status.d8 & 0x0E);
        retval |= DCD_HandleOutEP_ISR(pdev, gintr_status.d8 & 0x0E);
    }

    if (gintr_status.b.outep0_irq)
    {
        USB_OTG_WRITE_REG8(&pdev->regs.STATUSEGS ->STATUS, 0x11);
        if (SetAddress_Flag)
        {
            USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS->FADDR, 0, 0x80);  /* set 1,address  0 data can not into fifo*/
            SetAddress_Flag = 0;
        }
        if (gintr_status.b.setup)
        {
            /* ep0 setup packet*/
            USBD_DCD_INT_fops->SetupStage(pdev);
        }
        else
        {
            /* ep0 outpacket, not setup packet*/
        }
    }
    if (fifo_empty_irq.d8 & 0x0F)
    {
        USB_OTG_WRITE_REG8(&pdev->regs.STATUSEGS ->EMPTY, fifo_empty_irq.d8 & 0xFF);
        retval |= DCD_HandleInEP_ISR(pdev, fifo_empty_irq.d8 & 0x0F);
    }
    return retval;
}


/**
* @brief  DCD_HandleInEP_ISR
*         Indicates that an IN EP has a pending Interrupt
* @param  pdev: device instance
* @retval status
*/
static uint32_t DCD_HandleInEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint16_t ep_intr)
{
    USB_OTG_EP *ep;
    uint16_t epnum = 0;

    while (ep_intr)
    {
        ep = &pdev->dev.in_ep[epnum];
        /* Setup and start the Transfer */
        ep->is_in = 1;
        ep->num = epnum;
        if (ep_intr & 0x01) /* In ITR */
        {
            if (pdev->dev.in_ep[epnum].rem_data_len == 0)
            {

                if (pdev->dev.in_ep[epnum].xfer_len != 0 && pdev->dev.in_ep[epnum].xfer_len % pdev->dev.in_ep[epnum].maxpacket == 0)
                {
                    USB_OTG_EPReply_Zerolen(pdev, ep);
                    pdev->dev.in_ep[epnum].xfer_len = 0;
                }
                return 0;
            }
            else
            {
                if (pdev->dev.in_ep[epnum].xfer_len - pdev->dev.in_ep[epnum].xfer_count >= pdev->dev.in_ep[epnum].maxpacket)
                {
                    USB_OTG_WritePacket(pdev,
                                        pdev->dev.in_ep[epnum].xfer_buff + pdev->dev.in_ep[epnum].xfer_count,
                                        epnum,
                                        pdev->dev.in_ep[epnum].maxpacket);
                    USB_OTG_TRIG(pdev, ep);
                    pdev->dev.in_ep[epnum].xfer_count += pdev->dev.in_ep[epnum].maxpacket;
                    pdev->dev.in_ep[epnum].rem_data_len = pdev->dev.in_ep[epnum].xfer_len - pdev->dev.in_ep[epnum].xfer_count;
                    pdev->dev.zero_replay_flag = 1;
                }
                else
                {
                    USB_OTG_WritePacket(pdev,
                                        pdev->dev.in_ep[epnum].xfer_buff + pdev->dev.in_ep[epnum].xfer_count,
                                        epnum,
                                        pdev->dev.in_ep[epnum].xfer_len - pdev->dev.in_ep[epnum].xfer_count);
                    USB_OTG_TRIG(pdev, ep);
                    pdev->dev.in_ep[epnum].xfer_count = pdev->dev.in_ep[epnum].xfer_len;
                    pdev->dev.in_ep[epnum].rem_data_len = 0;
                    /* TX COMPLETE */
                    USBD_DCD_INT_fops->DataInStage(pdev, epnum);
                    pdev->dev.zero_replay_flag = 0;

                }
            }
        }

        epnum++;
        ep_intr >>= 1;
    }
    return 1;
}


/**
* @brief  DCD_HandleOutEP_ISR
*         Indicates that an OUT EP has a pending Interrupt
* @param  pdev: device instance
* @retval status
*/
static uint32_t DCD_HandleOutEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint16_t ep_intr)
{

    uint32_t epnum = 1;
    uint8_t rx_count;
    ep_intr >>= 1;
    while (ep_intr)
    {
        if (ep_intr & 0x1)
        {
            rx_count = USB_OTG_READ_REG8(&pdev ->regs.LENREGS[epnum]);
            if (rx_count)
            {
                USBD_DCD_INT_fops->DataOutStage(pdev, epnum);
            }
        }
        /* Endpoint disable  */
        epnum++;
        ep_intr >>= 1;
    }
    return 1;
}



/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/

