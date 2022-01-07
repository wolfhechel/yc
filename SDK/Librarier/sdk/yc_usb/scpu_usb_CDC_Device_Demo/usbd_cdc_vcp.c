/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usbd_cdc_vcp.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : Peripheral Device Interface low layer.
 *****************************************************************************/


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#pragma     data_alignment = 4
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_vcp.h"
#include "usb_conf.h"
#include "usbd_cdc_core.h"
#include "Misc.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LINE_CODING linecoding =
{
    115200, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
};


UART_InitTypeDef USART_InitStructure;

/* These are external variables imported from CDC core to be used for IN
   transfer management. */

#ifdef USER_SPECIFIED_DATA_SOURCE
    extern uint8_t  *APP_Rx_Buffer;
    uint8_t COM_Rx_data_buf[2048];
#else
    //extern uint8_t  APP_Rx_Buffer [];
    extern struct APP_DATA_STRUCT_DEF APP_Gdata_param;
#endif                            /* Write CDC received data in this buffer.
These data will be sent over USB IN endpoint
in the CDC core functions. */
//extern volatile uint32_t APP_Rx_ptr_in;
/* Increment this pointer or roll it back to
   start address when writing received data
   in the buffer APP_Rx_Buffer. */

/* Private function prototypes -----------------------------------------------*/
static uint16_t VCP_Init(void);
static uint16_t VCP_DeInit(void);
static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t *Buf, uint32_t Len);
//static uint16_t VCP_DataTx   (uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataRx(uint8_t *Buf, uint32_t Len);

static uint16_t VCP_COMConfig(uint8_t Conf);

CDC_IF_Prop_TypeDef VCP_fops =
{
    VCP_Init,
    VCP_DeInit,
    VCP_Ctrl,
    VCP_DataTx,
    VCP_DataRx
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  VCP_Init
  *         Initializes the Media on the STM32
  * @param  None
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_Init(void)
{
//    NVIC_InitTypeDef NVIC_InitStructure;

    /**/
#ifdef USER_SPECIFIED_DATA_SOURCE
    APP_Rx_Buffer = COM_Rx_data_buf;
#else
    APP_Gdata_param.COM_config_cmp = 0;
#endif
    memset(&APP_Gdata_param, 0, sizeof(APP_Gdata_param));
    return USBD_OK;
}

/**
  * @brief  VCP_DeInit
  *         DeInitializes the Media on the STM32
  * @param  None
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_DeInit(void)
{

    return USBD_OK;
}


/**
  * @brief  VCP_Ctrl
  *         Manage the CDC class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t *Buf, uint32_t Len)
{
    switch (Cmd)
    {
    case SEND_ENCAPSULATED_COMMAND:
        /* Not  needed for this driver */
        break;

    case GET_ENCAPSULATED_RESPONSE:
        /* Not  needed for this driver */
        break;

    case SET_COMM_FEATURE:
        /* Not  needed for this driver */
        break;

    case GET_COMM_FEATURE:
        /* Not  needed for this driver */
        break;

    case CLEAR_COMM_FEATURE:
        /* Not  needed for this driver */
        break;

    case SET_LINE_CODING:
        linecoding.bitrate = (uint32_t)(Buf[0] | (Buf[1] << 8) | (Buf[2] << 16) | (Buf[3] << 24));
        linecoding.format = Buf[4];
        linecoding.paritytype = Buf[5];
        linecoding.datatype = Buf[6];
        APP_Gdata_param.COM_config_cmp = 1;
        /* Set the new configuration */
        VCP_COMConfig(OTHER_CONFIG);
        break;

    case GET_LINE_CODING:
        Buf[0] = (uint8_t)(linecoding.bitrate);
        Buf[1] = (uint8_t)(linecoding.bitrate >> 8);
        Buf[2] = (uint8_t)(linecoding.bitrate >> 16);
        Buf[3] = (uint8_t)(linecoding.bitrate >> 24);
        Buf[4] = linecoding.format;
        Buf[5] = linecoding.paritytype;
        Buf[6] = linecoding.datatype;
        break;

    case SET_CONTROL_LINE_STATE:
        /* Not  needed for this driver */
        break;

    case SEND_BREAK:
        /* Not  needed for this driver */
        break;

    default:
        break;
    }

    return USBD_OK;
}

