/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usbd_ccid_cmd.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : USB-OTG Core layer.
 *****************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "usbd_ccid_cmd.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define CCID_UpdateCommandStatus(cmd_status,icc_status)\
 Ccid_bulkin_data.bStatus=(cmd_status|icc_status)
/*
The Above Macro can take any of following Values
#define BM_ICC_PRESENT_ACTIVE 0x00
#define BM_ICC_PRESENT_INACTIVE 0x01
#define BM_ICC_NO_ICC_PRESENT   0x02

#define BM_COMMAND_STATUS_OFFSET 0x06
#define BM_COMMAND_STATUS_NO_ERROR 0x00
#define BM_COMMAND_STATUS_FAILED   (0x01 << BM_COMMAND_STATUS_OFFSET)
#define BM_COMMAND_STATUS_TIME_EXTN  (0x02 << BM_COMMAND_STATUS_OFFSET)
*/

/* Private variables ---------------------------------------------------------*/
Ccid_bulkin_data_t Ccid_resp_buff;

//Add by link 2017.3.20
Ccid_bulkout_data_t Ccid_out_cmd[CCID_RECV_Q_SIZE];
uint32_t gu32RecvHead = 0;
uint32_t gu32RecvTail = 0;



extern Protocol0_DataStructure_t Protocol0_DataStructure;

/* Private function prototypes -----------------------------------------------*/
static uint8_t CCID_CheckCommandParams(uint32_t param_type);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  PC_to_RDR_IccPowerOn
  *         PC_TO_RDR_ICCPOWERON message execution, apply voltage and get ATR
  * @param  None
  * @retval uint8_t status of the command execution
  */
uint8_t PC_to_RDR_IccPowerOn(void)
{
    /* Apply the ICC VCC
       Fills the Response buffer with ICC ATR
       This Command is returned with RDR_to_PC_DataBlock();
    */
    const uint8_t au8ATR[] = { 0x3B, 0x60, 0x00, 0x00 };
    uint8_t index;
    uint8_t error;

    Ccid_bulkin_data.dwLength = 0;  /* Reset Number of Bytes in abData */

    error = CCID_CheckCommandParams(CHK_PARAM_SLOT | \
                                    CHK_PARAM_DWLENGTH | \
                                    CHK_PARAM_abRFU2 | \
                                    CHK_PARAM_CARD_PRESENT | \
                                    CHK_PARAM_ABORT);
    if (error != 0)
    {
        return error;
    }


    /* ATR is received, No Error Condition Found */
    Ccid_bulkin_data.dwLength = sizeof(au8ATR);
    CCID_UpdateCommandStatus(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);

    for (index = 0; index < sizeof(au8ATR); index++)
    {
        /* Copy the ATR to the Response Buffer */
        Ccid_bulkin_data.abData[index] = au8ATR[index];
    }

    return SLOT_NO_ERROR;
}

/**
  * @brief  PC_to_RDR_IccPowerOff
  *         Icc VCC is switched Off
  * @param  None
  * @retval uint8_t error: status of the command execution
  */
uint8_t PC_to_RDR_IccPowerOff(void)
{
    /*  The response to this command message is the RDR_to_PC_SlotStatus
    response message. */
    uint8_t error;

    error = CCID_CheckCommandParams(CHK_PARAM_SLOT | \
                                    CHK_PARAM_abRFU3 | \
                                    CHK_PARAM_DWLENGTH);
    if (error != 0)
    {
        return error;
    }
    /* Command is ok, Check for Card Presence */

    return SLOT_NO_ERROR;
}

/**
  * @brief  PC_to_RDR_GetSlotStatus
  *         Provides the Slot status to the host
  * @param  None
  * @retval uint8_t status of the command execution
  */
