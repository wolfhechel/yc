#include "string.h"
#include "yc3121.h"
#include "yc_7816.h"
#include "yc_7816_t0.h"
#include "yc_timer.h"

uint16_t ISO7816_CalcLRC(uint8_t *pBuff, uint16_t I_Len, uint8_t *pLRC)
{
    uint16_t i;

    if ((pBuff == NULL) || (pLRC == NULL))
    {
        return ISO7816_PARA_ERROR;
    }
    *pLRC = 0;
    for (i = 0; i < I_Len; i++)
    {
        *pLRC ^= pBuff[i];
    }
    return OK;
}

uint16_t ISO7816_CheckLRC(uint8_t *pBuff, uint16_t I_Len, uint8_t LRC)
{
    uint8_t tmpLRC = 0;
    if (OK != ISO7816_CalcLRC(pBuff, I_Len, &tmpLRC))
    {
        return ISO7816_PARA_ERROR;
    }

    if (LRC != tmpLRC)
    {
        return ISO7816_T1_LRC_ERROR;
    }
    return OK;
}

uint16_t ISO7816_SendBytes_T1(uint8_t *pBuff, uint16_t Len, uint8_t state)
{
    uint8_t   i;

    if ((!Len) || (NULL == pBuff))
    {
        return ISO7816_PARA_ERROR;
    }

#ifdef DEBUG_7816
    MyPrintf("send:\n");
#endif

    if (state == 0)
    {
        while (!(SCI7816_STAT & 0x40))			//bgt
        {
            if (detect_io_state() == sci_pin.detect_state)
            {
                return ISO7816_CARD_ABSENT;
            }
        }
        SCI7816_CWT = 0;						//quick startup send
        while (SCI7816_STAT & 8)				//dummy
        {
            if (detect_io_state() == sci_pin.detect_state)
            {
                return ISO7816_CARD_ABSENT;
            }
        }
    }

    SCI7816_CWT = terminal_send_cwt - 10;

    for (i = 0; i < Len; i++)
    {
        while (SCI7816_STAT & 0x10)		//sendbuf full
        {
            if (detect_io_state() == sci_pin.detect_state)
            {
                return ISO7816_CARD_ABSENT;
            }
        }

        SCI7816_INT = 0;

        SCI7816_DATA = pBuff[i];

#ifdef DEBUG_7816
        MyPrintf("%02X ", pBuff[i]);
#endif

        SCI7816_INT = 0;
    }

    if (state == 1)
    {
        while (SCI7816_STAT & 8)
        {
            if (detect_io_state() == sci_pin.detect_state)
            {
                return ISO7816_CARD_ABSENT;
            }
            SCI7816_INT = 0;
        }

        SCI7816_CWT = terminal_rec_bwt;

        while ((SCI7816_INT & 0x02) == 0)
        {
            if (detect_io_state() == sci_pin.detect_state)
            {
                return ISO7816_CARD_ABSENT;
            }
        }

        SCI7816_CWT = terminal_rec_cwt;
    }

#ifdef DEBUG_7816
    MyPrintf("\n");
#endif
    return OK;
}



uint16_t ISO7816_RecBlock_DATA(uint8_t *RxBuf, uint16_t RxLen)
{
    uint16_t  RemainLen = 0;
    uint8_t parity_flag = 0;

    if ((!(RxLen)) || (NULL == RxBuf))
    {
        return ISO7816_PARA_ERROR;
    }

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
        if (SCI7816_STAT & 1)
        {
            *(RxBuf + RemainLen) = SCI7816_DATA;

#ifdef DEBUG_7816
            MyPrintf("%02X ", *(RxBuf + RemainLen));
#endif

            RemainLen++;

            if (SCI7816_STAT & 0x100)	//parity error
            {
                parity_flag = 1;
            }
        }
    }
    while (RemainLen < (RxLen));

    if (parity_flag)
    {
        return ISO7816_PARITY_ERROR;
    }
    return OK;
}


