/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usbd_ccid_if.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : USB-OTG Core layer.
 *****************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usbd_ccid_if.h"
#include "usbd_ccid_core.h"
#include "usbd_ioreq.h"
#include "usbd_usr.h"
#include "yc3121.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t Ccid_BulkState;
uint8_t UsbIntMessageBuffer[INTR_MAX_PACKET_SIZE];  /* data buffer*/
volatile uint8_t PrevXferComplete_IntrIn;
usb_ccid_param_t usb_ccid_param;

uint8_t *pUsbMessageBuffer;
static uint32_t UsbMessageLength;
Ccid_bulkin_data_t pCcid_resp_buff;
Ccid_SlotStatus_t Ccid_SlotStatus;


uint8_t BulkOut_Data_Buff[BULK_MAX_PACKET_SIZE];

Ccid_bulkin_data_t Ccid_bulkin_data;

Ccid_bulkout_data_t Ccid_bulkout_data;

uint8_t UsbIntMessageBuffer[INTR_MAX_PACKET_SIZE];

/* Private function prototypes -----------------------------------------------*/
static void CCID_Response_SendData(USB_OTG_CORE_HANDLE *pdev,
                                   uint8_t *pbuf,
                                   uint16_t len);




/* Private function ----------------------------------------------------------*/
/**
  * @brief  CCID_Init
  *         Initialize the CCID USB Layer
  * @param  pdev: device instance
  * @retval None
  */
void CCID_Init(USB_OTG_CORE_HANDLE *pdev)
{
    /* CCID Related Initialization */
    CCID_SetIntrTransferStatus(1);  /* Transfer Complete Status */
    CCID_UpdSlotChange(1);
    //SC_InitParams();

//    /* Prepare EP to Receive First Cmd */
//    DCD_EP_PrepareRx(pdev,
//                     CCID_BULK_OUT_EP,
//                     (uint8_t *)&BulkOut_Data_Buff[0],
//                     CCID_BULK_EPOUT_SIZE);
}

/**
  * @brief  CCID_DeInit
  *         Uninitialize the CCID Machine
  * @param  pdev: device instance
  * @retval None
  */
void CCID_DeInit(USB_OTG_CORE_HANDLE *pdev)
{
    Ccid_BulkState = CCID_STATE_IDLE;
}

/**
  * @brief  CCID_Message_In
  *         Handle Bulk IN & Intr IN data stage
  * @param  pdev: device instance
  * @param  uint8_t epnum: endpoint index
  * @retval None
  */
void CCID_BulkMessage_In(USB_OTG_CORE_HANDLE *pdev,
                         uint8_t epnum)
{
    if (epnum == (CCID_BULK_IN_EP & 0x7F))
    {
        /* Filter the epnum by masking with 0x7f (mask of IN Direction)  */

        /*************** Handle Bulk Transfer IN data completion  *****************/

    }
    else if (epnum == (CCID_INTR_IN_EP & 0x7F))
    {
        /* Filter the epnum by masking with 0x7f (mask of IN Direction)  */
        CCID_SetIntrTransferStatus(1);  /* Transfer Complete Status */
    }
}


#define CCID_DATA_TIMEOUT   (20)    //CCID_DATA_TIMEOUT*50us
uint32_t ccid_wait_data_time;
uint32_t ccid_wait_data_len;
void CCID_wait_next_packet(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum)
{
    ccid_wait_data_time = 0;
    while (ccid_wait_data_time < CCID_DATA_TIMEOUT)
    {
        ccid_wait_data_len = USB_OTG_READ_REG8(pdev->regs.LENREGS[epnum]);
        if (ccid_wait_data_len >= CCID_BULK_EPOUT_SIZE || \
                ((UsbMessageLength + ccid_wait_data_len) == (Ccid_bulkout_data.dwLength + CCID_CMD_HEADER_SIZE)))
        {
            break;
        }
        ((void(*)(int))(FUNC_DELAY_US_ADDR))(50);
        ccid_wait_data_time++;
    }
}