uint8_t PC_to_RDR_GetSlotStatus(void)
{
    uint8_t error;

    error = CCID_CheckCommandParams(CHK_PARAM_SLOT | \
                                    CHK_PARAM_DWLENGTH | \
                                    CHK_PARAM_CARD_PRESENT | \
                                    CHK_PARAM_abRFU3);
    if (error != 0)
    {
        return error;
    }

    CCID_UpdateCommandStatus(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    return SLOT_NO_ERROR;
}


uint32_t resp_CCID_CMD(USB_OTG_CORE_HANDLE *pdev)
{
    uint8_t error = 0;
    uint16_t expectedLength;
    uint32_t u32Next;

    if (NULL == pdev)
    {
        return 0;
    }
    if (Q_EMPTY(gu32RecvHead, gu32RecvTail, CCID_RECV_Q_SIZE))
    {
        return 0;
    }
    u32Next = Q_NEXT(gu32RecvHead, CCID_RECV_Q_SIZE);

    expectedLength = (Ccid_out_cmd[u32Next].bSpecific_2 << 8) | Ccid_out_cmd[u32Next].bSpecific_1;

    Ccid_bulkin_data.dwLength = (uint16_t)expectedLength;

    error = SC_XferBlock(&Ccid_out_cmd[u32Next].abData[0], Ccid_out_cmd[u32Next].dwLength, expectedLength);

    gu32RecvHead = u32Next;
    if (error != SLOT_NO_ERROR)
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
    }
    else
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
        error = SLOT_NO_ERROR;
    }
    RDR_to_PC_DataBlock(error);
    DCD_EP_Tx(pdev, CCID_BULK_IN_EP, (uint8_t *)&Ccid_bulkin_data, Ccid_bulkin_data.u16SizeToSend);
    Ccid_BulkState = CCID_STATE_IDLE;
    return 0;
}

/**
  * @brief  PC_to_RDR_XfrBlock
  *         Handles the Block transfer from Host.
  *         Response to this command message is the RDR_to_PC_DataBlock
  * @param  None
  * @retval uint8_t status of the command execution
  */
uint8_t PC_to_RDR_XfrBlock(void)
{
    //uint16_t expectedLength;

    uint8_t error = 0;
    uint32_t u32Next;

    error = CCID_CheckCommandParams(CHK_PARAM_SLOT | \
                                    CHK_PARAM_CARD_PRESENT | \
                                    CHK_PARAM_abRFU3 | \
                                    CHK_PARAM_ABORT | \
                                    CHK_ACTIVE_STATE);

    if (error != 0) return error;

    if (Ccid_bulkout_data.dwLength > ABDATA_SIZE)
    {
        /* Check amount of Data Sent by Host is > than memory allocated ? */

        return SLOTERROR_BAD_DWLENGTH;
    }


    /* wLevelParameter = Size of expected data to be returned by the
                          bulk-IN endpoint */
//  expectedLength = (Ccid_bulkout_data.bSpecific_2 << 8) |
//      Ccid_bulkout_data.bSpecific_1;
//
//  Ccid_bulkin_data.dwLength = (uint16_t)expectedLength;


    /*
    error = SC_XferBlock(&Ccid_bulkout_data.abData[0],
                         Ccid_bulkout_data.dwLength,
                         expectedLength);
    */
    if (!Q_IS_FULL(gu32RecvHead, gu32RecvTail, CCID_RECV_Q_SIZE))
    {
        u32Next = Q_NEXT(gu32RecvTail, CCID_RECV_Q_SIZE);

        memcpy((char *)(Ccid_out_cmd + u32Next), (char *)(&Ccid_bulkout_data), (int)sizeof(Ccid_bulkout_data_t));
        gu32RecvTail = u32Next;
    }
    else
    {
        //Queue overflow
        error = SLOTERROR_BAD_DWLENGTH;
    }

    if (error != SLOT_NO_ERROR)
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
    }
    else
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
        error = SLOT_NO_ERROR;
    }

    return error;
}


/**
  * @brief  PC_to_RDR_GetParameters
  *         Provides the ICC parameters to the host
  *         Response to this command message is the RDR_to_PC_Parameters
  * @param  None
  * @retval uint8_t status of the command execution
  */
uint8_t PC_to_RDR_GetParameters(void)
{
    uint8_t error;

    error = CCID_CheckCommandParams(CHK_PARAM_SLOT | \
                                    CHK_PARAM_DWLENGTH | \
                                    CHK_PARAM_CARD_PRESENT | \
                                    CHK_PARAM_abRFU3);
    if (error != 0) return error;

    CCID_UpdateCommandStatus(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);

    return SLOT_NO_ERROR;
}


/**
  * @brief  PC_to_RDR_ResetParameters
  *         Set the ICC parameters to the default
  *         Response to this command message is the RDR_to_PC_Parameters
  * @param  None
  * @retval uint8_t status of the command execution
  */
