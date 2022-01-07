#include "yc_emv_contactless_l1.h"

/*
//注意1：EMV_Transceive函数中等待发送完成while超时退出时间为15ms，两处等待接收完成while超时退出时间为1s，RATS中while超时退出时间为1s.
//注意2：typeB的调制深度寄存器REG_TX_PA_MOD_GAIN的值设置必须大于等于8
//注意3：复位引脚由低到高后可以判断REG_PMU_STATE寄存器大于等于3时进行后续寄存器配置
//注意4：增加上电DC矫正功能，用于量产时对每个芯片矫正。
*/

typedef enum
{
    CMD_WUPA,
    CMD_REQA,
    CMD_ANTICOLLISION,
    CMD_SELECT,
    CMD_RATS,
    CMD_HLTA,
    CMD_WUPB,
    CMD_REQB,
    CMD_ATTRIB,
    CMD_HLTB,
    CMD_IBLOCK,
    CMD_SBLOCK,
    CMD_RBLOCK
} Cmd_t;



EMV_Poll_Type_t Poll_Type;

typedef struct
{
    unsigned char UID_CLn[3][5];
} YC_EMV_Contactless_L1_PICC_A_pcb_t;

static YC_EMV_Contactless_L1_PICC_A_pcb_t picc_a;

typedef struct
{
    unsigned char PUPI[4];
} YC_EMV_Contactless_L1_PICC_B_pcb_t;

static YC_EMV_Contactless_L1_PICC_B_pcb_t picc_b;

static unsigned char PCB;
static signed char WTXM;
static unsigned char FWI;
static unsigned char FSCI;
static unsigned char Block_Num;

unsigned int  Contactless_mscnt = 0;

#define 	TIM0_CNTNFC    TIM_CNT(0)
const static unsigned short FSC_Table[] = {16, 24, 32, 40, 48, 64, 96,
                                           128, 256, 512, 1024, 2048, 4096
                                          };

uint8_t gain_i_q_table[9][3] =
{
    {RX_GAIN_6DB, 0, 0},
    {RX_GAIN_12DB, 0, 0},
    {RX_GAIN_18DB, 0, 0},
    {RX_GAIN_24DB, 0, 0},
    {RX_GAIN_30DB, 0, 0},
    {RX_GAIN_36DB, 0, 0},
    {RX_GAIN_42DB, 0, 0},
    {RX_GAIN_48DB, 0, 0},
    {RX_GAIN_54DB, 0, 0},
}; /* gain,i,q */

static unsigned char bit_rate = 0;

/**
  * @brief      Set REG bit
  * @param      addr: REG addr
  * @param      mask: val
  * @retval None
  */
void SETBITMASK(unsigned char addr, unsigned char mask)
{
    unsigned char reg=0;
    reg = READREG(addr);
    WRITEREG(addr, (reg | mask));
}

/**
  * @brief      Clear REG bit
  * @param      addr: REG addr
  * @param      mask: val
  * @retval None
  */
static void CLEARBITMASK(unsigned char addr, unsigned char mask)
{
    unsigned char reg=0;
    reg = READREG(addr);
    WRITEREG(addr, (reg & (~mask)));
}

/**
  * @brief      Set time out timer.
  * @param      prescal: prescaler of time out timer.
  * @param      reload: reload of time out timer.
  * @retval None
  */
static void EMV_Set_Timer(unsigned short prescal, unsigned short reload)
{
    WRITEREG(REG_T_PRESCAL_HI, (prescal >> 8) & 0x1f);
    WRITEREG(REG_T_PRESCAL_LO, prescal & 0xff);
    WRITEREG(REG_T_RELOAD_HI, (reload >> 8) & 0xff);
    WRITEREG(REG_T_RELOAD_LO, reload & 0xff);
    WRITEREG(REG_T_CTRL, 0x38);
}

/**
  * @brief      N-th power of 2
  * @param      n: number of power
  * @retval     the result of N-th power of 2
  */
static unsigned int EMV_Power(unsigned char n)
{
    return (1 << n);
}

/**
  * @brief      Set FWT
  * @param      t: time
  * @retval None
  */
static void EMV_Set_FWT(unsigned int t)
{
    unsigned short prescal=0, reload=0;

    reload = 65535;
    prescal = t / 65535 - 1;

    EMV_Set_Timer(prescal, reload);
}


#define AntiColExceptionProcess()	do{										\
										if(errno == EMV_ERR_PARA)			\
										{									\
											return EMV_ERR_PARA;			\
										}									\
										if(errno == EMV_ERR_TRANSMISSION)	\
										{									\
											return EMV_ERR_COLLISION;		\
										}									\
										else if(errno == EMV_ERR_PROTOCOL)	\
										{									\
											return EMV_ERR_PROTOCOL;		\
										}									\
										else if(errno == EMV_ERR_TIME_OUT)  \
										{									\
											++retr;							\
											if(retr > 2)					\
												return EMV_ERR_TIME_OUT;	\
											Tretransmission();				\
										}									\
									}while(0)

#define GeneralExceptionProcess()	do{										\
										if(errno == EMV_ERR_PARA)			\
										{									\
											return EMV_ERR_PARA;			\
										}									\
										if(errno == EMV_ERR_TRANSMISSION)	\
										{									\
											return EMV_ERR_TRANSMISSION;	\
										}									\
										else if(errno == EMV_ERR_PROTOCOL)	\
										{									\
											return EMV_ERR_PROTOCOL;		\
										}									\
										else if(errno == EMV_ERR_TIME_OUT)  \
										{									\
											++retr;							\
											if(retr > 2)					\
												return EMV_ERR_TIME_OUT;	\
											Tretransmission();				\
										}									\
									}while(0)

/**
  * @brief  Common initialization
  * @param  reg_config: used for external register configuration
  * @note if not use this value, can input NULL
  * @retval None
  */
void YC_EMV_Contactless_L1_Init(void)
{
    SETBITMASK(REG_TOP_CTRL, 0x10);

    WRITEREG(REG_IRQ0_EN, 0x00);
    WRITEREG(REG_IRQ1_EN, 0x00);

    WRITEREG(REG_COLL_LEVEL, 0x06);
    WRITEREG(REG_DEMOD_CTRL, 0x88);

    WRITEREG(REG_ADC_CTRL, 0x00);

    WRITEREG(REG_TX_WAIT_CTRL, (((6780 >> 8) & 0x1f) << 2) | 0x02);
    WRITEREG(REG_TX_WAIT_FREQ, 0);
    WRITEREG(REG_TX_WAIT, 6780 & 0xff);

    WRITEREG(REG_RX_WAIT, 0x3b);
    WRITEREG(REG_RX_WAIT_FREQ, 0x04);

    WRITEREG(REG_WATER_LEVEL, WATER_LEVEL);
    WRITEREG(REG_MOD_WIDTH, 0x20);

    WRITEREG(REG_TEST_PA_CTRL, 0x14);

    SETBITMASK(REG_RSSI_CALC_CTRL, 0x02);
    SETBITMASK(REG_RSSI_CALC_CONFIG, 0x04);

    WRITEREG(REG_RX_CTRL3, RX_CTRL3);
    WRITEREG(REG_RX_CTRL4, RX_CTRL4);
    WRITEREG(REG_RX_CTRL5, RX_CTRL5);
	
	SETBITMASK(REG_RX_TYPE_B_FRAMING, 0x02);
	SETBITMASK(REG_RX_TYPE_B_FRAMING, 0x01);

    Set_Min_Level(RX_MINLEVEL);     //设置接收门限
    Set_Receive_Parameters(RX_GAIN_6DB, RX_I_R_500);       //设置接收增益及内部电阻
    Set_Carrier_Strength(PA_GAIN_VAL);  //设置发射功率
}

/**
  * @brief  Set PCD read card type
  * @param  picc_type: type
  * @note   initialization Block_Num
  * @retval None
  */
void YC_EMV_Contactless_L1_Switch_Card_Type(unsigned char picc_type)
{
    Block_Num = 0;

    if (picc_type == TYPE_A)
    {
        WRITEREG(REG_TX_PA_MOD_GAIN, 0x00);
        SETBITMASK(REG_TX_PA_GAIN_CTRL, 0x20);

        WRITEREG(REG_TX_CODER_CTRL, 0x00);
        WRITEREG(REG_RX_CODER_CTRL, 0x00);

        WRITEREG(REG_TX_CRC_CTRL, 0x04);
        WRITEREG(REG_RX_CRC_CTRL, 0x04);
    }
    else if (picc_type == TYPE_B)
    {
        WRITEREG(REG_TX_PA_MOD_GAIN, MOD_INDEX);
        CLEARBITMASK(REG_TX_PA_GAIN_CTRL, 0x20);

        WRITEREG(REG_TX_CODER_CTRL, 0x04);
        WRITEREG(REG_RX_CODER_CTRL, 0x0c);

        WRITEREG(REG_TX_CRC_CTRL, 0x12);
        WRITEREG(REG_RX_CRC_CTRL, 0x12);
    }
}