/**
  * @brief  CCID_BulkMessage_Out
  *         Proccess CCID OUT data
  * @param  pdev: device instance
  * @param  uint8_t epnum: endpoint index
  * @retval None
  */
void CCID_BulkMessage_Out(USB_OTG_CORE_HANDLE *pdev,
                          uint8_t epnum)
{

    uint16_t dataLen;
    dataLen = USBD_GetRxCount(pdev, CCID_BULK_OUT_EP);
    switch (Ccid_BulkState)
    {
    case CCID_STATE_IDLE:
    case CCID_STATE_SEND_RESP:
        if (dataLen == 0x00)
        {
            /* Zero Length Packet Received */
            Ccid_BulkState = CCID_STATE_IDLE;
        }
        else if (dataLen >= CCID_MESSAGE_HEADER_SIZE)
        {
            UsbMessageLength = dataLen;   /* Store for future use */

            /* Expected Data Length Packet Received */
            pUsbMessageBuffer = (uint8_t *)&Ccid_bulkout_data;

            /* Fill CCID_BulkOut Data Buffer from USB Buffer */
            CCID_ReceiveCmdHeader(pUsbMessageBuffer, dataLen);

            /*
            Refer : 6 CCID Messages
            The response messages always contain the exact same slot number,
            and sequence number fields from the header that was contained in
            the Bulk-OUT command message.
            */
            Ccid_bulkin_data.bSlot = Ccid_bulkout_data.bSlot;
            Ccid_bulkin_data.bSeq = Ccid_bulkout_data.bSeq;

            if (dataLen < CCID_BULK_EPOUT_SIZE)
            {
                /* Short message, less than the EP Out Size, execute the command,
                 if parameter like dwLength is too big, the appropriate command will
                 give an error */
                //MyPrintf("> Enter CCID_CmdDecode\r\n");
                CCID_CmdDecode(pdev);
                //MyPrintf("> Exit CCID_CmdDecode\r\n");
            }
            else
            {
                /* Long message, receive additional data with command */
                /* (u8dataLen == CCID_BULK_EPOUT_SIZE) */
                if (Ccid_bulkout_data.dwLength > ABDATA_SIZE)
                {
                    /* Check if length of data to be sent by host is > buffer size */

                    /* Too long data received.... Error ! */
                    Ccid_BulkState = CCID_STATE_UNCORRECT_LENGTH;
                }

                else
                {
                    /* Expect more data on OUT EP */
                    Ccid_BulkState = CCID_STATE_RECEIVE_DATA;
                    pUsbMessageBuffer += dataLen;  /* Point to new offset */

                    CCID_wait_next_packet(pdev, CCID_BULK_OUT_EP);

                    USBD_CCID_DataOut(pdev, epnum);

                } /* if (dataLen == CCID_BULK_EPOUT_SIZE) ends */
            } /*  if (dataLen >= CCID_BULK_EPOUT_SIZE) ends */
        } /* if (dataLen >= CCID_MESSAGE_HEADER_SIZE) ends */
        break;

    case CCID_STATE_RECEIVE_DATA:

        UsbMessageLength += dataLen;

        if (dataLen < CCID_BULK_EPOUT_SIZE)
        {
            /* Short message, less than the EP Out Size, execute the command,
               if parameter like dwLength is too big, the appropriate command will
               give an error */

            /* Full command is received, process the Command */

            CCID_ReceiveCmdHeader(pUsbMessageBuffer, dataLen);
            CCID_CmdDecode(pdev);
            Ccid_BulkState = CCID_STATE_IDLE;
        }
        else if (dataLen == CCID_BULK_EPOUT_SIZE)
        {

            if (UsbMessageLength < (Ccid_bulkout_data.dwLength + CCID_CMD_HEADER_SIZE))
            {
                CCID_ReceiveCmdHeader(pUsbMessageBuffer, dataLen); /* Copy data */
                pUsbMessageBuffer += dataLen;

                CCID_wait_next_packet(pdev, CCID_BULK_OUT_EP);

                USBD_CCID_DataOut(pdev, epnum);
            }
            else if (UsbMessageLength == (Ccid_bulkout_data.dwLength + CCID_CMD_HEADER_SIZE))
            {
                /* Full command is received, process the Command */
                CCID_ReceiveCmdHeader(pUsbMessageBuffer, dataLen);
                CCID_CmdDecode(pdev);
            }
            else
            {
                /* Too long data received.... Error ! */
                Ccid_BulkState = CCID_STATE_UNCORRECT_LENGTH;
            }
        }

        break;

    case CCID_STATE_UNCORRECT_LENGTH:
        Ccid_BulkState = CCID_STATE_IDLE;
        break;

    default:
        break;
    }
}