uint8_t PC_to_RDR_ResetParameters(void)
{
    uint8_t error;

    error = CCID_CheckCommandParams(CHK_PARAM_SLOT | \
                                    CHK_PARAM_DWLENGTH | \
                                    CHK_PARAM_CARD_PRESENT | \
                                    CHK_PARAM_abRFU3 | \
                                    CHK_ACTIVE_STATE);
    if (error != 0) return error;

    /* This command resets the slot parameters to their default values */
    Ccid_bulkout_data.abData[0] = DEFAULT_FIDI;
    Ccid_bulkout_data.abData[1] = DEFAULT_T01CONVCHECKSUM;
    Ccid_bulkout_data.abData[2] = DEFAULT_EXTRA_GUARDTIME;
    Ccid_bulkout_data.abData[3] = DEFAULT_WAITINGINTEGER;
    Ccid_bulkout_data.abData[4] = DEFAULT_CLOCKSTOP;
    Ccid_bulkout_data.abData[5] = 0x00;
    Ccid_bulkout_data.abData[6] = 0x00;

    error = SC_SetParams((Protocol0_DataStructure_t *)(&(Ccid_bulkout_data.abData[0])));

    if (error != SLOT_NO_ERROR)
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
    }
    else
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
        error = SLOT_NO_ERROR;
    }

    return error;
}


/**
  * @brief  PC_to_RDR_SetParameters
  *         Set the ICC parameters to the host defined parameters
  *         Response to this command message is the RDR_to_PC_Parameters
  * @param  None
  * @retval uint8_t status of the command execution
  */
uint8_t PC_to_RDR_SetParameters(void)
{
    uint8_t error;

    error = CCID_CheckCommandParams(CHK_PARAM_SLOT | \
                                    CHK_PARAM_CARD_PRESENT | \
                                    CHK_PARAM_abRFU2 | \
                                    CHK_ACTIVE_STATE);
    if (error != 0) return error;

    error = SLOT_NO_ERROR;

    /* for Protocol T=0 (bProtocolNum=0) (dwLength=00000005h) */
    if ((Ccid_bulkout_data.dwLength == 5) &&
            (Ccid_bulkout_data.bSpecific_0 != 0)) error = SLOTERROR_BAD_PROTOCOLNUM;

    /* for Protocol T=1 (bProtocolNum=1) (dwLength=00000007h) */
    if ((Ccid_bulkout_data.dwLength == 7) &&
            (Ccid_bulkout_data.bSpecific_0 != 1)) error = SLOTERROR_CMD_NOT_SUPPORTED;

    /* For T0, Waiting Integer 0 supported */
    if (Ccid_bulkout_data.abData[3] != 0) error = SLOTERROR_BAD_WAITINGINTEGER;

    if (Ccid_bulkout_data.abData[4] != DEFAULT_CLOCKSTOP) error = SLOTERROR_BAD_CLOCKSTOP;

    if (error != SLOT_NO_ERROR)
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
    }

    error = SC_SetParams((Protocol0_DataStructure_t *)(&(Ccid_bulkout_data.abData[0])));

    if (error != SLOT_NO_ERROR)
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
    }
    else
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
        error = SLOT_NO_ERROR;
    }

    return error;
}


/**
  * @brief  PC_to_RDR_Escape
  *         Execute the Escape command. This is user specific Implementation
  *         Response to this command message is the RDR_to_PC_Escape
  * @param  None
  * @retval uint8_t status of the command execution
  */
uint8_t PC_to_RDR_Escape(void)
{
    uint8_t error;
    uint16_t size;

    error = CCID_CheckCommandParams(CHK_PARAM_SLOT | \
                                    CHK_PARAM_CARD_PRESENT | \
                                    CHK_PARAM_abRFU3 | \
                                    CHK_PARAM_ABORT | \
                                    CHK_ACTIVE_STATE);

    if (error != 0) return error;

    /*
    error = SC_ExecuteEscape(&Ccid_bulkout_data.abData[0],
                             Ccid_bulkout_data.dwLength,
                             &Ccid_bulkin_data.abData[0],
                             &size);
    */

    Ccid_bulkin_data.dwLength = size;

    if (error != SLOT_NO_ERROR)
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
    }
    else
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    }

    return error;
}