uint16_t ISO7816_SendBlock_Soft(uint8_t *pBuff, uint16_t I_Len, uint8_t *pHead)
{
    uint16_t  ret;
    uint8_t Lrc1, Lrc2;
    uint8_t *pAddr;
    uint16_t sendLen;
    uint8_t DataBack = 0;

    if ((NULL == pBuff) || (pHead == NULL))
    {
        return ISO7816_PARA_ERROR;
    }

    ISO7816_CalcLRC(pHead, 3, &Lrc1);

    //发送数据
    if (I_Len != 0)
    {
        ISO7816_CalcLRC(pBuff, I_Len, &Lrc2);
        if (g_T1.bLink != 0)
        {
            DataBack = pBuff[I_Len];
        }
        pBuff[I_Len] =  Lrc1 ^ Lrc2;
        pAddr = pBuff;
        sendLen = I_Len + 1;

        ret = ISO7816_SendBytes_T1(pHead, 3, 0);
        COMPARE_RETURN(ret);
    }
    else
    {
        pHead[3] = Lrc1;
        pAddr = pHead;
        sendLen = 4;
        while (!(SCI7816_STAT & 0x40))			//bgt
        {
            if (detect_io_state() == sci_pin.detect_state)
            {
                return ISO7816_CARD_ABSENT;
            }
        }
    }

    //发送数据
    ret = ISO7816_SendBytes_T1(pAddr, sendLen, 1);
    COMPARE_RETURN(ret);

    if ((g_T1.bLink != 0) && (I_Len != 0))
    {
        pBuff[I_Len] = DataBack;
    }
    return OK;
}

void ISO7816_read_recfifo(void)
{
    uint16_t num = 0;
    uint8_t data;
    while ((SCI7816_STAT & 1) && (num < 256))
    {
        data = SCI7816_DATA;
        num++;
    }
}

uint16_t ISO7816_RecBlock_Soft(uint8_t *RxBuf, uint16_t *pRxLen)
{
    uint16_t  ret;
    uint16_t tmpLen;
    uint16_t RemainLen;
    uint16_t RetBack;

    if ((NULL == pRxLen) || (NULL == RxBuf))
    {
        return ISO7816_PARA_ERROR;
    }

    ret = ISO7816_RecBlock_DATA(RxBuf, T1_HEAD_LEN);
    if ((ret == ISO7816_REC_TIMEOUT) || (ret == ISO7816_PARA_ERROR))
    {
        return ret;
    }

    RetBack = ret;

    if (RxBuf[T1_HEAD_LEN - 1] > g_T1.bIFSD)
    {
        delay_ms(100);
        ISO7816_read_recfifo();
        return ISO7816_DATALEN_ERR;
    }

    RemainLen = RxBuf[T1_HEAD_LEN - 1] + 1; //信息域 + LRC
    ret = ISO7816_RecBlock_DATA(RxBuf + T1_HEAD_LEN, RemainLen);
    if (ret != OK)
    {
        return ret;
    }

    *pRxLen =  RemainLen + T1_HEAD_LEN;
    tmpLen = *pRxLen - 1;

    if (RetBack != OK)
    {
        return RetBack;
    }

    return ISO7816_CheckLRC(RxBuf, tmpLen, RxBuf[tmpLen]);
}

