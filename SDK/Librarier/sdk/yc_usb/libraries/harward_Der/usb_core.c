/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usb_core.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : USB-OTG Core layer.
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "usb_core.h"
#include "usb_bsp.h"
#include "usb_regs.h"
#include "usbd_usr.h"
#include "yc_timer.h"

/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Ptivate function prototypes ----------------------------------------------*/


/*******************************************************************************
* @brief  USB_OTG_WritePacket : Writes a packet into the Tx FIFO associated
*         with the EP
* @param  pdev : Selected device
* @param  src : source pointer
* @param  ch_ep_num : end point number
* @param  bytes : No. of bytes
* @retval USB_OTG_STS : status
*/

USB_OTG_STS USB_OTG_WritePacket(USB_OTG_CORE_HANDLE *pdev,
                                uint8_t             *src,
                                uint8_t             ch_ep_num,
                                uint8_t            len)
{
    USB_OTG_STS status = USB_OTG_OK;
    uint8_t i = 0;

    if (pdev->cfg.dma_enable == 0)
    {
        for (i = 0; i < len; i++)
        {
            USB_OTG_WRITE_REG8(pdev->regs.ep[ch_ep_num], *src++);
        }
    }

    return status;
}



/**
* @brief  USB_OTG_ReadPacket : Reads a packet from the Rx FIFO
* @param  pdev : Selected device
* @param  dest : Destination Pointer
* @param  bytes : No. of bytes
* @retval None
*/
void USB_OTG_ReadPacket(USB_OTG_CORE_HANDLE *pdev,
                        uint8_t *dest,
                        uint8_t ch_ep_num,
                        uint8_t len)
{
    uint8_t i = 0;
    uint8_t count8b = len ;
    uint8_t *data_buff = (uint8_t *)dest;
    for (i = 0; i < count8b; i++, data_buff++)
    {
        *data_buff = USB_OTG_READ_REG8(CORE_USB_EP(ch_ep_num));
    }
}



/**
* @brief  USB_OTG_SelectCore
*         Initialize core registers address.
* @param  pdev : Selected device
* @param  coreID : USB OTG Core ID
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_SelectCore(USB_OTG_CORE_HANDLE *pdev,
                                USB_OTG_CORE_ID_TypeDef coreID)
{
    uint32_t i = 0;
    USB_OTG_STS status = USB_OTG_OK;
#ifdef  USB_OTG_DMA_ENABLE
    pdev->cfg.dma_enable       = 1;
#else
    pdev->cfg.dma_enable       = 0;
#endif

    /* at startup the core is in FS mode */
    pdev->cfg.speed            = USB_OTG_SPEED_FULL;
    pdev->cfg.mps              = USB_OTG_FS_MAX_PACKET_SIZE ;


    /* initialize device cfg following its address */
    if (coreID == USB_OTG_FS_CORE_ID)
    {
        pdev->cfg.coreID           = USB_OTG_FS_CORE_ID;
        pdev->cfg.dev_endpoints    = 4 ;
        pdev->cfg.TotalFifoSize    = 64; /* in 8-bits */
        pdev->cfg.phy_itface       = USB_OTG_EMBEDDED_PHY;

#ifdef USB_OTG_FS_SOF_OUTPUT_ENABLED
        pdev->cfg.Sof_output       = 1;
#endif

#ifdef USB_OTG_FS_LOW_PWR_MGMT_SUPPORT
        pdev->cfg.low_power        = 1;
#endif
    }
    else if (coreID == USB_OTG_HS_CORE_ID)
    {
        pdev->cfg.coreID           = USB_OTG_HS_CORE_ID;
        pdev->cfg.host_channels    = 8 ;
        pdev->cfg.dev_endpoints    = 4 ;
        pdev->cfg.TotalFifoSize    = 512;/* in 8-bits */

#ifdef USB_OTG_ULPI_PHY_ENABLED
        pdev->cfg.phy_itface       = USB_OTG_ULPI_PHY;
#else
#ifdef USB_OTG_EMBEDDED_PHY_ENABLED
        pdev->cfg.phy_itface       = USB_OTG_EMBEDDED_PHY;
#endif
#endif

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
        pdev->cfg.dma_enable       = 1;
#endif

#ifdef USB_OTG_HS_SOF_OUTPUT_ENABLED
        pdev->cfg.Sof_output       = 1;
#endif

#ifdef USB_OTG_HS_LOW_PWR_MGMT_SUPPORT
        pdev->cfg.low_power        = 1;
#endif
    }

    /* Common USB Registers */
    pdev ->regs.CTRLREGS = (USB_OTG_CTRLREGS *)CORE_USB_CONFIG;
    pdev ->regs.xferctl = (USB_OTG_XFERCTL *)CORE_USB_TRIG;
    /* FIFOS */
    for (i = 0; i < NUM_EP_FIFO; i++)
    {
        pdev->regs.ep[i] = (uint8_t *)(CORE_USB_EP(i));
    }
    /* fifo len*/
    for (i = 0; i < NUM_EP_FIFO; i++)
    {
        pdev->regs.LENREGS[i] = (uint8_t *)(CORE_USB_EP_LEN(i));
    }
    pdev ->regs.STATUSEGS = (USB_OTG_STATUSREGS *)CORE_USB_STALL_STATUS;
    return status;
}