/**
  * @brief  PC_to_RDR_IccClock
  *         Execute the Clock specific command from host
  *         Response to this command message is the RDR_to_PC_SlotStatus
  * @param  None
  * @retval uint8_t status of the command execution
  */
uint8_t PC_to_RDR_IccClock(void)
{
    uint8_t error;

    error = CCID_CheckCommandParams(CHK_PARAM_SLOT | \
                                    CHK_PARAM_CARD_PRESENT | \
                                    CHK_PARAM_abRFU2 | \
                                    CHK_PARAM_DWLENGTH | \
                                    CHK_ACTIVE_STATE);
    if (error != 0) return error;

    /* bClockCommand � 00h restarts Clock
                     � 01h Stops Clock in the state shown in the bClockStop
                         field of the PC_to_RDR_SetParameters command
                         and RDR_to_PC_Parameters message.*/
    if (Ccid_bulkout_data.bSpecific_0 > 1)
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
        return SLOTERROR_BAD_CLOCKCOMMAND;
    }

    error = SC_SetClock(Ccid_bulkout_data.bSpecific_0);

    if (error != SLOT_NO_ERROR)
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
    }
    else
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    }

    return error;
}


/**
  * @brief  PC_to_RDR_Abort
  *         Execute the Abort command from host, This stops all Bulk transfers
  *         from host and ICC
  *         Response to this command message is the RDR_to_PC_SlotStatus
  * @param  None
  * @retval uint8_t status of the command execution
  */
uint8_t PC_to_RDR_Abort(void)
{
    uint8_t error;

    error = CCID_CheckCommandParams(CHK_PARAM_SLOT | \
                                    CHK_PARAM_abRFU3 | \
                                    CHK_PARAM_DWLENGTH);
    if (error != 0) return error;

    //CCID_CmdAbort(Ccid_bulkout_data.bSlot, Ccid_bulkout_data.bSeq);
    CCID_UpdateCommandStatus(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    return SLOT_NO_ERROR;
}

/**
  * @brief  CCID_CmdAbort
  *         Execute the Abort command from Bulk EP or from Control EP,
  *          This stops all Bulk transfers from host and ICC
  * @param  uint8_t slot: slot number that host wants to abort
  * @param  uint8_t seq : Seq number for PC_to_RDR_Abort
  * @retval uint8_t status of the command execution
  */
uint8_t CCID_CmdAbort(uint8_t slot, uint8_t seq)
{
    /* This function is called for REQUEST_ABORT & PC_to_RDR_Abort */

    if (slot >= CCID_NUMBER_OF_SLOTS)
    {
        /* This error condition is possible only from CLASS_REQUEST, otherwise
        Slot is already checked in parameters from PC_to_RDR_Abort request */
        /* Slot requested is more than supported by Firmware */
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_NO_ICC_PRESENT);
        return SLOTERROR_BAD_SLOT;
    }

    if (usb_ccid_param.bAbortRequestFlag == 1)
    {
        /* Abort Command was already received from ClassReq or PC_to_RDR */
        if ((usb_ccid_param.bSeq == seq) && (usb_ccid_param.bSlot == slot))
        {
            /* CLASS Specific request is already Received, Reset the abort flag */
            usb_ccid_param.bAbortRequestFlag = 0;
        }
    }
    else
    {
        /* Abort Command was NOT received from ClassReq or PC_to_RDR,
           so save them for next ABORT command to verify */
        usb_ccid_param.bAbortRequestFlag = 1;
        usb_ccid_param.bSeq = seq;
        usb_ccid_param.bSlot = slot;
    }

    return 0;
}

/**
  * @brief  PC_TO_RDR_T0Apdu
  *         Execute the PC_TO_RDR_T0APDU command from host
  *         Response to this command message is the RDR_to_PC_SlotStatus
  * @param  None
  * @retval uint8_t status of the command execution
  */
