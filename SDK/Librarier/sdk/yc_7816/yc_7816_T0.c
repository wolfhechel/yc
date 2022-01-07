#include "string.h"
#include "yc3121.h"
#include "yc_7816.h"
#include "yc_timer.h"

uint16_t ISO7816_RecBytes(uint8_t *RxBuf, uint16_t RxLen)
{
    uint16_t  RemainLen = 0;

    if ((!(RxLen)) || (NULL == RxBuf))
    {
        return ISO7816_PARA_ERROR;
    }

    SCI7816_CWT = terminal_rec_cwt;
    time0_cnt_last = TIM0_CNT;
    do
    {
        if (detect_io_state() == sci_pin.detect_state)
        {
            return ISO7816_CARD_ABSENT;
        }
        if (SCI7816_STAT & 0x80)				//CWT
        {
            return ISO7816_REC_TIMEOUT;
        }
        if (SCI7816_STAT & 0x04)
        {
            return ISO7816_ERR_NUM_OVER;
        }

        if ((SYSCTRL_HWCTRL_VAL & 8) == 0)
        {
            return ISO7816_POWER_OFF;
        }

        #ifndef EMV_L1_TEST
        if ((time0_cnt_last - TIM0_CNT) >=contact_over_time_cnt)
        {
            #ifdef DEBUG_7816
			MyPrintf("REC_TIMEOUT\n");
			#endif
            return ISO7816_REC_TIMEOUT;
        }
        #endif

        if (SCI7816_STAT & 1)
        {
            *(RxBuf + RemainLen) = SCI7816_DATA;

#ifdef DEBUG_7816
            MyPrintf("%02X ", *(RxBuf + RemainLen));
#endif

            RemainLen++;
        }

    }
    while (RemainLen < (RxLen));

    return OK;
}

uint16_t ISO7816_SendBytes_T0(uint8_t *pBuff, uint16_t Len)
{
    uint8_t   i;

    if ((!Len) || (NULL == pBuff))
    {
        return ISO7816_PARA_ERROR;
    }

#ifdef DEBUG_7816
    MyPrintf("send:\n");
#endif
    delay_ms(2);

    time0_cnt_last = TIM0_CNT;
    SCI7816_CWT = 0;						//quick startup send
    while (SCI7816_STAT & 8)				//dummy
    {
        if (detect_io_state() == sci_pin.detect_state)
        {
            return ISO7816_CARD_ABSENT;
        }
        if ((SYSCTRL_HWCTRL_VAL & 8) == 0)
        {
            return ISO7816_POWER_OFF;
        }

        #ifndef EMV_L1_TEST
        if ((time0_cnt_last - TIM0_CNT) >=contact_over_time_cnt)
        {
            #ifdef DEBUG_7816
			MyPrintf("SEND_TIMEOUT1\n");
			#endif
            return ISO7816_SEND_TIMEOUT;
        }
        #endif
    }

    for (i = 0; i < Len; i++)
    {
        SCI7816_INT = 0;
        SCI7816_DATA = pBuff[i];
        SCI7816_CWT = terminal_send_cwt;
        while (SCI7816_STAT & 8)
        {
            if (SCI7816_STAT & 0x20)			//出错次数到上限
            {
                SCI7816_CTRL |= 0x03;	//clear tx/rx fifo
                SCI7816_STAT |= 0X24;	//clear tx/rx retry error
                //直接下电
                return ISO7816_ERR_NUM_OVER;
            }

            if (detect_io_state() == sci_pin.detect_state)
            {
                return ISO7816_CARD_ABSENT;
            }

            if ((SYSCTRL_HWCTRL_VAL & 8) == 0)
            {
                return ISO7816_POWER_OFF;
            }

            #ifndef EMV_L1_TEST
            if ((time0_cnt_last - TIM0_CNT) >=contact_over_time_cnt)
            {
                #ifdef DEBUG_7816
                MyPrintf("SEND_TIMEOUT2\n");
                #endif
                return ISO7816_SEND_TIMEOUT;
            }
            #endif
        }

#ifdef DEBUG_7816
        MyPrintf("%02X ", pBuff[i]);
#endif
    }

    while ((SCI7816_INT & 0x02) == 0)
    {
        if (detect_io_state() == sci_pin.detect_state)
        {
            return ISO7816_CARD_ABSENT;
        }

        if ((SYSCTRL_HWCTRL_VAL & 8) == 0)
        {
            return ISO7816_POWER_OFF;
        }

        #ifndef EMV_L1_TEST
        if ((time0_cnt_last - TIM0_CNT) >=contact_over_time_cnt)
        {
            #ifdef DEBUG_7816
			MyPrintf("SEND_TIMEOUT3\n");
			#endif
            return ISO7816_SEND_TIMEOUT;
        }
        #endif
    }
    SCI7816_CWT = (terminal_rec_cwt | (1 << 24));

#ifdef DEBUG_7816
    MyPrintf("\n");
#endif
    return OK;
}