/**
  * @brief  RF switch on or off
  * @param  on: specifies which switch on or switch off Operating Field
  *   This parameter can be one of the values:
  *     @arg ON:  switch on Operating Field
  *     @arg OFF: switch off Operating Field
  * @retval None
  */
void YC_EMV_Contactless_L1_Rf_Switch(unsigned char on)
{
    if (on)
    {
        SETBITMASK(REG_TOP_CTRL, 0x01);
        SETBITMASK(REG_TX_PA_GAIN_CTRL, 0x10);
        SETBITMASK(REG_TX_PA_GAIN_CTRL, 0x20);
    }
    else
    {
        CLEARBITMASK(REG_TX_PA_GAIN_CTRL, 0x20);
        CLEARBITMASK(REG_TX_PA_GAIN_CTRL, 0x10);
		Nfc_SysTick_Delay_Ms(2);
        CLEARBITMASK(REG_TOP_CTRL, 0x01);
    }
}





void NFCtimer0_enable(unsigned long num)
{
    TIM_InitTypeDef TIM_struct;

    enable_clock(CLKCLS_TIM);

    TIM_struct.period = num;
    TIM_struct.TIMx = (TIM_NumTypeDef)0;
    TIM_Init(&TIM_struct);
    TIM_Cmd((TIM_NumTypeDef)0, ENABLE);
}

void NFC_time_start(void)
{
    NFCtimer0_enable(0xffffffff);

//    contactless_over_time_cnt = CONTACTLESS_OVER_TIME_VAL*CPU_MHZ;
	
	NFCMsCount();
	

}

unsigned int  NFC_gettim_cnt(void)
{
	
	return TIM0_CNTNFC;

}
void NFCMsCount(void)
{
	Contactless_mscnt  = (CPU_MHZ / 1000 );
	

}

unsigned char NFC_Shout_cnt(unsigned int last_cntnum ,unsigned int Num_Ms)
{

	if((last_cntnum - TIM0_CNTNFC) >= Num_Ms * Contactless_mscnt)
	{
		return EOT_SHOUT;
	
	}
		
	else 
	{
		return 0;
	
	}

}

/**
  * @brief Interface of transceiving.
  * @param cmd: command
  * @param request: data sent to PICC
  * @param requestlen: length of data sent to PICC
  * @param txalign: number of bit of last byte sent to PICC
  * @param response: data received from PICC
  * @param responselen: length of data received from PICC
  * @param rxalign: the first bit received
  * @retval Error code
  */
static unsigned char EMV_Transceive(Cmd_t cmd,
                                    const unsigned char *request, unsigned short requestlen, unsigned char txalign,
                                    unsigned char *response, unsigned short *responselen, unsigned char rxalign)
{
    unsigned short i = 0,errno=0;
    unsigned char j = 0;
    unsigned char len = 0;
    unsigned int t = 0,cnt_num = 0;
    unsigned char err_flag = 0;
    if ((cmd != CMD_HLTA) && (!response || !responselen))
    {
        return EMV_ERR_PARA;
    }
    if (!request && (requestlen != 0))
    {
        return EMV_ERR_PARA;
    }

    WRITEREG(REG_COMMAND, IDLE);

    SETBITMASK(REG_FIFO_CTRL, 0x01);
    CLEARBITMASK(REG_FIFO_CTRL, 0x01);

    WRITEREG(REG_IRQ0_CLEAR, 0xff);
    WRITEREG(REG_IRQ0_CLEAR, 0x00);
    WRITEREG(REG_IRQ1_CLEAR, 0x07);
    WRITEREG(REG_IRQ1_CLEAR, 0x00);


    if (bit_rate == TX_BAUD_RATE_106)
    {
        WRITEREG(REG_MOD_WIDTH, 0x26);
        CLEARBITMASK(REG_TX_PA_GAIN_CTRL, 0x20);
    }
    else if (bit_rate == TX_BAUD_RATE_212)
    {
        WRITEREG(REG_MOD_WIDTH, 0x12); // 16.5~20
        SETBITMASK(REG_TX_PA_GAIN_CTRL, 0x20);
    }
    else if (bit_rate == TX_BAUD_RATE_424)
    {
        WRITEREG(REG_MOD_WIDTH, 0x08); // 8~10
        SETBITMASK(REG_TX_PA_GAIN_CTRL, 0x20);
    }
    else
    {
        WRITEREG(REG_MOD_WIDTH, 0x04); // 4~5
        SETBITMASK(REG_TX_PA_GAIN_CTRL, 0x20);
    }

    if (txalign >= 8 || rxalign >= 8)
    {
        return EMV_ERR_PARA;
    }
    if (txalign != 0 || rxalign != 0)
    {
        CLEARBITMASK(REG_TX_CODER_CTRL, (BIT0 | BIT1 | BIT2) << 3);
        CLEARBITMASK(REG_RX_BIT_CTRL, BIT0 | BIT1 | BIT2);

        SETBITMASK(REG_TX_CODER_CTRL, txalign << 3);
        SETBITMASK(REG_RX_BIT_CTRL, rxalign & 0x07);
    }
    else
    {
        CLEARBITMASK(REG_TX_CODER_CTRL, (BIT0 | BIT1 | BIT2) << 3);
        CLEARBITMASK(REG_RX_BIT_CTRL, BIT0 | BIT1 | BIT2);
    }

    if (cmd == CMD_ANTICOLLISION || cmd == CMD_SELECT || cmd == CMD_RATS \
            || cmd == CMD_HLTA || cmd == CMD_IBLOCK || cmd == CMD_RBLOCK  || cmd == CMD_SBLOCK)
    {
        SETBITMASK(REG_FRAM_CON, BIT0);
        SETBITMASK(REG_FRAM_CON, BIT1);
    }
    else
    {
        CLEARBITMASK(REG_FRAM_CON, BIT0);
        SETBITMASK(REG_FRAM_CON, BIT1);
    }

    if (cmd == CMD_WUPA || cmd == CMD_REQA || cmd == CMD_ANTICOLLISION)
    {
        CLEARBITMASK(REG_TX_CRC_CTRL, BIT0);
        CLEARBITMASK(REG_RX_CRC_CTRL, BIT0);
    }
    else
    {
        SETBITMASK(REG_TX_CRC_CTRL, BIT0);
        SETBITMASK(REG_RX_CRC_CTRL, BIT0);
    }

    if (cmd == CMD_ANTICOLLISION || cmd == CMD_SELECT)
    {
        EMV_Set_Timer(0, 1400);        //9 * 128 + 84/20
    }

    else if (cmd == CMD_WUPA || cmd == CMD_REQA)
    {
        EMV_Set_Timer(WAIT_ATQA_PRESCALER, WAIT_ATQA_RELOAD);        //9 * 128 + 84/20
    }

    else if (cmd == CMD_RATS)
    {
        EMV_Set_Timer(4, 58812);        //FWT,ACTIVATION + Delta,Tpcd(16.4ms)
    }

    else if (cmd == CMD_WUPB || cmd == CMD_REQB)
    {
        EMV_Set_Timer(WAIT_ATQB_PRESCALER, WAIT_ATQB_RELOAD);        //FWT,ATQB + Delta,Tpcd(16.4ms)
    }

    else if (cmd == CMD_IBLOCK || cmd == CMD_RBLOCK || cmd == CMD_ATTRIB || cmd == CMD_HLTB)
    {
        t = 256 * 16 * EMV_Power(FWI) + 49152 + 222384;                         //FWT + Delta,FWT + Delta,Tpcd
        EMV_Set_FWT(t);
    }
    else if (cmd == CMD_SBLOCK)
    {

        if (WTXM * EMV_Power(FWI) > EMV_Power(14))
        {
            t = 256 * 16 * EMV_Power(14) + 49152 + 222384;                  //FWT + Delta,FWT + Delta,Tpcd
            EMV_Set_FWT(t);
        }
        else
        {
            t = WTXM * 256 * 16 * EMV_Power(FWI) + 49152 + 222384;  //WTXM * FWT + Delta,FWT + Delta,Tpcd
            EMV_Set_FWT(t);
        }
    }


    if (cmd == CMD_IBLOCK)
    {
        WRITEREG(REG_FIFO_DATA, PCB);
    }
    for (i = 0; i < requestlen && i < FIFO_LENGTH - 3; i++)
    {
        WRITEREG(REG_FIFO_DATA, request[i]);
    }


    SETBITMASK(REG_IRQ0_CLEAR, 0x20);
    CLEARBITMASK(REG_IRQ0_CLEAR, 0x20);


    if (cmd == CMD_HLTA)
    {
        WRITEREG(REG_COMMAND, TRANSMIT);
    }
    else
    {
        WRITEREG(REG_COMMAND, TRANSCEIVE);
    }
    while (i < requestlen)
    {

        if ((READREG(REG_IRQ0_STATE) & 0x20) != 0)
        {
            if (requestlen - i > FIFO_LENGTH - WATER_LEVEL - 2)
            {
                for (j = 0; j < FIFO_LENGTH - WATER_LEVEL - 2; j++)
                {
                    WRITEREG(REG_FIFO_DATA, request[i++]);
                }
            }
            else
            {
                for (; i < requestlen; i++)
                {
                    WRITEREG(REG_FIFO_DATA, request[i]);
                }
            }


            SETBITMASK(REG_IRQ0_CLEAR, 0x20);
            CLEARBITMASK(REG_IRQ0_CLEAR, 0x20);
        }
    }
	cnt_num = NFC_gettim_cnt();
	while ((READREG(REG_IRQ0_STATE) & 0x08) == 0)
	{
		#ifndef NFC_L1_TEST
			errno = NFC_Shout_cnt(cnt_num,15);
			if(errno != EMV_ERR_NONE)
			{
				#ifdef  SDK_DEBUG
				MyPrintf("0 EOT_SHOUT\n");
				#endif
				return errno;
			}
		#endif
	}
	


    SETBITMASK(REG_IRQ0_CLEAR, 0x40);
    CLEARBITMASK(REG_IRQ0_CLEAR, 0x40);

    if (cmd == CMD_HLTA)
    {
        return EMV_ERR_NONE;
    }
	cnt_num = NFC_gettim_cnt();
    while (1)
    {
        if ((READREG(REG_IRQ0_STATE) & 0x01) != 0)
        {
            return EMV_ERR_TIME_OUT;
        }
        if ((READREG(REG_IRQ0_STATE) & 0x04) != 0)
        {
            break;
        }


        if ((READREG(REG_IRQ0_STATE) & 0x40) != 0)
        {
            break;
        }
		#ifndef NFC_L1_TEST
			errno = NFC_Shout_cnt(cnt_num,1000);
			if(errno != EMV_ERR_NONE)
			{
				#ifdef  SDK_DEBUG
				MyPrintf("1 EOT_SHOUT\n");
				#endif
				return errno;
			}
		#endif
		
    }
    *responselen = 0;
	cnt_num = NFC_gettim_cnt();
	while ((READREG(REG_IRQ0_STATE) & 0x04) == 0)
	{
        if ((READREG(REG_IRQ0_STATE) & 0x40) != 0)
        {
            for (i = 0; i < FIFO_LENGTH - WATER_LEVEL; i++)
            {

                if (*responselen > FSD)
                {
                    return EMV_ERR_OVERFLOW;
                }
                if (*responselen < FSD)
                {
                    response[*responselen] = READREG(REG_FIFO_DATA);
                }
                else
                {
                    READREG(REG_FIFO_DATA);
                }

                *responselen += 1;
            }
            SETBITMASK(REG_IRQ0_CLEAR, 0x40);
            CLEARBITMASK(REG_IRQ0_CLEAR, 0x40);
        }
		#ifndef NFC_L1_TEST
			errno = NFC_Shout_cnt(cnt_num,1000);
			if(errno != EMV_ERR_NONE)
			{
				#ifdef  SDK_DEBUG
				MyPrintf("2 EOT_SHOUT\n");
				#endif
				return errno;
			}
		#endif
    }

    len = READREG(REG_FIFO_LENGTH);
    for (i = 0; i < len; i++)
    {
        if (*responselen > FSD)
        {
            return EMV_ERR_OVERFLOW;
        }
        if (*responselen < FSD)
        {
            response[*responselen] = READREG(REG_FIFO_DATA);
        }
        else
        {
            READREG(REG_FIFO_DATA);
        }

        *responselen += 1;
    }

    err_flag = READREG(REG_ERROR_FLAG);

    if ((err_flag & 0x80) || (err_flag & 0x04) || (err_flag & 0x02) || (err_flag & 0x01))
    {
        return EMV_ERR_TRANSMISSION;
    }
    return EMV_ERR_NONE;
}