uint8_t PC_TO_RDR_T0Apdu(void)
{
    uint8_t error;

    error = CCID_CheckCommandParams(CHK_PARAM_SLOT | \
                                    CHK_PARAM_CARD_PRESENT | \
                                    CHK_PARAM_DWLENGTH |
                                    CHK_PARAM_ABORT);
    if (error != 0) return error;

    if (Ccid_bulkout_data.bSpecific_0 > 0x03)
    {
        /* Bit 0 is associated with field bClassGetResponse
           Bit 1 is associated with field bClassEnvelope
           Other bits are RFU.*/

        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
        return SLOTERROR_BAD_BMCHANGES;
    }

    /*
    error = SC_T0Apdu(Ccid_bulkout_data.bSpecific_0,
                      Ccid_bulkout_data.bSpecific_1,
                      Ccid_bulkout_data.bSpecific_2);
    */
    if (error != SLOT_NO_ERROR)
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
    }
    else
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    }

    return error;
}

/**
  * @brief  PC_TO_RDR_Mechanical
  *         Execute the PC_TO_RDR_MECHANICAL command from host
  *         Response to this command message is the RDR_to_PC_SlotStatus
  * @param  None
  * @retval uint8_t status of the command execution
  */
uint8_t PC_TO_RDR_Mechanical(void)
{
    uint8_t error;

    error = CCID_CheckCommandParams(CHK_PARAM_SLOT | \
                                    CHK_PARAM_CARD_PRESENT | \
                                    CHK_PARAM_abRFU2 | \
                                    CHK_PARAM_DWLENGTH
                                   );
    if (error != 0) return error;

    if (Ccid_bulkout_data.bSpecific_0 > 0x05)
    {
        /* 01h � Accept Card
           02h � Eject Card
           03h � Capture Card
           04h � Lock Card
           05h � Unlock Card*/

        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
        return SLOTERROR_BAD_BFUNCTION_MECHANICAL;
    }

    //error = SC_Mechanical(Ccid_bulkout_data.bSpecific_0);

    if (error != SLOT_NO_ERROR)
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
    }
    else
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    }

    return error;
}

/**
  * @brief  PC_TO_RDR_SetDataRateAndClockFrequency
  *         Set the required Card Frequency and Data rate from the host.
  *         Response to this command message is the
  *           RDR_to_PC_DataRateAndClockFrequency
  * @param  None
  * @retval uint8_t status of the command execution
  */
uint8_t PC_TO_RDR_SetDataRateAndClockFrequency(void)
{
    uint8_t error;
    uint32_t clockFrequency;
    uint32_t dataRate;
    uint32_t temp = 0;

    error = CCID_CheckCommandParams(CHK_PARAM_SLOT | \
                                    CHK_PARAM_CARD_PRESENT | \
                                    CHK_PARAM_abRFU3);
    if (error != 0) return error;

    if (Ccid_bulkout_data.dwLength != 0x08)
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
        return SLOTERROR_BAD_LENTGH;
    }

    /* HERE we avoiding to an unaligned memory access*/
    temp = (Ccid_bulkout_data.abData[0]) & 0x000000FF;
    clockFrequency = temp;

    temp = (Ccid_bulkout_data.abData[1]) & 0x000000FF;
    clockFrequency |= temp << 8;

    temp = (Ccid_bulkout_data.abData[2]) & 0x000000FF;
    clockFrequency |= temp << 16;

    temp = (Ccid_bulkout_data.abData[3]) & 0x000000FF;
    clockFrequency |= temp << 24;

    temp = (Ccid_bulkout_data.abData[4]) & 0x000000FF;
    dataRate = temp;

    temp = (Ccid_bulkout_data.abData[5]) & 0x000000FF;
    dataRate |= temp << 8;

    temp = (Ccid_bulkout_data.abData[6]) & 0x000000FF;
    dataRate |= temp << 16;

    temp = (Ccid_bulkout_data.abData[7]) & 0x000000FF;
    dataRate |= temp << 24;

    //error = SC_SetDataRateAndClockFrequency(clockFrequency, dataRate);
    Ccid_bulkin_data.bError = error;

    if (error != SLOT_NO_ERROR)
    {
        Ccid_bulkin_data.dwLength = 0;
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
    }
    else
    {
        Ccid_bulkin_data.dwLength = 8;

        (Ccid_bulkin_data.abData[0]) = clockFrequency & 0x000000FF;

        (Ccid_bulkin_data.abData[1]) = (clockFrequency & 0x0000FF00) >> 8;

        (Ccid_bulkin_data.abData[2]) = (clockFrequency & 0x00FF0000) >> 16;

        (Ccid_bulkin_data.abData[3]) = (clockFrequency & 0xFF000000) >> 24;

        (Ccid_bulkin_data.abData[4]) = dataRate & 0x000000FF;

        (Ccid_bulkin_data.abData[5]) = (dataRate & 0x0000FF00) >> 8;

        (Ccid_bulkin_data.abData[6]) = (dataRate & 0x00FF0000) >> 16;

        (Ccid_bulkin_data.abData[7]) = (dataRate & 0xFF000000) >> 24;

        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    }

    return error;
}

