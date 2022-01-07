/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usb_core.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : USB-OTG Core layer.
 *****************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "sc_itf.h"
#include "usb_bsp.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

SC_Param_t  SC_Param;
Protocol0_DataStructure_t Protocol0_DataStructure;

SC_ADPU_Commands SC_ADPU;
SC_ADPU_Response SC_Response;

/* F Table */
//static uint32_t F_Table[16] = {372, 372, 558, 744, 1116, 1488, 1860, 0, 0, 512, 768,
//                               1024, 1536, 2048, 0, 0
//                              };
/* D Table */
static uint32_t D_Table[16] = {0, 1, 2, 4, 8, 16, 32, 64, 12, 20, 0, 0, 0, 0, 0,
                               0
                              };

uint32_t SC_GetDTableValue(uint8_t idx)
{
    return D_Table[idx];
}

void SC_InitParams(void)
{
    /*
    FI, the reference to a clock rate conversion factor
    over the bits b8 to b5
    - DI, the reference to a baud rate adjustment factor
    over the bits b4 to bl
    */

    SC_Param.SC_A2R_FiDi = DEFAULT_FIDI;
    SC_Param.SC_hostFiDi = DEFAULT_FIDI;

    Protocol0_DataStructure.bmFindexDindex = DEFAULT_FIDI;

    /* Placeholder, Ignored */
    /* 0 = Direct, first byte of the ICC’s ATR data. */
    Protocol0_DataStructure.bmTCCKST0 = DEFAULT_T01CONVCHECKSUM;

    /* Extra GuardTime = 0 etu */
    Protocol0_DataStructure.bGuardTimeT0 = DEFAULT_EXTRA_GUARDTIME;
    Protocol0_DataStructure.bWaitingIntegerT0 = DEFAULT_WAITINGINTEGER;
    Protocol0_DataStructure.bClockStop = 0; /* Stopping the Clock is not allowed */
}

/**
  * @brief  SC_SetClock function to define Clock Status request from the host.
  *         This is user implementable
  * @param  uint8_t bClockCommand: Clock status from the host
  * @retval uint8_t status value
  */
uint8_t SC_SetClock(uint8_t bClockCommand)
{
    /* bClockCommand • 00h restarts Clock
                     • 01h Stops Clock in the state shown in the bClockStop
                       field of the PC_to_RDR_SetParameters command
                       and RDR_to_PC_Parameters message.*/

    if (bClockCommand == 0)
    {
        /* 00h restarts Clock : Since Clock is always running, PASS this command */
        return SLOT_NO_ERROR;
    }
    else if (bClockCommand == 1)
    {
        return SLOTERROR_BAD_CLOCKCOMMAND;
    }

    return SLOTERROR_CMD_NOT_SUPPORTED;
}


/**
  * @brief  SC_SetParams
  *         Set the parameters for CCID/USART interface
  * @param  Protocol0_DataStructure_t* pPtr: pointer to buffer containing the
  *          parameters to be set in USART
  * @retval uint8_t status value
  */
uint8_t SC_SetParams(Protocol0_DataStructure_t *pPtr)
{
    uint16_t guardTime;   /* Keep it 16b for handling 8b additions */
    uint32_t fi_new;
    uint32_t di_new;

    guardTime = pPtr->bGuardTimeT0;
    if (guardTime > MAX_EXTRA_GUARD_TIME)
    {
        return SLOTERROR_BAD_GUARDTIME;
    }

    fi_new = pPtr->bmFindexDindex;
    di_new = pPtr->bmFindexDindex;

    /* Check for the FIDI Value set by Host */
    di_new &= (uint8_t)0x0F;
    if (SC_GetDTableValue(di_new) == 0)
    {
        return SLOTERROR_BAD_FIDI;
    }

    fi_new >>= 4;
    fi_new &= (uint8_t)0x0F;

    if (SC_GetDTableValue(fi_new) == 0)
    {
        return SLOTERROR_BAD_FIDI;
    }

//  /* Put Total GuardTime in USART Settings */


    /* Save Extra GuardTime Value */
    Protocol0_DataStructure.bGuardTimeT0 = guardTime;

    Protocol0_DataStructure.bmTCCKST0 = pPtr->bmTCCKST0;
    Protocol0_DataStructure.bClockStop = pPtr->bClockStop;

    /* Save New bmFindexDindex */
    SC_Param.SC_hostFiDi = pPtr->bmFindexDindex;

    Protocol0_DataStructure.bmFindexDindex = pPtr->bmFindexDindex;

    return SLOT_NO_ERROR;
}