/**
  * @brief  APDU processing
  * @param  inf: APDU
  * @param  inflen: APDU length
  * @param  res: APDU response
  * @param  reslen: APDU response length
  * @retval Error code
  */
unsigned char YC_EMV_Contactless_L1_APDU_Transceive(const unsigned char *inf,
        unsigned short inflen,
        unsigned char *res,
        unsigned short *reslen)
{
    unsigned short i=0;

    unsigned char Recv[FSD] = {0};
    unsigned char S_Block[2] = {0};
    unsigned char R_Block[1] = {0};

    unsigned short FSC = 0;
    unsigned char Send_Len = 0;
    unsigned short Recv_Len = 0;
    unsigned char Send_Offset = 0;
    unsigned char Recv_Offset = 0;

    unsigned char Swtx=0;

    unsigned char Time_Out=0;

    unsigned char Tran_Err=0;

    unsigned char Retran=0;

    unsigned char Recieving=0;
    unsigned char errno = EMV_ERR_NONE;

    if (!inf || !res || !reslen)
    {
        return EMV_ERR_PARA;
    }

#define ACK 0x0
#define NAK 0x10
#define SendFinish()    ((PCB & 0x10) == 0)
#define SendRB(type)    do{                                                                                                                                                     \
                                                    if(Swtx > 2)                                                                                                                    \
                                                    {                                                                                                                                               \
                                                            return EMV_ERR_TIME_OUT;                                                                                        \
                                                    }                                                                                                                                               \
                                                    if(Time_Out > 2)                                                                                                                \
                                                    {                                                                                                                                               \
                                                            return  EMV_ERR_TIME_OUT;                                                                                       \
                                                    }                                                                                                                                               \
                                                    if(Tran_Err > 2)                                                                                                                \
                                                    {                                                                                                                                               \
                                                            return EMV_ERR_TRANSMISSION;                                                                            \
                                                    }                                                                                                                                               \
                                                    R_Block[0] = (R_BLOCK << 6) | type | 0x22 | Block_Num;                                  \
                                                    errno = EMV_Transceive(CMD_RBLOCK, R_Block, 1, 0, Recv, &Recv_Len, 0);  \
                                            }while(0)

    FSC = FSC_Table[FSCI];
    if (FSC > FSC_CHAINING)
    {
        FSC = FSC_CHAINING;
    }

    *reslen = 0;

    while (1)
    {
        if ((inflen - Send_Offset) > FSC - 3)
        {
            Send_Len = FSC - 3;
            PCB = 0x12;
        }
        else
        {
            Send_Len = inflen - Send_Offset;
            PCB = 0x02;
        }
        PCB |= Block_Num;
        Retran = 0;

send:
        Swtx = 0;
        Time_Out = 0;
        Tran_Err = 0;
        Recieving = 0;
        errno = EMV_Transceive(CMD_IBLOCK, inf + Send_Offset, Send_Len, 0, Recv, &Recv_Len, 0);

check:
        if (errno == EOT_SHOUT)
        {
            return EOT_SHOUT;
        }
        if (errno == EMV_ERR_TIME_OUT)
        {
            Time_Out += 1;

            if (Recieving)
            {
                SendRB(ACK);
            }
            else
            {
                SendRB(NAK);
            }

            goto check;
        }

        if (errno == EMV_ERR_TRANSMISSION)
        {
            Tran_Err += 1;

            if (Recieving)
            {
                SendRB(ACK);
            }
            else
            {
                SendRB(NAK);
            }

            goto check;
        }

        switch ((Recv[0] >> 6) & 0x03)
        {
        case I_BLOCK:
            if (Recv_Len + 2 > FSD)                                         //I-Block of length > FSD
            {
                return EMV_ERR_PROTOCOL;
            }
            if ((Recv[0] & 0x20) || !(Recv[0] & 0x02))      //b6 must be 0, b2 must be 1.
            {
                return EMV_ERR_PROTOCOL;
            }
            if ((Recv[0] & 0x08) || (Recv[0] & 0x04))       //CID and NAD are not allowed.
            {
                return EMV_ERR_PROTOCOL;
            }
            if ((Recv[0] & 0x01) != Block_Num)
            {
                return EMV_ERR_PROTOCOL;
            }

            if (!SendFinish())
            {
                return EMV_ERR_PROTOCOL;
            }

            Block_Num ^= 0x01;
            *reslen += Recv_Len - 1;
            for (i = 1; i < Recv_Len; i++)
            {
                (res + Recv_Offset)[i - 1] = Recv[i];
            }
            Recv_Offset += Recv_Len - 1;

            if (Recv[0] & 0x10)
            {
                Swtx = 0;
                Time_Out = 0;
                Tran_Err = 0;
                Recieving = 1;

                SendRB(ACK);

                goto check;
            }
            else
            {
                return EMV_ERR_NONE;
            }

        case R_BLOCK:
            if (!(Recv[0] & 0x20) || !(Recv[0] & 0x02))
            {
                return EMV_ERR_PROTOCOL;
            }
            if ((Recv[0] & 0x08) || (Recv[0] & 0x04))
            {
                return EMV_ERR_PROTOCOL;
            }
            if (Recv[0] & 0x10)
            {
                return EMV_ERR_PROTOCOL;
            }
            else
            {
                if (Recieving)
                {
                    return EMV_ERR_PROTOCOL;
                }


                if ((Recv[0] & 0x01) != Block_Num)
                {
                    if (++Retran > 2)
                    {
                        return EMV_ERR_PROTOCOL;
                    }

                    goto send;
                }


                if (SendFinish())
                {
                    return EMV_ERR_PROTOCOL;
                }


                Block_Num ^= 0x01;
                Send_Offset += Send_Len;
            }

            break;

        case S_BLOCK:
            if (Recv[0] & 0x01)
            {
                return EMV_ERR_PROTOCOL;
            }
            if ((Recv[0] & 0x04) || (Recv[0] & 0x08))
            {
                return EMV_ERR_PROTOCOL;
            }
            if (!(Recv[0] & 0x02) || ((Recv[0] & 0x30) != 0x30))
            {
                return EMV_ERR_PROTOCOL;
            }


            WTXM = Recv[1] & 0x3f;
            if ((WTXM < 1) || (WTXM > 59))
            {
                return EMV_ERR_PROTOCOL;
            }


            S_Block[0] = (S_BLOCK << 6) | 0x30 | 0x02;

            S_Block[1] = WTXM;

            errno = EMV_Transceive(CMD_SBLOCK, S_Block, 2, 0, Recv, &Recv_Len, 0);

            Swtx += 1;

            goto check;

        default:
            return EMV_ERR_PROTOCOL;
        }
    }
}