uint16_t ISO7816_TPDU_T0(uint8_t *pCommand, uint16_t CommandLength,
                         uint8_t *pResponse, uint16_t *pResponseLength)

{
    uint8_t   sw1, Ins;
    uint16_t  p3;
    uint16_t  ret, tmpRecLen = 0, tmpSendOffset;

    tmpRecLen = 0;

    if ((NULL == pCommand) || (NULL == pResponse) ||
            (CommandLength < 4) || (NULL == pResponseLength))
    {
        return ISO7816_PARA_ERROR;
    }

    if (CommandLength == 4)
    {
        pCommand[4] = 0;
    }
    Ins = pCommand[1];

    g_7816Para.bFlag &= (~BIT3);

    ret = ISO7816_SendBytes_T0(pCommand, 5);
    COMPARE_RETURN(ret);

    //接收过程字节
    pResponse[tmpRecLen] = ISO7816_NULL_T0;
    do
    {
        ret = ISO7816_RecBytes(&pResponse[tmpRecLen], 1);
        COMPARE_RETURN(ret);
    }
    while (pResponse[tmpRecLen] == ISO7816_NULL_T0);   //如果是60的话等待

    sw1 = pResponse[0];
    p3 = pCommand[4];
    tmpSendOffset = 5;

    //过程字节判断
    do
    {
        //错误码或执行正确
        if ((0x60 == (sw1 & 0xF0)) || (0x90 == (sw1 & 0xF0)))
        {
            tmpRecLen++;
            ret = ISO7816_RecBytes(&pResponse[tmpRecLen++], 1);
            if (ret != OK)
            {
                return ret;
            }

            //错误码
            g_7816Para.bFlag |= BIT3;

            *pResponseLength = tmpRecLen;
            return  OK;
        }

        if (Ins == sw1)
        {
            if (5 == CommandLength)
            {
                ret = ISO7816_RecBytes(&pResponse[tmpRecLen], p3 + 2 - tmpRecLen);
                COMPARE_RETURN(ret);
                *pResponseLength = p3 + 2;
                return  OK;
            }
            else
            {
                ret = ISO7816_SendBytes_T0(pCommand + tmpSendOffset, p3);
                COMPARE_RETURN(ret);

                pResponse[tmpRecLen] = ISO7816_NULL_T0;
                //如果是60 则等待接收字节
                do
                {
                    ret = ISO7816_RecBytes(&pResponse[tmpRecLen], 1);
                    COMPARE_RETURN(ret);
                }
                while (pResponse[tmpRecLen] == ISO7816_NULL_T0);
                tmpRecLen++;

                ret = ISO7816_RecBytes(&pResponse[tmpRecLen++], 1);
                COMPARE_RETURN(ret);
                *pResponseLength = tmpRecLen;
                return  OK;
            }
        }
        else if (sw1 == (uint8_t)(~Ins)) //如果是INS的补码
        {

            if (5 == CommandLength)
            {
                ret = ISO7816_RecBytes(&pResponse[tmpRecLen++], 1);
                COMPARE_RETURN(ret);

                if (tmpRecLen >= (p3 + 2))
                {
                    *pResponseLength = tmpRecLen;
                    return  OK;
                }

                //再次接收过程字节
                ret = ISO7816_RecBytes(&pResponse[tmpRecLen], 1);
                COMPARE_RETURN(ret);

                sw1 = pResponse[tmpRecLen];

            }
            else
            {
                if (p3)
                {
                    //如果是INS的补码，一个字节一个字节传送
                    ret = ISO7816_SendBytes_T0(pCommand + tmpSendOffset, 1);
                    COMPARE_RETURN(ret);
                    tmpSendOffset++;
                    p3--;

                    //接收过程字节
                    do
                    {
                        ret = ISO7816_RecBytes(&pResponse[tmpRecLen], 1);
                        COMPARE_RETURN(ret);
                    }
                    while (pResponse[tmpRecLen] == ISO7816_NULL_T0);

                    sw1 = pResponse[tmpRecLen];

                    continue;
                }
                else
                {
                    //接收过程字节
                    do
                    {
                        ret = ISO7816_RecBytes(&pResponse[tmpRecLen], 1);
                        COMPARE_RETURN(ret);
                    }
                    while (pResponse[tmpRecLen] == ISO7816_NULL_T0);

                    sw1 = pResponse[tmpRecLen++];

                    ret = ISO7816_RecBytes(pResponse + 1, 1);
                    COMPARE_RETURN(ret);
                    *pResponseLength = 2;
                    return  OK;
                }
            }

        }
        else
        {
            return ISO7816_PROCEDURE_INS_ERROR;
        }

    }
    while (1);
}