uint8_t ISO7816_T1_Is_Blk_Valid(uint8_t *pProlg, uint8_t dir)
{
    uint8_t Ret = 0;

    if (pProlg[0] > 0)
    {
        Ret |= ERR_PCB;
        return Ret;
    }

    if (0 == (pProlg[1] & 0x80))
    {

        if ((pProlg[1] & 0x1F) > 0)
        {
            Ret |= ERR_PCB;
            return Ret;
        }
        if ((pProlg[1] & NS_BIT) != g_T1.bNumCard)
        {
            Ret |= ERR_PCB;
            return Ret;
        }

        //判断长度
        if ((pProlg[2] > g_T1.bIFSD) || (pProlg[2] == 0))
        {
            if (pProlg[2] == 0xff)
            {
                Ret |= ERR_FF;
            }
            else
            {
                Ret |= ERR_LEN;
            }

            return Ret;
        }

        if (g_T1.bFlagSendChain)
        {
            Ret |= ERR_PCB;
            return Ret;
        }
    }
    else if (R_BLK == (pProlg[1] & 0xC0))
    {
        if ((pProlg[1] & 0x20) != 0)
        {
            Ret |= ERR_PCB;
            return Ret;
        }

        if ((pProlg[1] & 0x0F) > 2)
        {
            Ret |= ERR_PCB;
            return Ret;
        }

        if (pProlg[2] > 0)
        {
            Ret |= ERR_LEN;
            return Ret;
        }

        if (dir == RECV)
        {
            if (((pProlg[1] & NR_BIT) << 2) == g_T1.bNum)
            {
                if (0 == g_T1.bLink)
                {
                    g_T1.bErrNotify = 1;
                    Ret |= ERR_PCB;
                    return Ret;
                }
            }
        }
        else
        {
            if (((pProlg[1] & NR_BIT) << 2) != g_T1.bNum)
            {
                g_T1.bErrNotify = 1;
                Ret |= ERR_PCB;
                return Ret;
            }
        }

    }
    else if (S_BLK == (pProlg[1] & 0xC0))
    {
        if ((pProlg[1] & 0x1C) > 0)
        {
            Ret |= ERR_PCB;
            return Ret;
        }
        if (((pProlg[1] & 0x20) > 0) && (0 == g_T1.bFlgSBSnd))
        {
            Ret |= ERR_PCB;
            return Ret;
        }

        //判断长度
        if (0 == (pProlg[1] & 0x1))
        {
            if (pProlg[2] > 0)
            {
                Ret |= ERR_LEN;
                return Ret;
            }
        }
        else
        {
            if (pProlg[2] != 1)
            {
                Ret |= ERR_LEN;
                return Ret;
            }

            if ((1 == (pProlg[1] & 0x03)) && ((pProlg[3] == 0xff) || (pProlg[3] < 0x10)))
            {
                Ret |= ERR_LEN;
                return Ret;
            }
        }
    }
    else
    {

    }

    if (g_T1.bFlgSBSnd != 0)
    {
        if (0 == (pProlg[1] & 0x80))
        {
            Ret |= ERR_RELVNT_S;
            return Ret;
        }
    }

    return Ret;
}