/*********************************************************** Type A command ***************************************************************/
/**
  * @brief  WUPA
  * @param  None
  * @retval Error code
  */
unsigned char YC_EMV_Contactless_L1_WUPA(void)
{
    unsigned char i=0;
    unsigned short len = 0;
    unsigned char errno = EMV_ERR_NONE;
    unsigned char WUPA[1]={0};
    unsigned char ATQA[FSD]={0};


    YC_EMV_Contactless_L1_Switch_Card_Type(TYPE_A);
    Tp();
    WUPA[0] = 0x52;


#ifdef SDK_DEBUG
    PRINT("WUPA ->: %02x\r\n", WUPA[0]);
#endif
    Set_Receive_Parameters(RX_GAIN_18DB, RX_I_R_500);

    errno = EMV_Transceive(CMD_WUPA, WUPA, 1, 7, ATQA, &len, 0);
    if (errno == EMV_ERR_NONE)
    {

#ifdef SDK_DEBUG
        PRINT("ATQA <-: ");
        for (i = 0; i < len; i++)
        {
            PRINT("%02x ", ATQA[i]);
        }
        PRINT("\r\n");
#endif
        if (len != 2)
        {
            return EMV_ERR_PROTOCOL;
        }
    }

    return errno;
}

/**
  * @brief  REQA
  * @param  None
  * @retval Error code
  */
unsigned char YC_EMV_Contactless_L1_REQA(void)
{
    unsigned char i=0;
    unsigned short len = 0;
    unsigned char errno = EMV_ERR_NONE;
    unsigned char REQA[1]={0};
    unsigned char ATQA[FSD]={0};

    REQA[0] = 0x26;


#ifdef  SDK_DEBUG
    PRINT("REQA ->: %02x\r\n", REQA[0]);
#endif

    errno = EMV_Transceive(CMD_REQA, REQA, 1, 7, ATQA, &len, 0);


    if (errno == EMV_ERR_NONE)
    {
#ifdef SDK_DEBUG
        PRINT("ATQA <-: ");
        for (i = 0; i < len; i++)
        {
            PRINT("%02x ", ATQA[i]);
        }
        PRINT("\r\n");
#endif

        if (len != 2)
        {
            return EMV_ERR_PROTOCOL;
        }
    }

    return errno;
}

/**
  * @brief  ANTICOLLISION
  * @param  Cascade level�� specifies cascade level
  *   This parameter can be one of the values:
  *     @arg CL1: cascase level 1
  *     @arg CL2: cascase level 2
  *     @arg CL2: cascase level 3
  * @retval Error code
  */
unsigned char YC_EMV_Contactless_L1_ANTICOLLISION(unsigned char cl)
{
    unsigned char i=0;
    unsigned short len = 0;
    unsigned char errno = EMV_ERR_NONE;
    unsigned char ANTICOLLISION[2]={0};
    unsigned char  UID_CLn_Temp[FSD]={0};

    ANTICOLLISION[0] = cl;
    ANTICOLLISION[1] = 0x20;


    SETBITMASK(REG_RX_BIT_CTRL, 0x10);
    CLEARBITMASK(REG_RX_BIT_CTRL, 0x08);

#ifdef SDK_DEBUG
    PRINT("ANTICOLLISION ->: ");
    for (i = 0; i < 2; i++)
    {
        PRINT("%02x ", ANTICOLLISION[i]);
    }
    PRINT("\r\n");
#endif

    errno = EMV_Transceive(CMD_ANTICOLLISION, ANTICOLLISION, 2, 0, UID_CLn_Temp, &len, 0);

#ifdef SDK_DEBUG
    PRINT("\r\n");
#endif
    CLEARBITMASK(REG_RX_BIT_CTRL, 0x10);
    SETBITMASK(REG_RX_BIT_CTRL, 0x08);


    if (errno == EMV_ERR_NONE)
    {

#ifdef SDK_DEBUG
        PRINT("UID CLn <-: ");
        for (i = 0; i < len; i++)
        {
            PRINT("%02x ", UID_CLn_Temp[i]);
        }
        PRINT("\r\n");
#endif
        if (len != 5)
        {
            return EMV_ERR_PROTOCOL;
        }


        if (UID_CLn_Temp[4] != (((UID_CLn_Temp[0] ^ UID_CLn_Temp[1]) ^ UID_CLn_Temp[2]) ^ UID_CLn_Temp[3]))
        {
            return EMV_ERR_TRANSMISSION;

        }
        switch (cl)
        {
        case CL1:
            for (i = 0; i < len; i++)
            {
                picc_a.UID_CLn[0][i] = UID_CLn_Temp[i];
            }
            break;

        case CL2:
            for (i = 0; i < len; i++)
            {
                picc_a.UID_CLn[1][i] = UID_CLn_Temp[i];
            }
            break;

        case CL3:
            for (i = 0; i < len; i++)
            {
                picc_a.UID_CLn[2][i] = UID_CLn_Temp[i];
            }
            break;

        default:
            return EMV_ERR_PARA;
        }
    }

    return errno;
}

/**
  * @brief  SELECT
  * @note   When UID is complete, then check if PICC is compliant with ISO/IEC 14443-4
  * @param  cl�� specifies cascade level
  *   This parameter can be one of the values:
  *     @arg CL1: cascase level 1
  *     @arg CL2: cascase level 2
  *     @arg CL2: cascase level 3
  * @param  UID_complete:
  *                       1: UID not complete
  *                       0: UID complete
  *     @note if not use this value, can input NULL
  * @param  ISOIEC14443_4_compliant:
  *           1: PICC compliant with ISO/IEC 14443-4
  *           0: PICC not compliant with ISO/IEC 14443-4
  *     @note if not use this value, can input NULL
  * @retval Error code
  */