/**
  * @brief  CCID_CmdDecode
  *         Parse the commands and Proccess command
  * @param  pdev: device instance
  * @retval None
  */
void CCID_CmdDecode(USB_OTG_CORE_HANDLE *pdev)
{
    uint8_t errorCode;
    switch (Ccid_bulkout_data.bMessageType)
    {
    case PC_TO_RDR_ICCPOWERON:
        MyPrintf("On\r\n");
        errorCode = PC_to_RDR_IccPowerOn();
        RDR_to_PC_DataBlock(errorCode);
        break;
    case PC_TO_RDR_ICCPOWEROFF:
        MyPrintf("Off\r\n");
        errorCode = PC_to_RDR_IccPowerOff();
        RDR_to_PC_SlotStatus(errorCode);
        break;

    case PC_TO_RDR_GETSLOTSTATUS:
        MyPrintf("St\r\n");
        errorCode = PC_to_RDR_GetSlotStatus();
        RDR_to_PC_SlotStatus(errorCode);
        break;
    case PC_TO_RDR_XFRBLOCK:
        MyPrintf("Sd\r\n");
        errorCode = PC_to_RDR_XfrBlock();
        //Need to process then send RESP APDU
        //RDR_to_PC_DataBlock(errorCode);
        break;

    case PC_TO_RDR_GETPARAMETERS:
        MyPrintf("GP\r\n");
        errorCode = PC_to_RDR_GetParameters();
        RDR_to_PC_Parameters(errorCode);
        break;
    case PC_TO_RDR_RESETPARAMETERS:
        MyPrintf("RP\r\n");
        errorCode = PC_to_RDR_ResetParameters();
        RDR_to_PC_Parameters(errorCode);
        break;

    case PC_TO_RDR_SETPARAMETERS:
        MyPrintf("SP\r\n");
        errorCode = PC_to_RDR_SetParameters();
        RDR_to_PC_Parameters(errorCode);
        break;
    case PC_TO_RDR_ESCAPE:
        MyPrintf("ES\r\n");
        errorCode = PC_to_RDR_Escape();
        RDR_to_PC_Escape(errorCode);
        break;
    case PC_TO_RDR_ICCCLOCK:
        MyPrintf("IC\r\n");
        errorCode = PC_to_RDR_IccClock();
        RDR_to_PC_SlotStatus(errorCode);
        break;
    case PC_TO_RDR_ABORT:
        MyPrintf("> Enter PC_TO_RDR_ABORT\r\n");
        errorCode = PC_to_RDR_Abort();
        RDR_to_PC_SlotStatus(errorCode);
        break;
    case PC_TO_RDR_T0APDU:
        MyPrintf("T0\r\n");
        errorCode = PC_TO_RDR_T0Apdu();
        RDR_to_PC_SlotStatus(errorCode);
        break;
    case PC_TO_RDR_MECHANICAL:
        MyPrintf("ME\r\n");
        errorCode = PC_TO_RDR_Mechanical();
        RDR_to_PC_SlotStatus(errorCode);
        break;
    case PC_TO_RDR_SETDATARATEANDCLOCKFREQUENCY:
        MyPrintf("FR\r\n");
        errorCode = PC_TO_RDR_SetDataRateAndClockFrequency();
        RDR_to_PC_DataRateAndClockFrequency(errorCode);
        break;
    case PC_TO_RDR_SECURE:
        MyPrintf("SE\r\n");
        errorCode = PC_TO_RDR_Secure();
        RDR_to_PC_DataBlock(errorCode);
        break;

    default:
        MyPrintf("> Enter default\r\n");
        RDR_to_PC_SlotStatus(SLOTERROR_CMD_NOT_SUPPORTED);
        break;
    }

    /********** Decide for all commands ***************/
    if (Ccid_BulkState == CCID_STATE_SEND_RESP)
    {
        CCID_Response_SendData(pdev, (uint8_t *)&Ccid_bulkin_data,
                               Ccid_bulkin_data.u16SizeToSend);
        Ccid_BulkState = CCID_STATE_IDLE;
//         MyPrintf("bulk_state1 = %d\n",Ccid_BulkState);
    }
}