/**
* @brief  USB_OTG_CoreInit
*         Initializes the USB_OTG controller registers and prepares the core
*         device mode or host mode operation.
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_CoreInit(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_STS status = USB_OTG_OK;
    return status;
}



/**
* @brief  USB_OTG_SetCurrentMode : Set ID line
* @param  pdev : Selected device
* @param  mode :  (Host/device)only device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_SetCurrentMode(USB_OTG_CORE_HANDLE *pdev, uint8_t mode)
{
    USB_OTG_STS status = USB_OTG_OK;
    if (mode == HOST_MODE)
    {

    }
    else if (mode == DEVICE_MODE)
    {
#ifdef USE_DEVICE_MODE
        pdev->dev.out_ep[0].xfer_buff = pdev->dev.setup_packet;
        pdev->dev.out_ep[0].xfer_len = 8;
#endif
    }
    delay_us(50);

    return status;
}

/**
* @brief  USB_OTG_EPActivate : Activates an EP
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EPActivate(USB_OTG_CORE_HANDLE *pdev, USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;
    USB_OTG_IRQ_MASK1_TypeDef intr_rxtxe;
    /* Read DEPCTLn register */
    if (ep->is_in == 1)
    {
        intr_rxtxe.d8 = 1 << ep->num;
        USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS->IRQ_MASK1, intr_rxtxe.d8, 0);
    }
    else
    {
        intr_rxtxe.d8 = 1 << ep->num;
        USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS ->IRQ_MASK1, intr_rxtxe.d8, 0);
    }
    return status;
}


/**
* @brief  USB_OTG_EPDeactivate : Deactivates an EP
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EPDeactivate(USB_OTG_CORE_HANDLE *pdev, USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;
    USB_OTG_IRQ_MASK1_TypeDef intr_rxtxe;
    USB_OTG_IRQ_MASK2_TypeDef empty_rxtxe;
    /* Read DEPCTLn register */
    if (ep->is_in == 1)
    {
        empty_rxtxe.d8 = 1 << ep->num;
        USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS ->IRQ_MASK1, 0, empty_rxtxe.d8);
    }
    else
    {
        intr_rxtxe.d8 = 1 << ep->num;
        USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS ->IRQ_MASK1, 0, intr_rxtxe.d8);
    }
    return status;
}



/**
* @brief  USB_OTG_GetMode : Get current mode
* @param  pdev : Selected device
* @retval current mode
*/
uint8_t USB_OTG_GetMode(USB_OTG_CORE_HANDLE *pdev)
{
    return DEVICE_MODE;
}