unsigned char YC_EMV_Contactless_L1_SELECT(unsigned char cl,
        unsigned char *UID_complete,
        unsigned char *ISOIEC14443_4_compliant)
{
    unsigned char i = 0;
    unsigned short len = 0;
    unsigned char errno = EMV_ERR_NONE;
    unsigned char SELECT[7] = {0};
    unsigned char SAK[FSD] = {0};

    SELECT[0] = cl;
    SELECT[1] = 0x70;

    switch (cl)
    {
    case CL1:
        for (i = 0; i < 5; i++)
        {
            SELECT[2 + i] = picc_a.UID_CLn[0][i];
        }
        break;

    case CL2:
        for (i = 0; i < 5; i++)
        {
            SELECT[2 + i] = picc_a.UID_CLn[1][i];
        }
        break;

    case CL3:
        for (i = 0; i < 5; i++)
        {
            SELECT[2 + i] = picc_a.UID_CLn[2][i];
        }
        break;

    default:
        return EMV_ERR_PARA;
    }


#ifdef SDK_DEBUG
    PRINT("SELECT ->: ");
    for (i = 0; i < 7; i++)
    {
        PRINT("%02x ", SELECT[i]);
    }
    PRINT("\r\n");
#endif


    errno = EMV_Transceive(CMD_SELECT, SELECT, 7, 0, SAK, &len, 0);

    if (errno == EMV_ERR_NONE)
    {
#ifdef SDK_DEBUG
        PRINT("SAK <-: ");
        for (i = 0; i < len; i++)
        {
            PRINT("%02x ", SAK[i]);
        }
        PRINT("\r\n");
#endif

        if (len != 1)
        {
            return EMV_ERR_PROTOCOL;
        }

        if (UID_complete)
        {
            *UID_complete = (SAK[0] >> 2) & 0x01;
        }

        if (ISOIEC14443_4_compliant)
        {
            *ISOIEC14443_4_compliant = (SAK[0] >> 5) & 0x01;
        }
    }

    return errno;
}

/**
  * @brief  RATS
  * @param  None
  * @retval Error code
  */
unsigned char YC_EMV_Contactless_L1_RATS(void)
{
    unsigned char i = 0;
    unsigned short len = 0;
    unsigned char SFGI = 0;
    unsigned char TA = 0, TB = 0, TC = 0;
    unsigned char errno = EMV_ERR_NONE;
    unsigned char RATS[2]={0};
	unsigned int cnt_num = 0;
    unsigned char ATS[FSD]={0};

    RATS[0] = 0xE0;

    RATS[1] = ((0x8 & 0xf) << 4);

    SETBITMASK(REG_RX_CODER_CTRL, 0x20);

#ifdef SDK_DEBUG
    PRINT("RATS ->: ");
    for (i = 0; i < 2; i++)
    {
        PRINT("%02x ", RATS[i]);
    }
    PRINT("\r\n");
#endif

    errno = EMV_Transceive(CMD_RATS, RATS, 2, 0, ATS, &len, 0);

    if (errno == EMV_ERR_NONE)
    {
#ifdef SDK_DEBUG
        PRINT("ATS <-: ");
        for (i = 0; i < len; i++)
        {
            PRINT("%02x ", ATS[i]);
        }
        PRINT("\r\n");
#endif

        if (ATS[0] != len)
        {
            return EMV_ERR_PROTOCOL;
        }

        FWI = 4;
        FSCI = 2;

        if (ATS[0] != 1)
        {
            if ((ATS[1] & 0xf) > 0xc)
            {
                FSCI = 0xc;
            }
            else
            {
                FSCI = ATS[1] & 0xf;
            }

            if (ATS[1] & 0x10)
            {
                TA = 1;
            }

            if (ATS[1] & 0x20)
            {
                TB = 1;
            }

            if (ATS[1] & 0x40)
            {
                TC = 1;
            }

            if (TA)
            {


            }

            if (TB)
            {

                if (((ATS[2 + TA] >> 4) & 0x0f) == 15)
                {
                    FWI = 4;
                }
                else
                {
                    FWI = (ATS[2 + TA] >> 4) & 0x0f;
                }


                if ((ATS[2 + TA] & 0x0f) == 15)
                {

                }
                else
                {
                    SFGI = ATS[2 + TA] & 0x0f;

                    WRITEREG(REG_T_PRESCAL_HI, (4480 >> 8) & 0x1f);
                    WRITEREG(REG_T_PRESCAL_LO, 4480 & 0xff);
                    WRITEREG(REG_T_RELOAD_HI, (EMV_Power(SFGI) >> 8) & 0xff);
                    WRITEREG(REG_T_RELOAD_LO, EMV_Power(SFGI) & 0xff);


                    SETBITMASK(REG_IRQ0_CLEAR, 0x01);
                    CLEARBITMASK(REG_IRQ0_CLEAR, 0x01);


                    WRITEREG(REG_T_CTRL, 0x01);
                    WRITEREG(REG_T_CTRL, 0x00);
					cnt_num = NFC_gettim_cnt();
                    while ((READREG(REG_IRQ0_STATE) & 0x01) == 0)
					{
						#ifndef NFC_L1_TEST
							errno = NFC_Shout_cnt(cnt_num,1000);
							if(errno != EMV_ERR_NONE)
							{
								#ifdef  SDK_DEBUG
								MyPrintf("3 EOT_SHOUT\n");
								#endif
								return errno;
							}
						#endif
					}
                }
            }
            if (TC)
            {

            }


        }
    }

    return errno;
}

/**
  * @brief  HLTA
  * @param  None
  * @retval None
  */
void YC_EMV_Contactless_L1_HLTA(void)
{
    unsigned char i = 0;
    unsigned char HLTA[2] = {0};

    HLTA[0] = 0x50;
    HLTA[1] = 0x00;

#ifdef SDK_DEBUG
    PRINT("HLTA ->: ");
    for (i = 0; i < 2; i++)
    {
        PRINT("%02x ", HLTA[i]);
    }
    PRINT("\r\n");
#endif

    EMV_Transceive(CMD_HLTA, HLTA, 2, 0, NULL, NULL, 0);
}

/*********************************************************** Type B command ***************************************************************/
/**
  * @brief  WUPB
  * @param  ISOIEC14443_4_compliant:
  *           1: PICC compliant with ISO/IEC 14443-4
  *           0: PICC not compliant with ISO/IEC 14443-4
  *     @note if not use this value, can input NULL
  * @retval Error code
  */
unsigned char YC_EMV_Contactless_L1_WUPB(unsigned char *ISOIEC14443_4_compliant)
{
    unsigned char i = 0;
    unsigned short len = 0;
    unsigned char errno = EMV_ERR_NONE;
    unsigned char ATQB[FSD] = {0};
    unsigned char WUPB[3] = {0};

    YC_EMV_Contactless_L1_Switch_Card_Type(TYPE_B);
    Nfc_SysTick_Delay_Ms(3);

    WUPB[0] = 0x05;
    WUPB[1] = 0x00;
    WUPB[2] = 0x08;

#ifdef SDK_DEBUG
    PRINT("WUPB ->: ");
    for (i = 0; i < 3; i++)
    {
        PRINT("%02x ", WUPB[i]);
    }
    PRINT("\r\n");
#endif


    errno = EMV_Transceive(CMD_WUPB, WUPB, 3, 0, ATQB, &len, 0);

    if (errno == EMV_ERR_NONE)
    {

#ifdef SDK_DEBUG
        PRINT("ATQB <-: ");
        for (i = 0; i < len; i++)
        {
            PRINT("%02x ", ATQB[i]);
        }
        PRINT("\r\n");
#endif

        if (ATQB[0] != 0x50)
        {
            return EMV_ERR_PROTOCOL;
        }

        if (len < 12)
        {
            return EMV_ERR_PROTOCOL;
        }


        for (i = 0; i < 4; i++)
        {
            picc_b.PUPI[i] = ATQB[1 + i];
        }


        if (((ATQB[10] >> 4) & 0xf) > 0xc)
        {
            FSCI = 0xc;
        }
        else
        {
            FSCI = (ATQB[10] >> 4) & 0xf;
        }


        if (ISOIEC14443_4_compliant)
        {
            *ISOIEC14443_4_compliant = ATQB[10] & 0x1;
        }

        if ((ATQB[10] & 0x8) != 0)
        {
            return EMV_ERR_PROTOCOL;
        }


        if (((ATQB[11] >> 4) & 0xf) == 15)
        {
            FWI = 4;
        }
        else
        {
            FWI = (ATQB[11] >> 4) & 0xf;
        }
    }
    return errno;
}

/**
  * @brief  REQB
  * @param  ISOIEC14443_4_compliant:
  *           1: PICC compliant with ISO/IEC 14443-4
  *           0: PICC not compliant with ISO/IEC 14443-4
  *     @note if not use this value, can input NULL
  * @retval Error code
  */