/**
  * @brief  PC_TO_RDR_Secure
  *         Execute the Secure Command from the host.
  *         Response to this command message is the RDR_to_PC_DataBlock
  * @param  None
  * @retval uint8_t status of the command execution
  */
uint8_t PC_TO_RDR_Secure(void)
{
    uint8_t error;
    uint16_t wLevelParameter;
    uint32_t responseLen;

    Ccid_bulkin_data.dwLength = 0;

    error = CCID_CheckCommandParams(CHK_PARAM_SLOT | \
                                    CHK_PARAM_CARD_PRESENT | \
                                    CHK_PARAM_ABORT);

    if (error != 0) return error;

    wLevelParameter = (Ccid_bulkout_data.bSpecific_1 + ((uint16_t)Ccid_bulkout_data.bSpecific_2 << 8));

    if ((EXCHANGE_LEVEL_FEATURE == TPDU_EXCHANGE) ||
            (EXCHANGE_LEVEL_FEATURE == SHORT_APDU_EXCHANGE))
    {
        /* TPDU level & short APDU level, wLevelParameter is RFU, = 0000h */
        if (wLevelParameter != 0)
        {
            CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
            error = SLOTERROR_BAD_LEVELPARAMETER;
            return error;
        }
    }

    /*
    error = SC_Secure(Ccid_bulkout_data.dwLength, bBWI, wLevelParameter,
                      &Ccid_bulkout_data.abData[0], &responseLen);
    */
    Ccid_bulkin_data.dwLength = responseLen;

    if (error != SLOT_NO_ERROR)
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
    }
    else
    {
        CCID_UpdateCommandStatus(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    }

    return error;
}

/******************************************************************************/
/*    BULK IN ROUTINES                              */
/******************************************************************************/

/**
  * @brief  RDR_to_PC_DataBlock
  *         Provide the data block response to the host
  *         Response for PC_to_RDR_IccPowerOn, PC_to_RDR_XfrBlock
  * @param  uint8_t errorCode: code to be returned to the host
  * @retval None
  */
void RDR_to_PC_DataBlock(uint8_t errorCode)
{
    uint16_t length = CCID_RESPONSE_HEADER_SIZE;

    Ccid_bulkin_data.bMessageType = RDR_TO_PC_DATABLOCK;
    Ccid_bulkin_data.bError = errorCode;
    Ccid_bulkin_data.bSpecific = 0;    /* bChainParameter */

    if (errorCode == SLOT_NO_ERROR)
    {
        length += Ccid_bulkin_data.dwLength;   /* Length Specified in Command */
    }

    Transfer_Data_Request((uint8_t *)(&Ccid_bulkin_data), length);

}


/**
  * @brief  RDR_to_PC_SlotStatus
  *         Provide the Slot status response to the host
  *          Response for PC_to_RDR_IccPowerOff
  *                PC_to_RDR_GetSlotStatus
  *                PC_to_RDR_IccClock
  *                PC_to_RDR_T0APDU
  *                PC_to_RDR_Mechanical
  *         Also the device sends this response message when it has completed
  *         aborting a slot after receiving both the Class Specific ABORT request
  *          and PC_to_RDR_Abort command message.
  * @param  uint8_t errorCode: code to be returned to the host
  * @retval None
  */
void RDR_to_PC_SlotStatus(uint8_t errorCode)
{

    Ccid_bulkin_data.bMessageType = RDR_TO_PC_SLOTSTATUS;
    Ccid_bulkin_data.dwLength = 0;
    Ccid_bulkin_data.bError = errorCode;
    Ccid_bulkin_data.bSpecific = 0;    /* bClockStatus = 00h Clock running
                                          01h Clock stopped in state L
                                          02h Clock stopped in state H
                                          03h Clock stopped in an unknown state
                                          All other values are RFU. */


    Transfer_Data_Request((uint8_t *)(&Ccid_bulkin_data),
                          LEN_RDR_TO_PC_SLOTSTATUS);

}