/**
* @brief  USB_OTG_IsDeviceMode : Check if it is device mode
* @param  pdev : Selected device
* @retval num_in_ep
*/
uint8_t USB_OTG_IsDeviceMode(USB_OTG_CORE_HANDLE *pdev)
{
    return (USB_OTG_GetMode(pdev) != HOST_MODE);
}
/**
* @brief  USB_OTG_EP0StartXfer : Handle the setup for a data xfer for EP0 and
*         starts the xfer
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
extern uint8_t out0_data_len;
USB_OTG_STS USB_OTG_EP0StartXfer(USB_OTG_CORE_HANDLE *pdev, USB_OTG_EP *ep)
{
    USB_OTG_STS  status = USB_OTG_OK;
    uint8_t rx_count;

    /* IN endpoint */
    if (ep->is_in == 1)
    {
        ep->rem_data_len = ep->xfer_len - ep->xfer_count;
        /* Zero Length Packet? */
        if (ep->rem_data_len == 0)
        {
            if (ep->xfer_len != 0 && ep->xfer_len % ep->maxpacket == 0)
            {
                USB_OTG_EPReply_Zerolen(pdev, ep);
                ep->xfer_len = 0;
            }
        }
        else
        {
            if (ep->rem_data_len > ep->maxpacket)
            {
                USB_OTG_WritePacket(pdev,
                                    ep->xfer_buff + ep->xfer_count,
                                    0,
                                    ep->maxpacket);
                ep->xfer_count += ep->maxpacket;
                ep->rem_data_len = ep->xfer_len - ep->xfer_count;
                USB_OTG_TRIG(pdev, ep);
            }
            else
            {
                USB_OTG_WritePacket(pdev,
                                    ep->xfer_buff + ep->xfer_count,
                                    0,
                                    ep->rem_data_len);
                ep->xfer_count = ep->xfer_len;
                ep->rem_data_len = 0;
                USB_OTG_TRIG(pdev, ep);
            }
        }

        if (pdev->cfg.dma_enable == 0)
        {
            /* Enable the Tx FIFO Empty Interrupt for this EP */
            if (ep->xfer_len > 0)
            {
            }
        }
    }
    else
    {
        if (ep->xfer_len == 0)
        {

        }
        else
        {
            if (out0_data_len)
            {
                rx_count = out0_data_len;
            }
            else
            {
                rx_count = USB_OTG_READ_REG8(pdev->regs.LENREGS[0]);
                if(rx_count == 0)
                {
                
                }
            }
            USB_OTG_ReadPacket(pdev, ep->xfer_buff + ep->xfer_count, ep->num, ep->xfer_len);
            ep->xfer_count = 0;
            ep->xfer_buff = pdev->dev.setup_packet;


        }

    }
    return status;
}