unsigned char YC_EMV_Contactless_L1_REQB(unsigned char *ISOIEC14443_4_compliant)
{
    unsigned char i = 0;
    unsigned short len = 0;
    unsigned char errno = EMV_ERR_NONE;
    unsigned char ATQB[FSD] = {0};
    unsigned char REQB[3] = {0};

    YC_EMV_Contactless_L1_Switch_Card_Type(TYPE_B);
    Nfc_SysTick_Delay_Ms(3);

    REQB[0] = 0x05;

    REQB[1] = 0x00;

    REQB[2] = 0x00;


#ifdef SDK_DEBUG
    PRINT("REQB ->: ");
    for (i = 0; i < 3; i++)
    {
        PRINT("%02x ", REQB[i]);
    }
    PRINT("\r\n");
#endif

    errno = EMV_Transceive(CMD_REQB, REQB, 3, 0, ATQB, &len, 0);

    if (errno == EMV_ERR_NONE)
    {


#ifdef SDK_DEBUG
        PRINT("ATQB <-: ");
        for (i = 0; i < len; i++)
        {
            PRINT("%02x ", ATQB[i]);
        }
        PRINT("\r\n");
#endif

        if (ATQB[0] != 0x50)
        {
            return EMV_ERR_PROTOCOL;
        }

        if (len < 12)
        {
            return EMV_ERR_PROTOCOL;
        }


        for (i = 0; i < 4; i++)
        {
            picc_b.PUPI[i] = ATQB[1 + i];
        }


        if (((ATQB[10] >> 4) & 0xf) > 0xc)
        {
            FSCI = 0xc;
        }
        else
        {
            FSCI = (ATQB[10] >> 4) & 0xf;
        }


        if (ISOIEC14443_4_compliant)
        {
            *ISOIEC14443_4_compliant = ATQB[10] & 0x1;
        }

        if ((ATQB[10] & 0x8) != 0)
        {
            return EMV_ERR_PROTOCOL;
        }


        if (((ATQB[11] >> 4) & 0xf) == 15)
        {
            FWI = 4;
        }
        else
        {
            FWI = (ATQB[11] >> 4) & 0xf;
        }

    }
    return errno;
}

/**
  * @brief  ATTRIB
  * @param  None
  * @retval Error code
  */
unsigned char YC_EMV_Contactless_L1_ATTRIB(void)
{
    unsigned char i = 0;
    unsigned short len = 0;
    unsigned char errno = EMV_ERR_NONE;
    unsigned char ATTRIB[9] = {0};
    unsigned char  ATTRIB_RES[FSD] = {0};

    ATTRIB[0] = 0x1D;

    ATTRIB[1] = picc_b.PUPI[0];
    ATTRIB[2] = picc_b.PUPI[1];
    ATTRIB[3] = picc_b.PUPI[2];
    ATTRIB[4] = picc_b.PUPI[3];

    ATTRIB[5] = 0x00;

    ATTRIB[6] = 0x8 & 0xf;

    ATTRIB[7] = 0x01;

    ATTRIB[8] = 0x00;


    SETBITMASK(REG_RX_CODER_CTRL, 0x20);

#ifdef SDK_DEBUG
    PRINT("ATTRIB ->: ");
    for (i = 0; i < 9; i++)
    {
        PRINT("%02x ", ATTRIB[i]);
    }
    PRINT("\r\n");
#endif

    errno = EMV_Transceive(CMD_ATTRIB, ATTRIB, 9, 0, ATTRIB_RES, &len, 0);

    if (errno == EMV_ERR_NONE)
    {


#ifdef SDK_DEBUG
        PRINT("ATTRIB_RES <-: ");
        for (i = 0; i < len; i++)
        {
            PRINT("%02x ", ATTRIB_RES[i]);
        }
        PRINT("\r\n");
#endif

        if (len != 1)
        {
            return EMV_ERR_PROTOCOL;
        }


        if ((ATTRIB_RES[0] & 0xf) != 0x00)
        {
            return EMV_ERR_PROTOCOL;
        }
    }
    return errno;
}

/**
  * @brief  HLTB
  * @param  None
  * @retval Error code
  */
unsigned char YC_EMV_Contactless_L1_HLTB(void)
{
    unsigned char i = 0;
    unsigned short len = 0;
    unsigned char errno = EMV_ERR_NONE;
    unsigned char HLTB[5] = {0};
    unsigned char HLTB_RES[FSD] = {0};

    HLTB[0] = 0x50;
    HLTB[1] = picc_b.PUPI[0];
    HLTB[2] = picc_b.PUPI[1];
    HLTB[3] = picc_b.PUPI[2];
    HLTB[4] = picc_b.PUPI[3];

#ifdef SDK_DEBUG
    PRINT("HLTB ->: ");
    for (i = 0; i < 5; i++)
    {
        PRINT("%02x ", HLTB[i]);
    }
    PRINT("\r\n");
#endif

    errno = EMV_Transceive(CMD_HLTB, HLTB, 5, 0, HLTB_RES, (unsigned short *)&len, 0);



#ifdef SDK_DEBUG
    PRINT("HLTB_RES <-: ");
    for (i = 0; i < len; i++)
    {
        PRINT("%02x ", HLTB_RES[i]);
    }
    PRINT("\r\n");
#endif

    return errno;
}