uint8_t SC_XferBlock(uint8_t *ptrBlock, uint32_t blockLen, uint16_t expectedLen)
{

    uint32_t index, i = 0;
    uint32_t cmdType = 1;
    if ((ptrBlock[0] == 0x00) && (ptrBlock[1] == 0x80))
    {
        memcpy(Ccid_bulkin_data.abData, ptrBlock + 5, blockLen - 5);
        Ccid_bulkin_data.dwLength = blockLen - 5;
        //memcpy(Ccid_bulkin_data.abData,COMMDATA,Ccid_bulkin_data.dwLength);
        Ccid_bulkin_data.abData[Ccid_bulkin_data.dwLength++] = 0x90;
        Ccid_bulkin_data.abData[Ccid_bulkin_data.dwLength++] = 0x00;
    }
    else
    {
        SC_ADPU.Header.CLA = ptrBlock[i++];
        SC_ADPU.Header.INS = ptrBlock[i++];
        SC_ADPU.Header.P1 = ptrBlock[i++];
        SC_ADPU.Header.P2 = ptrBlock[i++];

        //CLA + INS + P1 + P2
        if (blockLen == 0x04)
        {
            /* Case: no data to exchange with ICC, only header sent by Host= 4Bytes
            Command TPDU = CLA INS P1 P2, the CCID is responsible to add P3 =00h */
            SC_ADPU.Body.LC = 0;
            SC_ADPU.Body.LE = 0;
            cmdType = 0x01;
        }
        //CLA + INS + P1 + P2 + LE
        else if (0x05 == blockLen)
        {
            SC_ADPU.Body.LC = 0;
            if (0 != ptrBlock[i])
            {
                SC_ADPU.Body.LE = ptrBlock[i];
            }
            else
            {
                SC_ADPU.Body.LE = 0x100;
            }
            i++;
            cmdType = 0x02;
        }
        //CLA + INS + P1 + P2 + LC + DATA[LC]
        //CLA + INS + P1 + P2 + LC + DATA[LC] + LE
        else if (0x05 < blockLen)
        {
            SC_ADPU.Body.LC = ptrBlock[i++];
            memcpy(SC_ADPU.Body.Data, ptrBlock + i, SC_ADPU.Body.LC);
            i += SC_ADPU.Body.LC;
            //CLA + INS + P1 + P2 + LC + DATA[LC]
            if (i == blockLen)
            {
                cmdType = 0x03;
                SC_ADPU.Body.LE = 0;
            }
            //CLA + INS + P1 + P2 + LC + DATA[LC] + LE
            else if ((i + 1) == blockLen)
            {
                cmdType = 0x04;
                if (0 != ptrBlock[i])
                {
                    SC_ADPU.Body.LE = ptrBlock[i];
                }
                else
                {
                    SC_ADPU.Body.LE = 0x100;
                }
            }
            else
            {
                return SLOTERROR_BAD_DWLENGTH;
            }
        }
        else
        {
            return SLOTERROR_BAD_DWLENGTH;
        }

        MyPrintf("case %d\r\n", cmdType);
        MyPrintf("lc %d\r\n", SC_ADPU.Body.LC);
        MyPrintf("le %d\r\n", SC_ADPU.Body.LE);

        if (3 == cmdType || 4 == cmdType)
        {
            MyPrintf("Recv:\n");
            for (index = 5; index < SC_ADPU.Body.LC + 5; index++)
            {
                SC_ADPU.Body.Data[index] = ptrBlock[index];
                MyPrintf("%02x ", SC_ADPU.Body.Data[index]);
            }
            MyPrintf("End:\n");
        }

        /************ Process the commands based on their Types from the Host ********/
        if (2 == cmdType || 4 == cmdType)
        {
            SC_Response.SW1 = 0x90;
            SC_Response.SW2 = 0x00;
            SC_ADPU.Body.LE = 10;
            //SC_Response.Data recv buffer.
            for (index = 0; index < SC_ADPU.Body.LE; index++)
            {
                SC_Response.Data[index] = index;
            }


            if (0x84 == SC_ADPU.Header.INS)
            {
                for (index = 0; index < SC_ADPU.Body.LE; index++)
                {
                    /* Copy the ATR to the Response Buffer */
                    Ccid_bulkin_data.abData[index] = SC_Response.Data[index];
                }
            }
            else
            {
                SC_ADPU.Body.LE = 0;
            }

            Ccid_bulkin_data.abData[index++] = SC_Response.SW1;
            Ccid_bulkin_data.abData[index] = SC_Response.SW2;


            /* Response Len = Expected Len + SW1, SW2*/
            Ccid_bulkin_data.dwLength = (SC_ADPU.Body.LE) + 2;

        }
        else if (1 == cmdType || 3 == cmdType)
        {
            SC_Response.SW1 = 0x90;
            SC_Response.SW2 = 0x00;
            Ccid_bulkin_data.dwLength = 2;
        }
    }
    return SLOT_NO_ERROR;

}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