/**
* @brief  USB_OTG_EPStartXfer : Handle the setup for data xfer for an EP and
*         starts the xfer
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/

volatile uint8_t data_len;

USB_OTG_STS USB_OTG_EPStartXfer(USB_OTG_CORE_HANDLE *pdev, USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;
    uint8_t  rx_count;
    /* IN endpoint */

    if (ep->is_in == 1)
    {
        if ((pdev->cfg.dma_enable == 0) || ((USB_OTG_DEV_DMA_EP_NUM & 0x07) != ep->num))
        {
            ep->rem_data_len = ep->xfer_len - ep->xfer_count;
            if (ep->rem_data_len == ep->maxpacket)
            {
                USB_OTG_WritePacket(pdev,
                                    ep->xfer_buff + ep->xfer_count,
                                    ep->num,
                                    ep->maxpacket);
                ep->xfer_count += ep->maxpacket;
                USB_OTG_TRIG(pdev, ep);
                ep->xfer_count = ep->xfer_len;
                ep->rem_data_len = 0;
                pdev->dev.zero_replay_flag = 1;

            }
            /* Zero Length Packet? */
            else if (ep->rem_data_len == 0)
            {
                USB_OTG_WritePacket(pdev,
                                    ep->xfer_buff + ep->xfer_count,
                                    ep->num,
                                    0);
                ep->xfer_count = ep->xfer_len;
                ep->rem_data_len = 0;
            }
            else
            {
                if (ep->rem_data_len > ep->maxpacket)
                {
                    USB_OTG_WritePacket(pdev,
                                        ep->xfer_buff + ep->xfer_count,
                                        ep->num,
                                        ep->maxpacket);
                    USB_OTG_TRIG(pdev, ep);
                    ep->xfer_count += ep->maxpacket;
                    if (ep->xfer_len >= ep->xfer_count)
                    {
                        ep->rem_data_len = ep->xfer_len - ep->xfer_count;
                    }
                    else
                    {
                        ep->rem_data_len = 0;
                        ep->xfer_count = ep->xfer_len;
                    }
                }
                else
                {
                    USB_OTG_WritePacket(pdev,
                                        ep->xfer_buff + ep->xfer_count,
                                        ep->num,
                                        ep->rem_data_len);
                    USB_OTG_TRIG(pdev, ep);
                    ep->xfer_count = ep->xfer_len;
                    ep->rem_data_len = 0;
                }
            }
        }
    }
    else
    {
        /* OUT endpoint */
        rx_count = USB_OTG_READ_REG8(pdev->regs.LENREGS[ep ->num]);
        USB_OTG_ReadPacket(pdev, ep->xfer_buff + ep->xfer_count, ep->num, rx_count);
        ep->xfer_count += rx_count;
        data_len = rx_count;
        if (ep->xfer_len <= ep->xfer_count)
        {
            ep->rem_data_len = ep->xfer_count - ep->xfer_len;
        }
        else
        {
            ep->rem_data_len = 0;
            ep->xfer_len = ep->xfer_count;
        }
    }
    return status;
}


/**
* @brief  USB_OTG_TRIG : Handle start xfer and set tx trig
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS  USB_OTG_TRIG(USB_OTG_CORE_HANDLE *pdev, USB_OTG_EP *ep)
{
    int i;
    USB_OTG_STS status = USB_OTG_OK;
    USB_OTG_WRITE_REG8(CORE_USB_STATUS, USB_STATUS_NAK);
    for (i = 0; i < 10000; i++)
    {
        if (USB_OTG_READ_REG8(CORE_USB_STATUS) & USB_STATUS_NAK)  break;
    }
    USB_OTG_WRITE_REG8(CORE_USB_FIFO_EMPTY, 1 << ep ->num);
    USB_OTG_WRITE_REG8(CORE_USB_TRIG, 1 << ep ->num);
    return status;
}
/**
* @brief  USB_OTG_EPSetStall : Set the EP STALL
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EPSetStall(USB_OTG_CORE_HANDLE *pdev, USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;
    if (ep->num == 0)
    {
        USB_OTG_WRITE_REG8(&pdev->regs.xferctl ->STALL, 0x01);
        USB_OTG_EPReply_Zerolen(pdev, ep);
    }
    return status;
}

/**
* @brief  USB_OTG_EPSetStall : ack zero  length packet
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/

USB_OTG_STS USB_OTG_EPReply_Zerolen(USB_OTG_CORE_HANDLE *pdev, USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;
    USB_OTG_WRITE_REG8(&pdev->regs.xferctl ->TRIG, 0x10 << (ep ->num));
    return status;
}



/**
* @brief  USB_OTG_RemoteWakeup : active remote wakeup signalling
* @param  None
* @retval : None
*/
void USB_OTG_ActiveRemoteWakeup(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_CTL_TypeDef power;
    /* Note: If CLK has been stopped,it will need be restarted before
     * this write can occur.
     */
    power.d8 = USB_OTG_READ_REG8(CORE_USB_CONFIG);
    power.b.resume = 1;
    power.b.wakeup_enable = 1;
    USB_OTG_WRITE_REG8(CORE_USB_CONFIG, power.d8);
    /* The software should leave then this bit set for approximately 10ms
     * (minimum of 2ms, a maximum of 15ms) before resetting it to 0.
     */
    delay_ms(2);
    delay_us(500);
    power.b.resume = 0;
    power.b.wakeup_enable = 0;
    USB_OTG_WRITE_REG8(CORE_USB_CONFIG, power.d8);
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