static uint8_t EMV_Poll(void)
{
    uint8_t errno = 0;
    Poll_Type.Type_A = 0;
    Poll_Type.Type_B = 0;

    while (1)
    {

        if (!Poll_Type.Type_A)
        {
            Tp();
            errno  = YC_EMV_Contactless_L1_WUPA();

            if (errno == EOT_SHOUT)
            {
                return EOT_SHOUT;
            }

            if (errno  != EMV_ERR_TIME_OUT)
            {
                Poll_Type.Type_A = 1;

                YC_EMV_Contactless_L1_HLTA();
            }

            if (!Poll_Type.Type_B)
            {
                Tp();
                errno  = YC_EMV_Contactless_L1_WUPB(NULL);
                if (errno == EOT_SHOUT)
                {
                    return EOT_SHOUT;
                }
                if (errno != EMV_ERR_TIME_OUT)
                {
                    Poll_Type.Type_B = 1;
                }
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }

        if (!(Poll_Type.Type_A || Poll_Type.Type_B))
        {
            return EMV_ERR_NO_CARD;
        }
    }

    return EMV_ERR_NONE;
}

/**
  * @brief  Type A Collision Detection
  *                 Requirements 9.4: Type A Collision Detection
  * @param  None
  * @retval Error code
  */
unsigned char EMV_A_Collision_Detect(void)
{
    uint8_t i = 0;
    uint8_t cl = 0;
    uint8_t UID_complete = 0;
    uint8_t ISOIEC14443_4_compliant = 0;
    uint8_t retr = 0;
    uint8_t errno = EMV_ERR_NONE;

    Tp();

    while (1)
    {
        errno = YC_EMV_Contactless_L1_WUPA();
        if (errno == EOT_SHOUT)
        {
            return EOT_SHOUT;
        }
        if (errno == EMV_ERR_NONE)
        {
            break;
        }

        AntiColExceptionProcess();
    }

    do
    {
        ++i;

        if (i == 1)
        {
            cl = CL1;
        }
        else if (i == 2)
        {
            cl = CL2;
        }
        else
        {
            cl = CL3;
        }

        retr = 0;
        while (1)
        {

            errno = YC_EMV_Contactless_L1_ANTICOLLISION(cl);
            if (errno == EOT_SHOUT)
            {
                return EOT_SHOUT;
            }
            if (errno == EMV_ERR_NONE)
            {
                break;
            }

            AntiColExceptionProcess();
        }

        retr = 0;
        while (1)
        {

            errno = YC_EMV_Contactless_L1_SELECT(cl, &UID_complete, &ISOIEC14443_4_compliant);
            if (errno == EOT_SHOUT)
            {
                return EOT_SHOUT;
            }
            if (errno == EMV_ERR_NONE)
            {
                break;
            }

            GeneralExceptionProcess();
        }

        if (!UID_complete)
        {
            if (!ISOIEC14443_4_compliant)
            {
                return EMV_ERR_PROTOCOL;
            }

            break;
        }

        if (i == 3)
        {
            return EMV_ERR_PROTOCOL;
        }

    }
    while (i < 3);

    return errno;
}
/**
  * @brief  Type B Collision Detection
  *                     Requirements 9.5: Type B Collision Detection
  * @param  None
  * @retval Error code
  */
unsigned char EMV_B_Collision_Detect(void)
{
    uint8_t retr = 0;
    uint8_t ISOIEC14443_4_compliant = 0;
    uint8_t errno = EMV_ERR_NONE;

    Tp();

    while (1)
    {

        errno = YC_EMV_Contactless_L1_WUPB(&ISOIEC14443_4_compliant);
        if (errno == EOT_SHOUT)
        {
            return EOT_SHOUT;
        }
        if (errno == EMV_ERR_NONE)
        {
            if (!ISOIEC14443_4_compliant)
            {
                return EMV_ERR_PROTOCOL;
            }

            break;
        }

        AntiColExceptionProcess();
    }

    return errno;
}

/**
  * @brief  General Collision Detection
  *                 Requirements 9.3: Collision Detection
  * @param  None
  * @retval Error code
  */
static uint8_t EMV_Collision_General_Detect(void)
{
    uint8_t errno = EMV_ERR_NONE;

    if (Poll_Type.Type_A && Poll_Type.Type_B)
    {
        return EMV_ERR_COLLISION;
    }
    else if (Poll_Type.Type_A)
    {
        errno = EMV_A_Collision_Detect();
    }
    else if (Poll_Type.Type_B)
    {
        errno = EMV_B_Collision_Detect();
    }

    return errno;
}

/**
  * @brief  PICC activation
  * @param  None
  * @retval error code
  */
static uint8_t EMV_Active(void)
{
    uint8_t retr = 0;
    uint8_t errno = EMV_ERR_NONE;

    errno = EMV_Poll();
    if (errno == EOT_SHOUT)
    {
        return EOT_SHOUT;
    }

    if (errno != EMV_ERR_NONE)
    {
        return EMV_ERR_NO_CARD;
    }

    errno = EMV_Collision_General_Detect();
    if (errno == EOT_SHOUT)
    {
        return EOT_SHOUT;
    }

    if (errno == EMV_ERR_NONE)
    {
        if (Poll_Type.Type_A)
        {
            while (1)
            {

                errno = YC_EMV_Contactless_L1_RATS();
                if (errno == EOT_SHOUT)
                {
                    return EOT_SHOUT;
                }
                if (errno == EMV_ERR_NONE)
                {
                    break;
                }

                GeneralExceptionProcess();
            }
        }
        else if (Poll_Type.Type_B)
        {
            while (1)
            {

                errno = YC_EMV_Contactless_L1_ATTRIB();
                if (errno == EOT_SHOUT)
                {
                    return EOT_SHOUT;
                }
                if (errno == EMV_ERR_NONE)
                {
                    break;
                }

                GeneralExceptionProcess();
            }
        }
    }

    return errno;
}

/**
  * @brief  Remove PICC
  * @param  None
  * @retval error code
  */
static uint8_t EMV_PICC_Remove(void)
{
    if (Poll_Type.Type_A)
    {
        YC_EMV_Contactless_L1_Switch_Card_Type(TYPE_A);

        Tp();

        while (YC_EMV_Contactless_L1_WUPA() != EMV_ERR_TIME_OUT)
        {
removea:
            YC_EMV_Contactless_L1_HLTA();

            Tp();
        }

        Tretransmission();

        if (YC_EMV_Contactless_L1_WUPA() != EMV_ERR_TIME_OUT)
        {
            goto removea;
        }

        Tretransmission();

        if (YC_EMV_Contactless_L1_WUPA() != EMV_ERR_TIME_OUT)
        {
            goto removea;
        }

        return EMV_ERR_TIME_OUT;
    }
    else if (Poll_Type.Type_B)
    {
        YC_EMV_Contactless_L1_Switch_Card_Type(TYPE_B);

        Tp();

        while (YC_EMV_Contactless_L1_WUPB(NULL) != EMV_ERR_TIME_OUT)
        {
removeb:
            Tp();
        }

        Tretransmission();

        if (YC_EMV_Contactless_L1_WUPB(NULL) != EMV_ERR_TIME_OUT)
        {
            goto removeb;
        }

        Tretransmission();

        if (YC_EMV_Contactless_L1_WUPB(NULL) != EMV_ERR_TIME_OUT)
        {
            goto removeb;
        }

        return EMV_ERR_TIME_OUT;
    }
    else
    {
        return EMV_ERR_NO_CARD;
    }
}



static uint8_t EMV_LoopBack(void)
{
    uint16_t i = 0;
    uint8_t errno = EMV_ERR_NONE;
    uint8_t res[FSD] = {0};
    uint16_t reslen = 0;

    const uint8_t APDU_PPSE[] = {0x00, 0xA4, 0x04, 0x00, 0x0E,
                                 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31,
                                 0x00
                                };

#ifdef SDK_DEBUG
    PRINT("C-APDU ->: ");
    for (i = 0; i < sizeof(APDU_PPSE); i++)
    {
        PRINT("%02x ", APDU_PPSE[i]);
    }
    PRINT("\n");
#endif
    errno = YC_EMV_Contactless_L1_APDU_Transceive(APDU_PPSE, sizeof(APDU_PPSE), res, &reslen);


    if (errno != EMV_ERR_NONE)
    {
        return errno;
    }


#ifdef SDK_DEBUG
    PRINT("R-APDU <-: ");
    for (i = 0; i < reslen; i++)
    {
        PRINT("%02x ", res[i]);
    }
    PRINT("\n");
#endif

    while (1)
    {
        if (res[1] == 0x70)
        {
            return EOT_IND_REMOVE;
        }

        if (res[1] == 0x72)
        {
            return EOT_IND_POWER_OFF;
        }


#ifdef SDK_DEBUG
        PRINT("C-APDU ->: ");
        for (i = 0; i < reslen - 2; i++)
        {
            PRINT("%02x ", res[i]);
        }
        PRINT("\n");
#endif

        errno = YC_EMV_Contactless_L1_APDU_Transceive(res, reslen - 2, res, &reslen);

        if (errno != EMV_ERR_NONE)
        {
            return errno;
        }


#ifdef SDK_DEBUG
        PRINT("R-APDU <-: ");
        for (i = 0; i < reslen; i++)
        {
            PRINT("%02x ", res[i]);
        }
        PRINT("\n");
#endif
    }
}
static void EMV_Field_Reset(void)
{
    YC_EMV_Contactless_L1_Rf_Switch(OFF);
    Tp();
    YC_EMV_Contactless_L1_Rf_Switch(ON);
}

static void EMV_Field_Power_Off(void)
{
    YC_EMV_Contactless_L1_Rf_Switch(OFF);
    Tpoweroff();
    YC_EMV_Contactless_L1_Rf_Switch(ON);
}

/**
  * @brief  Enter low power mode
  * @param  None
  * @retval None
  */
void YC_EMV_Contactless_L1_Sleep(void)
{
    CLEARBITMASK(REG_TX_PA_GAIN_CTRL, 0x20);
    CLEARBITMASK(REG_TX_PA_GAIN_CTRL, 0x10);
    Nfc_SysTick_Delay_Ms(20);
    CLEARBITMASK(REG_TOP_CTRL, BIT0);
    SETBITMASK(REG_TOP_CTRL, BIT2);
}

/**
  * @brief  Exit low power mode
  * @param  None
  * @retval None
  */
void YC_EMV_Contactless_L1_Wake(void)
{
    CLEARBITMASK(REG_TOP_CTRL, BIT2);
    SETBITMASK(REG_TOP_CTRL, BIT0);
}

void set_dc_win(uint8_t win)
{
    uint8_t tmp = 0;
    tmp = READREG(REG_RSSI_CALC_CONFIG);
    WRITEREG(REG_RSSI_CALC_CONFIG, ((tmp & 0xFC) | win));
}

void set_rssi_calc_en(uint8_t en)
{
    uint8_t tmp = 0;
    tmp = READREG(REG_RSSI_CALC_CTRL);
    tmp = tmp & 0xfd;
    tmp = tmp | ((en & 0x01) << 1);
    WRITEREG(REG_RSSI_CALC_CTRL, tmp);
}

void force_dc_calc(void)
{
    uint8_t tmp = 0;
    tmp = READREG(REG_RSSI_CALC_CTRL);
    WRITEREG(REG_RSSI_CALC_CTRL, (tmp & 0xFE));
    WRITEREG(REG_RSSI_CALC_CTRL, (tmp | 0x01));
    Nfc_SysTick_Delay_Ms(1);
    WRITEREG(REG_RSSI_CALC_CTRL, (tmp & 0xFE));
    Nfc_SysTick_Delay_Ms(2);
}
uint8_t dc_cali_i(void)
{
    uint8_t cali = 0, dc = 0, cnt = 0;
    set_cali_i(0x00);
    while (1)
    {
        cnt ++;
        force_dc_calc();
        dc = get_dc_i();
        cali = get_cali_i();
        if (cnt == 0xff)
        {
            return CALI_FAIL;
        }

        if ((dc <= DC_ACCEPT_RANGE) || (dc >= (256 - DC_ACCEPT_RANGE)))
        {
            return CALI_OK;
        }

        if ((dc & 0x80) == 0)
        {
            if ((cali & 0x80) == 0)
            {
                if (cali == 0)
                {
                    cali = 0x80;
                }
                else
                {
                    cali = cali - 1;
                }

            }
            else
            {
                if (cali == 0xff)
                {
                    cali = cali;
                }
                else
                {
                    cali = cali + 1;
                }
            }

        }
        else
        {
            if ((cali & 0x80) == 0)
            {
                if (cali == 0x7f)
                {
                    cali = 0x7f;
                }
                else
                {
                    cali = cali + 1;
                }
            }
            else
            {
                if (cali == 0x80)
                {
                    cali = 0x00;
                }
                else
                {
                    cali = cali - 1;
                }
            }
        }

        set_cali_i(cali);

    }
}
uint8_t dc_cali_q(void)
{
    uint8_t cali = 0, dc = 0, cnt = 0;
    set_cali_q(0x00);
    while (1)
    {
        cnt ++;
        force_dc_calc();
        dc = get_dc_q();

        cali = get_cali_q();

        if (cnt == 0xff)
        {
            return CALI_FAIL;
        }

        if ((dc <= DC_ACCEPT_RANGE) || (dc >= (256 - DC_ACCEPT_RANGE)))
        {
            return CALI_OK;
        }
        if ((dc & 0x80) == 0)
        {
            if ((cali & 0x80) == 0)
            {
                if (cali == 0)
                {
                    cali = 0x80;
                }
                else
                {
                    cali = cali - 1;
                }
            }
            else
            {
                if (cali == 0xff)
                {
                    cali = cali;
                }
                else
                {
                    cali = cali + 1;
                }
            }
        }
        else
        {
            if ((cali & 0x80) == 0)
            {
                if (cali == 0x7f)
                {
                    cali = 0x7f;
                }
                else
                {
                    cali = cali + 1;
                }
            }
            else
            {
                if (cali == 0x80)
                {
                    cali = 0x00;
                }
                else
                {
                    cali = cali - 1;
                }
            }
        }
        set_cali_q(cali);

    }

}

void dc_cali_alg(uint8_t win_size)
{
    WRITEREG(REG_DA_RX_FLT_GRCK_EN, 0x00);
    WRITEREG(REG_DA_TX_PA_EN, 0x00);
    set_dc_win(win_size);

    WRITEREG(REG_TOP_CTRL, 0x11);
    set_rssi_calc_en(1);

    WRITEREG(REG_ADC_CTRL, 0x08);

    WRITEREG(REG_RX_CTRL3, RX_CTRL3);
    WRITEREG(REG_RX_CTRL4, RX_CTRL4);
    WRITEREG(REG_RX_CTRL5, RX_CTRL5);
    Set_Receive_Parameters(RX_GAIN_6DB, RX_I_R_1K);

    if (CALI_OK == dc_cali_i())
    {
        gain_i_q_table[0][1] = get_cali_i();
    }
    else
    {
        gain_i_q_table[0][1] = 0;
    }
    if (CALI_OK == dc_cali_q())
    {
        gain_i_q_table[0][2] = get_cali_q();
    }
    else
    {
        gain_i_q_table[0][2] = 0;
    }
    Set_Receive_Parameters(RX_GAIN_18DB, RX_I_R_500);

    if (CALI_OK == dc_cali_i())
    {
        gain_i_q_table[2][1] = get_cali_i();
    }
    else
    {
        gain_i_q_table[2][1] = 0;
    }
    if (CALI_OK == dc_cali_q())
    {
        gain_i_q_table[2][2] = get_cali_q();
    }
    else
    {
        gain_i_q_table[2][2] = 0;
    }
}

/************************************Parameter Adjustment Function**************************************/
void Set_Carrier_Strength(unsigned char PaGain)
{
    WRITEREG(REG_TX_PA_GAIN, PaGain);
}
void Set_Receive_Parameters(unsigned char RxGain, unsigned char RfAtt)
{
    uint8_t val = 0;
    uint8_t i = 0;

    val = RxGain | RfAtt;
    WRITEREG(REG_RX_CTRL0, val);
    switch (RxGain)
    {
    case    RX_GAIN_6DB:
        i = 0;
        break;
    case    RX_GAIN_12DB:
        i = 1;
        break;
    case    RX_GAIN_18DB:
        i = 2;
        break;
    case    RX_GAIN_24DB:
        i = 3;
        break;
    case    RX_GAIN_30DB:
        i = 4;
        break;
    case    RX_GAIN_36DB:
        i = 5;
        break;
    case    RX_GAIN_42DB:
        i = 6;
        break;
    case    RX_GAIN_48DB:
        i = 7;
        break;
    case    RX_GAIN_54DB:
        i = 8;
        break;
    default:
        i = 0;
        break;
    }
    WRITEREG(REG_RX_CTRL1, gain_i_q_table[i][1]);
    WRITEREG(REG_RX_CTRL2, gain_i_q_table[i][2]);
}

void Set_Min_Level(unsigned short MinLevel)
{
    WRITEREG(REG_MIN_LEVEL_LOW, MinLevel & 0xFF);
    WRITEREG(REG_MIN_LEVEL_HIGH, MinLevel >> 8);
}
void YC_EMV_Contactless_L1_InitRSSI(void)
{
    SETBITMASK(REG_TOP_CTRL, 0x10);
    WRITEREG(REG_RX_CTRL3, 0x3D);
    SETBITMASK(REG_RX_CTRL4, 0x01);
    WRITEREG(REG_RX_CTRL5, 0x7C);
    SETBITMASK(REG_RSSI_CALC_CONFIG, 0x04);
    SETBITMASK(REG_RSSI_CALC_CTRL, 0x02);

}


uint8_t RSSI_maxi = 0, RSSI_maxq = 0;
uint8_t Aset_read_power_index(void)
{
    uint8_t tmp = 0;
    WRITEREG(REG_RSSI_CALC_CTRL, 0x02);
    WRITEREG(REG_RSSI_CALC_CTRL, 0x03);
    Nfc_SysTick_Delay_Us(100);
    tmp = READREG(REG_AVG_POW);
    RSSI_maxi = READREG(REG_AVG_I);
    RSSI_maxq = READREG(REG_AVG_Q);
    WRITEREG(REG_RSSI_CALC_CTRL, 0x00);


    return tmp;


}
signed char positive_numi = 0, positive_numq = 0 ;
signed char negative_numi = 0, negative_numq = 0 ;
unsigned char  Aset_read_power_xRSSI(void)
{

    unsigned char tmp = 0, tmp_avgi = 0, tmp_avgq = 0;
    set_dc_win(WIN_SIZE_128);

    WRITEREG(REG_TX_PA_GAIN, PA_GAIN_VAL);

    YC_EMV_Contactless_L1_Rf_Switch(ON);
    Set_Receive_Parameters(RX_GAIN_6DB, RX_I_R_1K);       //设置接收增益及内部电阻


    WRITEREG(REG_ADC_CTRL, 0x08);
    WRITEREG(REG_XTAL_CTRL4, 0x07);

    WRITEREG(REG_RSSI_CALC_CTRL, 0x02);
    WRITEREG(REG_RSSI_CALC_CTRL, 0x03);
    Nfc_SysTick_Delay_Us(100);

    tmp = READREG(REG_AVG_POW);
    tmp_avgi = READREG(REG_AVG_I);
    tmp_avgq = READREG(REG_AVG_Q);

    if ((tmp_avgi & 0x80) | (tmp_avgq & 0x80))
    {
        negative_numi = tmp_avgi;
        negative_numq = tmp_avgq;

        return 11;

    }
    else
    {
        positive_numi = tmp_avgi;
        positive_numq = tmp_avgq;
        WRITEREG(REG_ADC_CTRL, 0x00);
        WRITEREG(REG_XTAL_CTRL4, 0x06);
        WRITEREG(REG_RSSI_CALC_CTRL, 0x02);

        return 0;

    }
}

unsigned char  Aset_read_power_xRSSI_RXVPP(void)
{
    unsigned char i = 0, j = 0, time = 0;


    YC_EMV_Contactless_L1_InitRSSI();
    YC_EMV_Contactless_L1_Rf_Switch(ON);

    Nfc_SysTick_Delay_Ms(30);
    while (0x0b == Aset_read_power_xRSSI())
    {
        time++;
        YC_EMV_Contactless_L1_Rf_Switch(OFF);
        Nfc_SysTick_Delay_Ms(30);
        YC_EMV_Contactless_L1_Rf_Switch(ON);
        if (time > 0x0f)
        {
            break;
        }
    }
    set_dc_win(WIN_SIZE_256);

    WRITEREG(REG_TX_PA_GAIN, 0);

    YC_EMV_Contactless_L1_Rf_Switch(ON);
    Set_Receive_Parameters(RX_GAIN_6DB, RX_I_R_1K);       //设置接收增益及内部电阻

    WRITEREG(REG_ADC_CTRL, 0x08);
    WRITEREG(REG_XTAL_CTRL4, 0x07);
    Nfc_SysTick_Delay_Us(100);


    for (j = 0; j <= PA_GAIN_VAL; j++)
    {
        WRITEREG(REG_TX_PA_GAIN, j);
        i = Aset_read_power_index();
        PRINT("pa = 0x%02x, rssi = 0x%02x \r\n", j, i);

    }
    WRITEREG(REG_ADC_CTRL, 0x00);
    WRITEREG(REG_XTAL_CTRL4, 0x06);
    WRITEREG(REG_RSSI_CALC_CTRL, 0x02);

    return i;
}