uint16_t ISO7816_TPDU_T1Rec(uint8_t *pRec, uint16_t *RecLen)
{
    uint16_t Ret;
    uint8_t r_cnt = 0;
    uint8_t ifs_cnt = 0;
    uint8_t s_blk[5];
    uint16_t tmpRecLen;
    uint8_t SWTXFlag = 0;

    if (g_T1.bFlgIRec == 1)
    {
        g_T1.bFlgIRec = 0;

        memcpy(pRec, &g_7816Para.aRecBuff[3],  g_7816Para.aRecBuff[2]);
        *RecLen = g_7816Para.aRecBuff[2];
    }
    else
    {
        while (1)
        {
            if (g_T1.bFlgSRec == 1)
            {
                g_T1.bFlgSRec = 0;
            }
            else
            {
                //接收数据
                Ret = ISO7816_RecBlock_Soft(g_7816Para.aRecBuff, &tmpRecLen);
                //如果校验错误，则直接跳过检查
                if (Ret == OK)
                {
                    Ret = ISO7816_T1_Is_Blk_Valid(g_7816Para.aRecBuff, RECV);
                }
                else if ((Ret == ISO7816_T1_LRC_ERROR) || (Ret == ISO7816_PARITY_ERROR))
                {
                    Ret = ERR_CHKSUM;
                }
                else if (Ret == ISO7816_DATALEN_ERR)
                {
                    Ret = ERR_FF;//ERR_LEN;;
                }
                else
                {
                    return Ret;
                }

                //恢复WTX
                if (SWTXFlag == 1)
                {
                    terminal_rec_bwt = terminal_rec_bwt_atr;
                    SWTXFlag = 0;
                }

                /*无效则发R/S*/
                if (Ret != 0)
                {
                    g_T1.bErrCnt++;

                    if ((ifs_cnt + g_T1.bErrCnt) <= 1)
                    {
                        g_T1.aRBlk[0] = 0;    //g_Nad;
                        g_T1.aRBlk[1] = 0x80 | (g_T1.bNumCard >> 2);
                        g_T1.aRBlk[2] = 0;
                        if (g_T1.bRecRBLink == 0)
                        {
                            if (Ret & (ERR_PCB | ERR_LEN | ERR_RELVNT_S))
                            {
                                g_T1.aRBlk[1] |= 2;
                            }
                            else if (Ret & (ERR_PARITY | ERR_CHKSUM))
                            {
                                g_T1.aRBlk[1] |= 1;
                            }
                            else
                            {
                                return ISO7816_ERR_NUM_OVER;
                            }
                        }
                        else
                        {
                            if ((Ret == ERR_FF) && (g_7816Para.aRecBuff[2] == 0xff))
                            {
                                return ISO7816_ERR_NUM_OVER;
                            }
                        }
                    }

                    if (g_T1.bErrCnt < ERR_SEND_CNT)
                    {
                        if (g_7816Para.aRecBuff[1] & 0x80)
                        {
                            Ret = ISO7816_SendBlock_Soft(&g_T1.aRBlk[3], 0, g_T1.aRBlk);
                            COMPARE_RETURN(Ret);
                        }
                        else
                        {
                            if (g_T1.bRecRBLink != 0)
                            {
                                if (g_T1.bErrCnt < (ERR_SEND_CNT - 1))
                                {
                                    Ret = ISO7816_SendBlock_Soft(&g_T1.aRBlk[3], 0, g_T1.aRBlk);
                                    COMPARE_RETURN(Ret);
                                }
                                else
                                {
                                    return ISO7816_ERR_NUM_OVER;
                                }
                            }
                            else
                            {
                                Ret = ISO7816_SendBlock_Soft(&g_T1.aRBlk[3], 0, g_T1.aRBlk);
                                COMPARE_RETURN(Ret);
                            }
                        }
                    }
                    else
                    {
                        return ISO7816_ERR_NUM_OVER;
                    }
                    continue;
                }
                g_T1.bErrCnt = 0;
            }

            if (R_BLK == (g_7816Para.aRecBuff[1] & 0xC0))
            {
                g_T1.aRBlk[0] = 0;
                g_T1.aRBlk[1] = 0x80 | (g_T1.bNumCard >> 2);
                g_T1.aRBlk[2] = 0;
                if (r_cnt < ERR_SEND_CNT)
                {
                    Ret = ISO7816_SendBlock_Soft(&g_T1.aRBlk[3], 0, g_T1.aRBlk);
                    COMPARE_RETURN(Ret);
                    continue;
                }
                r_cnt++;
                return ISO7816_ERR_NUM_OVER;
            }
            else if (S_BLK == (g_7816Para.aRecBuff[1] & 0xC0))
            {
                s_blk[0] = 0;
                s_blk[1] = 0xC0;
                if (0 == (g_7816Para.aRecBuff[1] & S_RQS_MSK))
                {
                    s_blk[1] |= 0x20;
                    s_blk[2]  = 0;
                    g_T1.bLink = 0;
                    g_T1.bNum = 0;
                    g_T1.bNumCard = 0;
                    g_T1.bIFSC = IFSC;
                    g_T1.bIFSD = IFSC;

                    Ret = ISO7816_SendBlock_Soft(&s_blk[3], 0, s_blk);
                    COMPARE_RETURN(Ret);
                    continue;
                }
                else if (1 == (g_7816Para.aRecBuff[1] & S_RQS_MSK))
                {
                    s_blk[1] |= 0x21;
                    s_blk[2]  = 1;
                    s_blk[3]  = g_7816Para.aRecBuff[3];
                    if ((0x10 > g_7816Para.aRecBuff[3]) || (0xFF == g_7816Para.aRecBuff[3]))
                    {
                        ifs_cnt++;

                        if (g_T1.bErrCnt < 1)
                        {
                            g_T1.aRBlk[0] = 0;

                            g_T1.aRBlk[1] = 0x82 | (g_T1.bNumCard >> 2);

                            g_T1.aRBlk[2] = 0;
                        }

                        if ((ifs_cnt + g_T1.bErrCnt) < ERR_SEND_CNT)
                        {
                            Ret = ISO7816_SendBlock_Soft(&g_T1.aRBlk[3], 0, g_T1.aRBlk);
                            COMPARE_RETURN(Ret);
                            continue;
                        }
                        return ISO7816_ERR_NUM_OVER;
                    }
                    else
                    {
                        if (s_blk[3] != g_7816Para.aRecBuff[3])
                        {
                            Ret = ISO7816_SendBlock_Soft(&s_blk[3], 1, s_blk);
                            COMPARE_RETURN(Ret);
                            continue;
                        }
                        else
                        {
                            g_T1.bIFSC = g_7816Para.aRecBuff[3];
                        }
                    }

                    if (ifs_cnt < ERR_SEND_CNT)
                    {
                        Ret = ISO7816_SendBlock_Soft(&s_blk[3], 1, s_blk);
                        COMPARE_RETURN(Ret);
                    }
                    continue;
                }
                else if (2 == (g_7816Para.aRecBuff[1] & S_RQS_MSK))
                {
                    s_blk[1] |= 0x22;
                    s_blk[2]  = 0;
                    Ret = ISO7816_SendBlock_Soft(&s_blk[3], 0, s_blk);
                    COMPARE_RETURN(Ret);

                    return ISO7816_RET_BROKEN_CHAIN;
                }
                else if ((g_7816Para.aRecBuff[1] & 0x3F) == 3)
                {
                    SWTXFlag = 1;
                    //设置等待时间
                    terminal_change_rec_bwt_num = terminal_rec_bwt_atr * g_7816Para.aRecBuff[3];
                    terminal_rec_bwt = terminal_change_rec_bwt_num;

#ifdef DEBUG_7816
                    MyPrintf("WTS\n");
                    MyPrintf("%08x", terminal_rec_bwt);
                    MyPrintf("\n");
                    MyPrintf("%08x", g_7816Para.aRecBuff[3]);
                    MyPrintf("\n");
                    MyPrintf("%08x", terminal_change_rec_bwt_num);
                    MyPrintf("\n");
#endif

                    s_blk[1] |= 0x23;
                    s_blk[2]  = 1;
                    s_blk[3]  = g_7816Para.aRecBuff[3];
                    Ret = ISO7816_SendBlock_Soft(&s_blk[3], 1, s_blk);
                    COMPARE_RETURN(Ret);
                    continue;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                memcpy(pRec, &g_7816Para.aRecBuff[3],  g_7816Para.aRecBuff[2]);
                *RecLen = g_7816Para.aRecBuff[2];
            }
            break;
        }
    }
    r_cnt = 0;

    if (R_BLK == (g_7816Para.aRecBuff[1] & 0xC0))
    {

    }
    else if (S_BLK == (g_7816Para.aRecBuff[1] & 0xC0))
    {

    }
    else
    {
        g_T1.bNumCard ^= BIT6;
        g_T1.bErrCnt = 0;

        if (M_BIT == (g_7816Para.aRecBuff[1] & M_BIT))
        {
            g_T1.bRecRBLink = 1;

            g_T1.aRBlk[0] = 0 ;
            g_T1.aRBlk[1] = 0x80;
            if ((g_7816Para.aRecBuff[1] & NS_BIT) == 0)
            {
                g_T1.aRBlk[1] |= NR_BIT;
            }
            g_T1.aRBlk[2] = 0;

            Ret = ISO7816_SendBlock_Soft(&g_T1.aRBlk[3], 0, g_T1.aRBlk);
            COMPARE_RETURN(Ret);

            return ISO7816_RET_CHAIN;
        }
        else
        {
            g_T1.bRecRBLink = 0;
        }

    }
    g_T1.bErrCnt = 0;
    return OK;
}

uint16_t ISO7816_TPDU_T1Send(uint8_t *pCommand, uint16_t CommandLength, uint8_t Type)
{
    uint16_t Ret;
    uint16_t SendLen = 0;
    uint16_t RecLen;
    uint16_t TmpLen = 0;
    uint8_t  r_cnt = 0;
    uint8_t  r_chain_cnt = 0;
    uint8_t  *pSend = NULL;
    uint8_t  prolHead[3];
    uint8_t  s_blk[5];
    uint8_t  resendflag = 0;
    uint8_t SWTXFlag = 0;

    uint8_t IFS_issued = 1;

    uint16_t CmdLen = CommandLength;

    g_T1.bFlagSendChain = 0;

    if ((NULL == pCommand) || (Type > 4))
    {
        return ISO7816_PARA_ERROR;
    }

    //初始化化
    if (Type == I_BLOCK_C)
    {
        prolHead[0] = 0;  //nad
        prolHead[1] = 0;  //pcd

        //设置序号 bNum 为BIT6
        prolHead[1] ^= g_T1.bNum;

        //计算I块长度
        if (g_T1.bIFSC < CmdLen)
        {
            //需要连接
            prolHead[1] |= BIT5;
            prolHead[2] = g_T1.bIFSC;
            CmdLen -= g_T1.bIFSC;
            SendLen = g_T1.bIFSC;
            g_T1.bFlagSendChain = 1;
            g_T1.bLink = 1;
        }
        else
        {
            prolHead[2] = CmdLen;
            SendLen = CmdLen;
            CmdLen = 0;
        }

        pSend = pCommand;
    }

    g_T1.bFlgSBSnd = 0;
    //发送数据
    while (1)
    {
        if (Type < S_BLOCK_C)
        {
            Ret = ISO7816_SendBlock_Soft(pSend, SendLen, prolHead);
            COMPARE_RETURN(Ret);

            if (resendflag == 0)
            {
                g_T1.bNum ^= BIT6;
            }
        }
        else
        {
            s_blk[0] = 0;
            s_blk[1] = 0xC0;
            TmpLen = 0;
            if (T_S_IFS == Type)
            {
                s_blk[1] |= 0x1;
                s_blk[2]  = 1;
                s_blk[3]  = pCommand[0];

                g_T1.bIFSD = pCommand[0];

                TmpLen++;
            }
            else if (T_S_ABORT == Type)
            {
                s_blk[1] |= 0x2;
                s_blk[2]  = 0;
            }
            else
            {

            }

            Ret = ISO7816_SendBlock_Soft(&s_blk[3], TmpLen, s_blk);
            COMPARE_RETURN(Ret);

            g_T1.bFlgSBSnd = 1;
        }
        if (!resendflag)
        {
            g_T1.bErrCnt = 0;
        }

        while (1)
        {

            Ret = ISO7816_RecBlock_Soft(g_7816Para.aRecBuff, &RecLen);

            g_T1.bErrNotify = 0;

            if (Ret == OK)
            {
                Ret = ISO7816_T1_Is_Blk_Valid(g_7816Para.aRecBuff, RECV);
            }
            else if ((Ret == ISO7816_T1_LRC_ERROR) || (Ret == ISO7816_PARITY_ERROR))
            {
                Ret = ERR_CHKSUM;
            }
            else if (Ret == ISO7816_DATALEN_ERR)
            {
                Ret = ERR_FF;//ERR_LEN;
            }
            else if (Ret == ISO7816_NOTLRC_ERROR)
            {
                Ret = ERR_LEN;
            }
            else
            {
                return Ret;
            }

            //恢复WTX
            if (SWTXFlag == 1)
            {
                terminal_rec_bwt = terminal_rec_bwt_atr;
                SWTXFlag = 0;
            }

            if (Ret != 0)
            {
                if (!g_T1.bErrNotify)
                {
                    g_T1.bErrCnt++;
                }
                if (Type < S_BLOCK_C)
                {
                    if (((g_T1.bErrCnt <= 1) && ((!g_T1.bErrNotify)))
                            || ((g_T1.bErrCnt == 0) && ((g_T1.bErrNotify))))
                    {
                        g_T1.aRBlk[0] = 0;
                        g_T1.aRBlk[1] = 0x80 | (g_T1.bNumCard >> 2);
                        g_T1.aRBlk[2] = 0;
                        if (Ret & (ERR_PCB | ERR_LEN | ERR_RELVNT_S))
                        {
                            g_T1.aRBlk[1] |= 2;
                        }
                        else if (Ret & (ERR_PARITY | ERR_CHKSUM))
                        {
                            g_T1.aRBlk[1] |= 1;
                        }
                        else
                        {
                            return ISO7816_ERR_NUM_OVER;
                        }
                    }
                    if (g_T1.bErrCnt < ERR_SEND_CNT)
                    {
                        Ret = ISO7816_SendBlock_Soft(&g_T1.aRBlk[3], 0, g_T1.aRBlk);
                        COMPARE_RETURN(Ret);
                    }
                }
                else
                {
                    if (g_T1.bErrCnt < ERR_SEND_CNT)
                    {
                        Ret = ISO7816_SendBlock_Soft(&s_blk[3], TmpLen, s_blk);
                        COMPARE_RETURN(Ret);
                    }
                }

                if (g_T1.bErrCnt >= ERR_SEND_CNT)
                {
                    return ISO7816_ERR_NUM_OVER;
                }
                continue;
            }

            if ((0xC2 == (g_7816Para.aRecBuff[1] * 0x80)) && (g_T1.bFlagSendChain))
            {
                s_blk[0] = 0;
                s_blk[1] = 0xE2;
                s_blk[2] = 0;
                Ret = ISO7816_SendBlock_Soft(&s_blk[3], 0, s_blk);
                COMPARE_RETURN(Ret);
                continue;
            }

            if ((g_7816Para.aRecBuff[1] == 0xc3) && (g_T1.bFlagSendChain != 0))
            {
                SWTXFlag = 1;

                //设置等待时间
                terminal_change_rec_bwt_num = terminal_rec_bwt_atr * g_7816Para.aRecBuff[3];
                terminal_rec_bwt = terminal_change_rec_bwt_num;
#ifdef DEBUG_7816
                MyPrintf("WTS\n");
                MyPrintf("%08x", terminal_rec_bwt);
                MyPrintf("\n");
                MyPrintf("%08x", g_7816Para.aRecBuff[3]);
                MyPrintf("\n");
                MyPrintf("%08x", terminal_change_rec_bwt_num);
                MyPrintf("\n");
#endif

                s_blk[0] = 0x0;
                s_blk[1] = 0xE3;
                s_blk[2]  = 1;
                s_blk[3]  = g_7816Para.aRecBuff[3];
                Ret = ISO7816_SendBlock_Soft(&s_blk[3], 1, s_blk);
                COMPARE_RETURN(Ret);

                continue;
            }
            break;
        }

        if (R_BLK == (g_7816Para.aRecBuff[1] & 0xC0))
        {
            r_cnt++;
            if (g_T1.bFlagSendChain)
            {
                r_cnt = 0;
                r_chain_cnt++;
                if ((g_7816Para.aRecBuff[1] & NR_BIT) == (g_T1.bNum >> 2)) /*nr!=ns, 对方请求下一个frame*/
                {
                    r_chain_cnt = 0;

                    pSend += SendLen;
                    prolHead[0]  = 0;
                    prolHead[1]  = 0;
                    prolHead[1] |= g_T1.bNum;

                    if (g_T1.bIFSC < CmdLen)
                    {
                        prolHead[1] |= BIT5;
                        prolHead[2] = g_T1.bIFSC;
                        CmdLen -= g_T1.bIFSC;
                        SendLen = g_T1.bIFSC;
                        g_T1.bLink = 1;
                    }
                    else
                    {
                        prolHead[2] = CmdLen;
                        SendLen = CmdLen;
                        CmdLen = 0;
                        g_T1.bFlagSendChain = 0;
                        g_T1.bLink = 0;
                    }
                }
                else
                {
                    g_T1.bNum ^= BIT6;
                }
                resendflag = 0;
            }
            else
            {
                resendflag = 1;
            }

            if ((r_cnt >= ERR_SEND_CNT) || (r_chain_cnt >= ERR_SEND_CNT))
            {
                return ISO7816_ERR_NUM_OVER;
            }
        }
        else if (S_BLK == (g_7816Para.aRecBuff[1] & 0xC0))
        {
            if (Type < S_BLOCK_C)
            {
                g_T1.bFlgSRec = 1;
            }
            else
            {
                if (IFS_issued)
                {
                    if ((0x21 != (g_7816Para.aRecBuff[1] & S_RSP_MSK))
                            || (g_7816Para.aRecBuff[3] != pCommand[0]))
                    {
                        resendflag = 1;
                        g_T1.bErrCnt++;
                        continue;
                    }
                    IFS_issued = 0;
                }

                if (0x21 == (g_7816Para.aRecBuff[1] & S_RSP_MSK))
                {
                    if (g_7816Para.aRecBuff[3] == pCommand[0])
                    {
                        g_T1.bIFSD = pCommand[0];
                    }
                    else
                    {
                        resendflag = 1;
                        continue;
                    }
                }
                else if (0x22 == (g_7816Para.aRecBuff[1] & S_RSP_MSK))
                {

                }

                else if (0x23 == (g_7816Para.aRecBuff[1] & S_RSP_MSK))
                {
                    if (g_7816Para.aRecBuff[3] == pCommand[0])
                    {
                        return ISO7816_RET_RESPONSE_DIFFERENT;
                    }
                }
                else
                {

                }
            }
            break;

        }
        else
        {
            g_T1.bFlgIRec = 1;
            break;
        }
    }

    return OK;
}

uint16_t ISO7816_TPDU_T1(uint8_t *pCommand, uint16_t CommandLength,
                         uint8_t *pResponse,  uint16_t *pResponseLength)
{
    uint16_t Ret;
    uint16_t RecLen = 0;
    uint8_t RecvBuf[REC_SIZE_MAX];
    uint8_t *pRecBuf = RecvBuf;

    if ((NULL == pCommand) || (NULL == pResponse) ||
            (CommandLength < 1) || (NULL == pResponseLength))
    {
        return ISO7816_PARA_ERROR;
    }

    Ret = ISO7816_TPDU_T1Send(pCommand, CommandLength, I_BLOCK_C);
    COMPARE_RETURN(Ret);

    *pResponseLength = 0;
    do
    {
        Ret = ISO7816_TPDU_T1Rec(pRecBuf, &RecLen);
        if ((OK != Ret) && (ISO7816_RET_CHAIN != Ret))
        {
            return ISO7816_T1_TRANSFER_ERROR;
        }
        if (REC_SIZE_MAX < RecLen)
        {
            return ISO7816_T1_TRANSFER_ERROR;
        }
        pRecBuf += RecLen;
        *pResponseLength += RecLen;
    }
    while (ISO7816_RET_CHAIN == Ret);

    memcpy(pResponse, RecvBuf, *pResponseLength);

    return OK;
}