/**
  * @brief  RDR_to_PC_Parameters
  *         Provide the data block response to the host
  *         Response for PC_to_RDR_GetParameters, PC_to_RDR_ResetParameters
  *                      PC_to_RDR_SetParameters
  * @param  uint8_t errorCode: code to be returned to the host
  * @retval None
  */
void RDR_to_PC_Parameters(uint8_t errorCode)
{
    uint16_t length = CCID_RESPONSE_HEADER_SIZE;

    Ccid_bulkin_data.bMessageType = RDR_TO_PC_PARAMETERS;
    Ccid_bulkin_data.bError = errorCode;

    if (errorCode == SLOT_NO_ERROR)
    {
        Ccid_bulkin_data.dwLength = LEN_PROTOCOL_STRUCT_T0;
        length += LEN_PROTOCOL_STRUCT_T0;
    }
    else
    {
        Ccid_bulkin_data.dwLength = 0;
    }

    Ccid_bulkin_data.abData[0] = Protocol0_DataStructure.bmFindexDindex;
    Ccid_bulkin_data.abData[1] = Protocol0_DataStructure.bmTCCKST0;
    Ccid_bulkin_data.abData[2] = Protocol0_DataStructure.bGuardTimeT0;
    Ccid_bulkin_data.abData[3] = Protocol0_DataStructure.bWaitingIntegerT0;
    Ccid_bulkin_data.abData[4] = Protocol0_DataStructure.bClockStop;

    /* bProtocolNum */
    Ccid_bulkin_data.bSpecific = BPROTOCOL_NUM_T0;

    Transfer_Data_Request((uint8_t *)(&Ccid_bulkin_data), length);
}

/**
  * @brief  RDR_to_PC_Escape
  *         Provide the Escaped data block response to the host
  *         Response for PC_to_RDR_Escape
  * @param  uint8_t errorCode: code to be returned to the host
  * @retval None
  */
void RDR_to_PC_Escape(uint8_t errorCode)
{
    uint16_t length = CCID_RESPONSE_HEADER_SIZE;

    Ccid_bulkin_data.bMessageType = RDR_TO_PC_ESCAPE;

    Ccid_bulkin_data.bSpecific = 0;    /* Reserved for Future Use */
    Ccid_bulkin_data.bError = errorCode;

    if (errorCode == SLOT_NO_ERROR)
    {
        length += Ccid_bulkin_data.dwLength;   /* Length Specified in Command */
    }

    Transfer_Data_Request((uint8_t *)(&Ccid_bulkin_data), length);
}



/**
  * @brief  RDR_to_PC_DataRateAndClockFrequency
  *         Provide the Clock and Data Rate information to host
  *         Response for PC_TO_RDR_SetDataRateAndClockFrequency
  * @param  uint8_t errorCode: code to be returned to the host
  * @retval None
  */
void RDR_to_PC_DataRateAndClockFrequency(uint8_t errorCode)
{
    uint16_t length = CCID_RESPONSE_HEADER_SIZE;

    Ccid_bulkin_data.bMessageType = RDR_TO_PC_DATARATEANDCLOCKFREQUENCY;
    Ccid_bulkin_data.bError = errorCode;
    Ccid_bulkin_data.bSpecific = 0;    /* Reserved for Future Use */

    if (errorCode == SLOT_NO_ERROR)
    {
        length += Ccid_bulkin_data.dwLength;   /* Length Specified in Command */
    }

    Transfer_Data_Request((uint8_t *)(&Ccid_bulkin_data), length);
}

/**
  * @brief  RDR_to_PC_NotifySlotChange
  *         Interrupt message to be sent to the host, Checks the card presence
  *           status and update the buffer accordingly
  * @param  None
  * @retval None
  */