int isUsbBufFull()
{
    int ret = 0;
    ret = APP_Gdata_param.rx_structure.Rx_counter >= CDC_APP_RX_DATA_SIZE;
    return ret;
}
/**
  * @brief  VCP_DataTx
  *         CDC received data to be send over USB IN endpoint are managed in
  *         this function.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
uint16_t VCP_DataTx(uint8_t *Buf, uint32_t Len)
{
#ifdef USER_SPECIFIED_DATA_SOURCE
    uint32_t i = 0;
    for (i = 0; i < Len; i++)
    {
        *(APP_Rx_Buffer + APP_Rx_ptr_in) = *Buf++;
        APP_Rx_ptr_in++;
    }
#else
    uint32_t i = 0;
    if (linecoding.datatype == 7)
    {
        for (i = 0; i < Len; i++)
        {
            APP_Gdata_param.rx_structure.APP_Rx_Buffer[APP_Gdata_param.rx_structure.APP_Rx_ptr_in++] = (*Buf++) & 0x7F;
            APP_Gdata_param.rx_structure.Rx_counter++;
            /* To avoid buffer overflow */
            if (APP_Gdata_param.rx_structure.APP_Rx_ptr_in == CDC_APP_RX_DATA_SIZE)
            {
                APP_Gdata_param.rx_structure.APP_Rx_ptr_in = 0;
            }
        }
    }
    else if (linecoding.datatype == 8)
    {
        for (i = 0; i < Len; i++)
        {
            /* Detect whether the buffer overflow */
            APP_Gdata_param.rx_structure.APP_Rx_Buffer[APP_Gdata_param.rx_structure.APP_Rx_ptr_in++] = *Buf++;
            APP_Gdata_param.rx_structure.Rx_counter++;
            /* To avoid buffer overflow */
            if (APP_Gdata_param.rx_structure.APP_Rx_ptr_in == CDC_APP_RX_DATA_SIZE)
            {
                APP_Gdata_param.rx_structure.APP_Rx_ptr_in = 0;
            }
        }
    }
#endif
    return USBD_OK;
}

/**
  * @brief  Get_TxBuf_length
  *         Get the length of the remaining data to be transmitted
  * @param  NONE
  * @retval Result receive data length
  */
uint32_t VCP_GetTxBuflen(void)
{
    uint32_t ret = 0x00;
    if (APP_Gdata_param.rx_structure.APP_Rx_ptr_out < APP_Gdata_param.rx_structure.APP_Rx_ptr_in)
        ret = APP_Gdata_param.rx_structure.APP_Rx_ptr_in - APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
    else
        ret = CDC_APP_RX_DATA_SIZE + APP_Gdata_param.rx_structure.APP_Rx_ptr_in \
              - APP_Gdata_param.rx_structure.APP_Rx_ptr_out;
    return ret;
}

/**
  * @brief  Get_RxBuf_rsaddr
  *         Get reading receive data starting position.
  * @param  NONE
  * @retval Result received data is read starting position
  */
uint32_t VCP_GetTxBufrsaddr(void)
{
    return (APP_Gdata_param.rx_structure.APP_Rx_ptr_out);
}

/**
  * @brief  Get_RxData
  *         Get receive data by byte
  * @param  NONE
  * @retval Result receive data
  */
extern volatile uint8_t usbFlowCtrl;
extern volatile unsigned char APP_Tx_ptr_out_count;
uint8_t VCP_GetRxChar(void)
{
    uint8_t ret;
    ret = (APP_Gdata_param.tx_structure.APP_Tx_Buffer[APP_Gdata_param.tx_structure.APP_Tx_ptr_out++]);
    if (APP_Gdata_param.tx_structure.APP_Tx_ptr_out >= CDC_APP_RX_DATA_SIZE)
    {
        APP_Gdata_param.tx_structure.APP_Tx_ptr_out = 0;
        APP_Tx_ptr_out_count++;

    }
    if ((VCP_GetRxBuflen() + CDC_DATA_MAX_PACKET_SIZE) < CDC_APP_RX_DATA_SIZE)
    {
        if (usbFlowCtrl == 1)
        {
            usbFlowCtrl = 0;
            NVIC_EnableIRQ(USB_IRQn);
        }
    }
	return ret;
}

/**
  * @brief  Get_RxBuf_length
  *         Get receive data length
  * @param  NONE
  * @retval Result receive data length
  */