/**
  * @brief  Transfer_Data_Request
  *         Prepare the request response to be sent to the host
  * @param  uint8_t* dataPointer: Pointer to the data buffer to send
  * @param  uint16_t dataLen : number of bytes to send
  * @retval None
  */
void Transfer_Data_Request(uint8_t *dataPointer, uint16_t dataLen)
{
    /**********  Update Global Variables ***************/
    Ccid_bulkin_data.u16SizeToSend = dataLen;
    Ccid_BulkState = CCID_STATE_SEND_RESP;
}


/**
  * @brief  CCID_Response_SendData
  *         Send the data on bulk-in EP
  * @param  pdev: device instance
  * @param  uint8_t* buf: pointer to data buffer
  * @param  uint16_t len: Data Length
  * @retval None
  */
static void CCID_Response_SendData(USB_OTG_CORE_HANDLE *pdev,
                                   uint8_t *buf,
                                   uint16_t len)
{
    DCD_EP_Tx(pdev, CCID_BULK_IN_EP, buf, len);
}

/**
  * @brief  CCID_IntMessage
  *         Send the Interrupt-IN data to the host
  * @param  pdev: device instance
  * @retval None
  */
void CCID_IntMessage(USB_OTG_CORE_HANDLE *pdev)
{
    /* Check if there us change in Smartcard Slot status */
    if (CCID_IsSlotStatusChange() && CCID_IsIntrTransferComplete())
    {
        /* Check Slot Status is changed. Card is Removed/ Fitted  */
        RDR_to_PC_NotifySlotChange();

        CCID_SetIntrTransferStatus(0);  /* Reset the Status */
        CCID_UpdSlotChange(0);    /* Reset the Status of Slot Change */
        my_delay_ms(10);
        DCD_EP_Tx(pdev, CCID_INTR_IN_EP, UsbIntMessageBuffer, 2);
    }
}

/**
  * @brief  CCID_ReceiveCmdHeader
  *         Receive the Data from USB BulkOut Buffer to Pointer
  * @param  uint8_t* pDst: destination address to copy the buffer
  * @param  uint8_t u8length: length of data to copy
  * @retval None
  */
void CCID_ReceiveCmdHeader(uint8_t *pDst, uint8_t u8length)
{
    uint32_t Counter;

    for (Counter = 0; Counter < u8length; Counter++)
    {
        *pDst++ = BulkOut_Data_Buff[Counter];
    }

}

/**
  * @brief  CCID_IsIntrTransferComplete
  *         Provides the status of previous Interrupt transfer status
  * @param  None
  * @retval uint8_t PrevXferComplete_IntrIn: Value of the previous transfer status
  */
uint8_t CCID_IsIntrTransferComplete(void)
{
    return PrevXferComplete_IntrIn;
}

/**
  * @brief  CCID_IsIntrTransferComplete
  *         Set the value of the Interrupt transfer status
  * @param  uint8_t xfer_Status: Value of the Interrupt transfer status to set
  * @retval None
  */
void CCID_SetIntrTransferStatus(uint8_t xfer_Status)
{
    PrevXferComplete_IntrIn = xfer_Status;
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