void RDR_to_PC_NotifySlotChange(void)
{
    UsbIntMessageBuffer[OFFSET_INT_BMESSAGETYPE] = RDR_TO_PC_NOTIFYSLOTCHANGE;


    /*
    SLOT_ICC_PRESENT 0x01 : LSb : (0b = no ICC present, 1b = ICC present)
    SLOT_ICC_CHANGE 0x02 : MSb : (0b = no change, 1b = change).
    */
    UsbIntMessageBuffer[OFFSET_INT_BMSLOTICCSTATE] = SLOT_ICC_PRESENT |
            SLOT_ICC_CHANGE;
}


/**
  * @brief  CCID_UpdSlotStatus
  *         Updates the variable for the slot status
  * @param  uint8_t slotStatus : slot status from the calling function
  * @retval None
  */
void CCID_UpdSlotStatus(uint8_t slotStatus)
{
    Ccid_SlotStatus.SlotStatus = slotStatus;
}

/**
  * @brief  CCID_UpdSlotChange
  *         Updates the variable for the slot change status
  * @param  uint8_t changeStatus : slot change status from the calling function
  * @retval None
  */
void CCID_UpdSlotChange(uint8_t changeStatus)
{
    Ccid_SlotStatus.SlotStatusChange = changeStatus;
}

/**
  * @brief  CCID_IsSlotStatusChange
  *         Provides the value of the variable for the slot change status
  * @param  None
  * @retval uint8_t slot change status
  */
uint8_t CCID_IsSlotStatusChange(void)
{
    return Ccid_SlotStatus.SlotStatusChange;
}

/**
  * @brief  CCID_CheckCommandParams
  *         Checks the specific parameters requested by the function and update
  *          status accordingly. This function is called from all
  *          PC_to_RDR functions
  * @param  uint32_t param_type : Parameter enum to be checked by calling function
  * @retval uint8_t status
  */
static uint8_t CCID_CheckCommandParams(uint32_t param_type)
{
    uint32_t parameter;

    Ccid_bulkin_data.bStatus = BM_ICC_PRESENT_ACTIVE | BM_COMMAND_STATUS_NO_ERROR;

    parameter = (uint32_t)param_type;

    if (parameter & CHK_PARAM_SLOT)
    {
        /*
        The slot number (bSlot) identifies which ICC slot is being addressed
        by the message, if the CCID supports multiple slots.
        The slot number is zero-relative, and is in the range of zero to FFh.
        */

        /* SLOT Number is 0 onwards, so always < CCID_NUMBER_OF_SLOTs */
        /* Error Condition !!! */
        if (Ccid_bulkout_data.bSlot >= CCID_NUMBER_OF_SLOTS)
        {
            /* Slot requested is more than supported by Firmware */
            CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_NO_ICC_PRESENT);
            return SLOTERROR_BAD_SLOT;
        }
    }

    if (parameter & CHK_PARAM_CARD_PRESENT)
    {

    }

    /* Check that DwLength is 0 */
    if (parameter & CHK_PARAM_DWLENGTH)
    {
        if (Ccid_bulkout_data.dwLength != 0)
        {
            CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
            return SLOTERROR_BAD_LENTGH;
        }
    }

    /* abRFU 2 : Reserved for Future Use*/
    if (parameter & CHK_PARAM_abRFU2)
    {

        if ((Ccid_bulkout_data.bSpecific_1 != 0) ||
                (Ccid_bulkout_data.bSpecific_2 != 0))
        {
            CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
            return SLOTERROR_BAD_ABRFU_2B;        /* bSpecific_1 */
        }
    }

    if (parameter & CHK_PARAM_abRFU3)
    {
        /* abRFU 3 : Reserved for Future Use*/
        if ((Ccid_bulkout_data.bSpecific_0 != 0) ||
                (Ccid_bulkout_data.bSpecific_1 != 0) ||
                (Ccid_bulkout_data.bSpecific_2 != 0))
        {
            CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
            return SLOTERROR_BAD_ABRFU_3B;
        }
    }


    if (parameter & CHK_PARAM_ABORT)
    {
        if (usb_ccid_param.bAbortRequestFlag)
        {
            CCID_UpdateCommandStatus(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_INACTIVE);
            return SLOTERROR_CMD_ABORTED;
        }
    }

    if (parameter & CHK_ACTIVE_STATE)
    {
        /* Commands Parameters ok, Check the Card Status */
        /* Card is detected */
        //Card always active T0/T1 doesn't matter.

    }

    return 0;
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/