extern volatile unsigned char APP_Tx_ptr_in_count;
uint32_t VCP_GetRxBuflen(void)
{
    if (APP_Tx_ptr_in_count >= APP_Tx_ptr_out_count)
    {
        return ((APP_Tx_ptr_in_count - APP_Tx_ptr_out_count) * APP_TX_DATA_SIZE + APP_Gdata_param.tx_structure.APP_Tx_ptr_in - APP_Gdata_param.tx_structure.APP_Tx_ptr_out);
    }
    else
    {
        return ((CDC_CMD_DATA_SIZE - APP_Tx_ptr_out_count - APP_Tx_ptr_in_count) * APP_TX_DATA_SIZE + APP_Gdata_param.tx_structure.APP_Tx_ptr_in - APP_Gdata_param.tx_structure.APP_Tx_ptr_out);
    }
}

/**
  * @brief  Get_RxBuf_rsaddr
  *         Get reading receive data starting position.
  * @param  NONE
  * @retval Result received data is read starting position
  */
int8_t *VCP_GetRxBufrsaddr(void)
{
    return (int8_t*)(&(APP_Gdata_param.tx_structure.APP_Tx_Buffer[APP_Gdata_param.tx_structure.APP_Tx_ptr_out]));
}

/**
  * @brief  VCP_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will block any OUT packet reception on USB endpoint
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result
  *         in receiving more data while previous ones are still not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
  */
extern volatile unsigned char APP_Tx_ptr_in_count;
extern volatile uint8_t usbFlowCtrl;
static uint16_t VCP_DataRx(uint8_t *Buf, uint32_t Len)
{
    uint32_t i;

    for (i = 0; i < Len; i++)
    {

        APP_Gdata_param.tx_structure.APP_Tx_Buffer[APP_Gdata_param.tx_structure.APP_Tx_ptr_in++] = Buf[i];
        if (APP_Gdata_param.tx_structure.APP_Tx_ptr_in >= CDC_APP_TX_DATA_SIZE)
        {
            APP_Gdata_param.tx_structure.APP_Tx_ptr_in = 0;
            APP_Tx_ptr_in_count++;
        }

        APP_Gdata_param.tx_structure.Tx_counter++;
    }

    if ((VCP_GetRxBuflen() + CDC_DATA_MAX_PACKET_SIZE) >= CDC_APP_TX_DATA_SIZE)
    {
        if (usbFlowCtrl == 0)
        {
            usbFlowCtrl = 1;
            NVIC_DisableIRQ(USB_IRQn);
        }
    }
    return USBD_OK;
}



/**
  * @brief  VCP_COMConfig
  *         Configure the COM Port with default values or values received from host.
  * @param  Conf: can be DEFAULT_CONFIG to set the default configuration or OTHER_CONFIG
  *         to set a configuration received from the host.
  * @retval None.
  */
static uint16_t VCP_COMConfig(uint8_t Conf)
{

    return USBD_OK;
}

/**
  * @brief  EVAL_COM_IRQHandler
  *
  * @param  None.
  * @retval None.
  */
#if 0
void EVAL_COM_IRQHandler(void)
{
    UART_TypeDef *UARTx = UART1;
    uint8_t ra_data_re = 0x00;
    switch (UARTx->OFFSET_8.IIR & 0x0f)
    {
    case UART_IT_ID_TX_EMPTY:
//            /* Send the received data to the PC Host*/
//            VCP_DataTx (0,0);

        break;
    case UART_IT_ID_RX_RECVD:
#ifdef USER_SPECIFIED_DATA_SOURCE
        if (linecoding.datatype == 7)
        {
            ra_data_re = UART_ReceiveData(UART1) & 0x7F;
        }
        else if (linecoding.datatype == 8)
        {
            ra_data_re = UART_ReceiveData(UART1);
        }
        VCP_DataTx(&ra_data_re, 1);
#else
//            UART_ITConfig(UART1, UART_IT_RX_RECVD, DISABLE);

        ra_data_re = UART_ReceiveData(UART1);
        VCP_DataTx(&ra_data_re, 1);

//            UART_ITConfig(UART1, UART_IT_RX_RECVD, ENABLE);
#endif

        /* If overrun condition occurs, clear the ORE flag and recover communication */
//            (void)UART_ReceiveData(UART1);
        break;
    default :
        break;
    }
}
#endif

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
