/*
File Name    : yc_iso7816.c
Author       : Yichip
Version      : V2.0
Date         : 2020/7/28
Description  : iso7816 encapsulation.
*/

#include"string.h"

#include "yc_7816.h"
#include "system.h"
#include "yc_timer.h"
#include "yc_exti.h"
#include "yc_7816_T1.h"
#include "yc_7816_T0.h"

unsigned int terminal_send_cwt = 12;
unsigned int terminal_rec_cwt = 10086;
unsigned int terminal_rec_bwt = 15380;
unsigned int terminal_rec_bgt = 26;

unsigned int terminal_rec_bwt_atr = 0;
unsigned int terminal_change_rec_bwt_num = 0;


unsigned char atr_cwt_valid_flag = 0;	//接收复位信息时，如果未收到第一个字节之前cwt标志置一，此标志此时无效

volatile unsigned int time0_cnt_last = 0;
volatile unsigned int time0_cnt_new = 0;

volatile unsigned char contact_atr_reset_l_flag = 0;			//复位IO保持低电平到时间标志
volatile unsigned char contact_atr_rfb_overtime_falg = 0;		//ICC 回应第一个字节超时标志
volatile unsigned char contact_atr_rab_overtime_falg = 0;		//ICC 回应所有字节超时标志


uint16_t fi_table[16] = {372, 372, 558, 744, 1116, 1488, 1860, 0,
                         0, 512, 768, 1024, 1536, 2048, 0, 0
                        };
uint16_t di_table[16] = {0, 1, 2, 4, 8, 16, 32, 64,
                         12, 20, 0, 0, 0, 0, 0, 0
                        };

uint32_t period_7816 = 0;
uint32_t contact_reset_l_cnt = 0;
uint32_t contact_atr_rfb_time_cnt = 0;
uint32_t contact_atr_rab_time_cnt = 0;
uint32_t contact_over_time_cnt = 0;

//跟T1有关的参数
st_T1_Parameters g_T1;
//跟7816协议参数有关的参数
st_7816_Para g_7816Para;
//sci接口引脚定义
SCI_PIN_Def sci_pin;

uint8_t get_io_num(uint8_t gpio_group, uint16_t pin)
{
    uint16_t val;
    uint8_t num = 0, i;

    val = pin;
    for (i = 0; i < 16; i++)
    {
        if ((val >> i) & 1)
        {
            num = i;
            break;
        }
    }

    val = gpio_group * 16 + num;
    return val;
}

uint8_t detect_io_state(void)
{
    return GPIO_ReadInputDataBit((GPIO_TypeDef)sci_pin.detect.gpio_group, sci_pin.detect.gpio_pin);
}

uint8_t crad_in_io_filter(void)
{
    uint8_t ret[2];

    ret[0] = detect_io_state();
    delay_ms(2);
    ret[1] = detect_io_state();
    if (ret[0] == ret[1])
    {
        return ret[0];
    }

    return 2;
}

//state 0-vcard off  1-3.3 vcard   2-1.8 vcard
void vcard_control(uint8_t state)
{
    if (state == 1)
    {
        SYSCTRL_HWCTRL(0) |= 0x1f;
    }
    else if (state == 2)
    {
        SYSCTRL_HWCTRL(0) &= 0xef;
        SYSCTRL_HWCTRL(0) |= 0x8;
    }
    else
    {
        SYSCTRL_HWCTRL(0) &= 0xf7;
    }
}

void iso7816_deactive(void)
{
    GPIO_CONFIG(get_io_num(sci_pin.reset.gpio_group, sci_pin.reset.gpio_pin)) = GPCFG_OUTPUT_LOW;//RST
    delay_ms(1);

    GPIO_CONFIG(get_io_num(sci_pin.clk.gpio_group, sci_pin.clk.gpio_pin)) = GPCFG_OUTPUT_LOW;//clk
    TIM_Cmd(ISO7816_TIMx, DISABLE);

    GPIO_CONFIG(get_io_num(sci_pin.io.gpio_group, sci_pin.io.gpio_pin)) = GPCFG_OUTPUT_LOW;//io

    vcard_control(ISO7816_VCC_0V);
#ifdef DEBUG_7816
    MyPrintf("power down\n");
#endif
}

void timer0_enable(unsigned long num)
{
    TIM_InitTypeDef TIM_struct;

    enable_clock(CLKCLS_TIM);

    TIM_struct.period = num;
    TIM_struct.TIMx = (TIM_NumTypeDef)0;
    TIM_Init(&TIM_struct);
    TIM_Cmd((TIM_NumTypeDef)0, ENABLE);
}

void atr_time_start(void)
{
    timer0_enable(0xffffffff);

    contact_over_time_cnt = CONTACT_OVER_TIME_VAL*CPU_MHZ;
}

uint16_t ISO7816_RecAtrBytes(uint8_t *RxBuf, uint16_t RxLen)
{
    uint16_t  RemainLen = 0;

    if ((!RxLen) || (NULL == RxBuf))
    {
        return ISO7816_PARA_ERROR;
    }

    do
    {
        if (detect_io_state() == sci_pin.detect_state)
        {
            return ISO7816_CARD_ABSENT;
        }

        if ((SCI7816_STAT & 0x80) && (atr_cwt_valid_flag == 1))				//CWT
        {
#ifdef DEBUG_7816
            MyPrintf("CWT error\n");
#endif
            return ISO7816_REC_TIMEOUT;
        }

        if (contact_atr_rfb_overtime_falg == 0)
        {
            time0_cnt_new = TIM0_CNT;
            time0_cnt_new = time0_cnt_last - time0_cnt_new;
            if (time0_cnt_new >= CONTACT_ATR_RFB_TIME_CNT) //icc not response atr
            {
#ifdef DEBUG_7816
                MyPrintf("receive atr firstbyte overtime %d\n", CONTACT_ATR_RFB_TIME_CNT);
#endif
                return ISO7816_REC_TIMEOUT;
            }
        }

        if (contact_atr_rab_overtime_falg == 0)
        {
            time0_cnt_new = TIM0_CNT;
            time0_cnt_new = time0_cnt_last - time0_cnt_new;
            if (time0_cnt_new >= CONTACT_ATR_RAB_TIME_CNT) //icc response atr overtime
            {
#ifdef DEBUG_7816
                MyPrintf("receive atr allbyte overtime\n");
#endif
                return ISO7816_REC_TIMEOUT;
            }
        }

        if (SCI7816_STAT & 1)
        {
            *(RxBuf + RemainLen) = SCI7816_DATA;

            if (SCI7816_STAT & 0x100)	//parity error
            {
#ifdef DEBUG_7816
                MyPrintf("PARITY error\n");
#endif
                return ISO7816_PARITY_ERROR;
            }

#ifdef DEBUG_7816
            MyPrintf("%02X ", *(RxBuf + RemainLen));
#endif

            RemainLen++;
        }
    }
    while (RemainLen < RxLen);

    return OK;
}

uint8_t ISO7816_cold_reset(uint8_t state)
{
    PWM_InitTypeDef PWM_struct;

    uint16_t fi_table_1[16] = {372, 372, 558, 744, 1116, 1488, 1860, 0,
                               0, 512, 768, 1024, 1536, 2048, 0, 0
                              };
    uint16_t di_table_1[16] = {0, 1, 2, 4, 8, 16, 32, 64,
                               12, 20, 0, 0, 0, 0, 0, 0
                              };

#ifdef DEBUG_7816
    MyPrintf("sci7816 cold reset\n");
#endif

    memcpy(fi_table, fi_table_1, 16);
    memcpy(di_table, di_table_1, 16);

    terminal_send_cwt = 20;
    terminal_rec_cwt = 10086;

    terminal_rec_bwt = 15380;
    terminal_rec_bgt = 100;

    g_7816Para.bTA1 = 0xff;
    g_7816Para.bTA2 = 0xFF;
    g_7816Para.bTA2Flag = 0;

    g_7816Para.bFlag = 0;

    contact_atr_reset_l_flag = 0;
    contact_atr_rfb_overtime_falg = 0;
    contact_atr_rab_overtime_falg = 0;

    atr_cwt_valid_flag = 0;

    vcard_control(ISO7816_VCC_0V);
    GPIO_CONFIG(get_io_num(sci_pin.clk.gpio_group, sci_pin.clk.gpio_pin)) = GPCFG_OUTPUT_LOW;//clk
    GPIO_CONFIG(get_io_num(sci_pin.reset.gpio_group, sci_pin.reset.gpio_pin)) = GPCFG_OUTPUT_LOW;//RST
    GPIO_CONFIG(get_io_num(sci_pin.io.gpio_group, sci_pin.io.gpio_pin)) = GPCFG_OUTPUT_LOW;	//io

    //enable power
    vcard_control(state);

    PWM_struct.HighLevelPeriod = period_7816;
    PWM_struct.LowLevelPeriod = period_7816;
    PWM_struct.SatrtLevel = OutputHigh;
    PWM_struct.TIMx = ISO7816_TIMx;
    TIM_PWMInit(&PWM_struct);
    TIM_Cmd(ISO7816_TIMx, ENABLE);
    GPIO_CONFIG(get_io_num(sci_pin.clk.gpio_group, sci_pin.clk.gpio_pin)) = GPCFG_PWM_OUT0 + ISO7816_TIMx;

    //enable io
    GPIO_CONFIG(get_io_num(sci_pin.io.gpio_group, sci_pin.io.gpio_pin)) = GPCFG_SCI7816_IO | GPCFG_PU;
    uint8_t TempChar;
    while (SCI7816_STAT & 1)
    {
        TempChar = SCI7816_DATA;
#ifdef DEBUG_7816
        MyPrintf("%02x", TempChar);
        MyPrintf("\n");
#endif

        if (detect_io_state() == sci_pin.detect_state)
        {
            return ISO7816_CARD_ABSENT;
        }
    }

    //clk
    //delay 40000-45000clk  (250ns*42000clk = 10.5ms)
    time0_cnt_last = TIM0_CNT;
    time0_cnt_new = 0;
    while (time0_cnt_new <= CONTACT_RESET_L_CNT)
    {
        time0_cnt_new = TIM0_CNT;
        time0_cnt_new = time0_cnt_last - time0_cnt_new;
    }

    //config sci
    SCI7816_CTRL |= 0x04;	//check atr first byte
    SCI7816_CTRL |= 0x03;	//clear tx/rx fifo
    SCI7816_STAT |= 0X124;	//clear tx/rx retry error

    //config sci7816 tmode bit=1 实现ATR发送字节校验位错误时只要判断出检验错误即可，不需要将IO拉低
    SCI7816_MODE = (4 | 0 << SCICFG_ETU_SEL | 1 << SCICFG_MASTER | 3 << SCICFG_MCLK_SEL\
                    | 1 << SCICFG_BGTEN | 1 << SCICFG_CWTEN | 1 << SCICFG_EDCEN | 1 << SCICFG_IO_EN) | 1 << SCICFG_TMODE;

    SCI7816_ETU = 372;
    SCI7816_BGT = terminal_rec_bgt;
    SCI7816_CWT = terminal_rec_cwt;

    //rst disable
    GPIO_CONFIG(get_io_num(sci_pin.reset.gpio_group, sci_pin.reset.gpio_pin)) = GPCFG_OUTPUT_HIGH;//RST

    //icc response atr firstbyte max time 42020 clk
    time0_cnt_last = TIM0_CNT;
    contact_atr_rfb_overtime_falg = 0;
    contact_atr_rab_overtime_falg = 1;
    return  OK;
}

void ISO7816_warm_reset(void)
{
#ifdef DEBUG_7816
    MyPrintf("sci7816 warm reset\n");
#endif

    g_7816Para.bTA1 = 0xff;
    g_7816Para.bTA2 = 0xFF;
    g_7816Para.bTA2Flag = 0;

    atr_cwt_valid_flag = 0;

    SCI7816_CTRL |= 0x04;	//check atr first byte
    SCI7816_CTRL |= 0x03;	//clear tx/rx fifo
    SCI7816_STAT |= 0X124;	//clear tx/rx retry error

    //config sci7816 tmode bit=1 实现ATR发送字节校验位错误时只要判断出检验错误即可，不需要将IO拉低
    SCI7816_MODE = (4 | 0 << SCICFG_ETU_SEL | 1 << SCICFG_MASTER | 3 << SCICFG_MCLK_SEL\
                    | 1 << SCICFG_BGTEN | 1 << SCICFG_CWTEN | 1 << SCICFG_EDCEN | 1 << SCICFG_IO_EN) | 1 << SCICFG_TMODE;

    GPIO_CONFIG(get_io_num(sci_pin.reset.gpio_group, sci_pin.reset.gpio_pin)) = GPCFG_OUTPUT_LOW;//RST

    //delay 40000-45000clk  (250ns*42000clk = 10.5ms)
    time0_cnt_last = TIM0_CNT;
    time0_cnt_new = 0;
    while (time0_cnt_new <= CONTACT_RESET_L_CNT)
    {
        time0_cnt_new = TIM0_CNT;
        time0_cnt_new = time0_cnt_last - time0_cnt_new;
    }

    GPIO_CONFIG(get_io_num(sci_pin.reset.gpio_group, sci_pin.reset.gpio_pin)) = GPCFG_OUTPUT_HIGH;//RST

    //icc response atr firstbyte max time 42020 clk
    time0_cnt_last = TIM0_CNT;
    contact_atr_rfb_overtime_falg = 0;
    contact_atr_rab_overtime_falg = 1;
}

void ISO7816_config_mode_fun(void)
{
    if (g_7816Para.bProlType == PROTOCOL_T0)
    {
        SCI7816_MODE |= (1 << SCICFG_RETRY_EN) | (4 << SCICFG_RETRY);
        SCI7816_MODE &= ~(1 << SCICFG_TMODE);
    }
    else
    {
        SCI7816_MODE |= 1 << SCICFG_TMODE;
    }
}

uint16_t ISO7816_GetAtr(uint8_t *pAtr, uint8_t *pLength)
{
    uint8_t i;
    uint8_t Len, Y, K, Tck;
    uint16_t ret;
    uint8_t TDn = 0;
    uint8_t TD2Flag = 0;

    if ((NULL == pAtr) || (NULL == pLength))
    {
        return ISO7816_PARA_ERROR;
    }

    ret = ISO7816_RecAtrBytes(pAtr, 1);  //Ts
    if (OK != ret)
    {
        return ret;
    }

    SCI7816_CTRL &= 0XFB;

    if ((pAtr[0] != 0x3b) && (pAtr[0] != 0x3f))
    {
        if (pAtr[0] == 0x23)
        {
            SCI7816_MODE &= ~(1 << SCICFG_BIT_ORDER);		//正向卡片
        }
        else if (pAtr[0] == 0x03)
        {
            SCI7816_MODE |= (1 << SCICFG_BIT_ORDER);		//反向卡片
        }
        else
        {
            return ISO7816_ATR_ERROR;
        }
    }

    contact_atr_rfb_overtime_falg = 1;

    time0_cnt_last = TIM0_CNT;
    contact_atr_rab_overtime_falg = 0;

    atr_cwt_valid_flag = 1;

    ret = ISO7816_RecAtrBytes(pAtr + 1, 1); //T0
    COMPARE_RETURN(ret);

    Y = pAtr[1] & 0xF0;
    K = pAtr[1] & 0x0F;
    Len = 2;
    g_7816Para.bProlType = 0xff;
    while (Y)
    {
        TDn++;
        if (Y & BIT4)
        {
            ret = ISO7816_RecAtrBytes(&pAtr[Len++], 1);
            COMPARE_RETURN(ret);
        }
        if (Y & BIT5)
        {
            ret = ISO7816_RecAtrBytes(&pAtr[Len++], 1);
            COMPARE_RETURN(ret);
        }
        if (Y & BIT6)
        {
            ret = ISO7816_RecAtrBytes(&pAtr[Len++], 1);
            COMPARE_RETURN(ret);
        }
        if (Y & BIT7)
        {
            ret = ISO7816_RecAtrBytes(&pAtr[Len], 1);
            COMPARE_RETURN(ret);
            Y = pAtr[Len] & 0xF0;
            if (g_7816Para.bProlType == 0xff)
            {
                g_7816Para.bProlType = pAtr[Len] & 0x0F;
                if (1 < g_7816Para.bProlType)
                {
                    //如果协议大于1，默认为0
                    g_7816Para.bProlType = 0;
                }
            }
            //TD2存在，并且值为1 或 E
            if (TDn == 2) /*&& (((pAtr[Len] & 0x0F) == 1) || ((pAtr[Len] & 0x0F) == 14))*/
            {
                TD2Flag = 1;
            }

            Len++;
        }
        else
        {
            Y = 0;
        }
    }

    //如果协议没有修改，默认为0
    if (g_7816Para.bProlType == 0xff)
    {
        g_7816Para.bProlType = 0;
    }

    if (K)
    {
        ret = ISO7816_RecAtrBytes(&pAtr[Len], K);
        COMPARE_RETURN(ret);
        Len += K;
    }
    if (g_7816Para.bProlType || (TD2Flag == 1))
    {
        ret = ISO7816_RecAtrBytes(&pAtr[Len], 1);
        COMPARE_RETURN(ret);
        Len++;
    }
    if ((ATR_SIZE_MAX - 1) < Len)
    {
        return ISO7816_ATR_LEN_ERROR;
    }
    if (g_7816Para.bProlType)
    {
        for (i = 1, Tck = 0; i < Len; i++)
        {
            Tck ^= pAtr[i];
        }
        if (Tck)
        {
            return ISO7816_ATR_TCK_ERROR;
        }
    }

    *pLength = Len;

    //判断Rx Fifo 中是否还有数据，如果有的话，直接Warmrest
    delay_ms(2);
    i = 3;
    ret = 0;
    do
    {
        while (SCI7816_STAT & 1)
        {
            ret = 1;
            ISO7816_RecAtrBytes(&pAtr[Len], 1);

            if (detect_io_state() == sci_pin.detect_state)
            {
                return ISO7816_CARD_ABSENT;
            }
        }
        if (ret == 1)
        {
            return ISO7816_ATR_LEN_ERROR;
        }

    }
    while (i--);

    return OK;
}

uint16_t ISO7816_DisposeATR(uint8_t *pATR, uint8_t ATRLen)
{
    uint8_t tmpLen;
    uint8_t tmpTDx;
    uint8_t offset;
    uint32_t tmpTimer;
    uint8_t TC1_N = 0;
    uint8_t TD1_N = 0, TD1_Flag = 0;
    uint8_t tmpCWI, tmpBWI;

    //IFSC 默认值32
    g_T1.bIFSC = 32;
    g_7816Para.bFlag &= (~BIT0);
    if ((pATR == NULL) || (ATRLen < 3) || (ATRLen > ATR_SIZE_MAX))
    {
        return ISO7816_PARA_ERROR;
    }

    tmpTDx = pATR[1] & 0xF0;

    offset = 2;
    //TA1--TD1
    //TA1
    if ((tmpTDx & BIT4) != 0)
    {
        offset++;
        g_7816Para.bFlag |= BIT0;
        g_7816Para.bTA1 = pATR[2];

        if ((g_7816Para.bTA1 != 0x11) && (g_7816Para.bTA1 != 0x12) && (g_7816Para.bTA1 != 0x13))
        {
            //不发PPS
            //return ISO7816_ATR_TA1_ERROR;
        }
    }
    //TB1
    if ((tmpTDx & BIT5) != 0)
    {
        //如果是冷复位，必须要有TB1,并且值为0
        if (((g_7816Para.bFlag & BIT1) == 0) && (pATR[offset]) != 0)
        {
            return ISO7816_ATR_TB1_ERROR;
        }
        offset++;
    }
    else
    {
        //如果是冷复位，必须要有TB1
        if ((g_7816Para.bFlag & BIT1) == 0)
        {
            return ISO7816_ATR_TB1_ERROR;
        }
    }

    //TC1
    if ((tmpTDx & BIT6) != 0)
    {
        if (pATR[offset] == 0xFF)
        {
            if (g_7816Para.bProlType == 1)
            {
                terminal_send_cwt = 11;
            }
            else//T0
            {
                terminal_send_cwt = 12;
            }
        }
        else
        {
            terminal_send_cwt = pATR[offset] + 12;
        }
        //保存N值，后面会用到
        TC1_N = pATR[offset];
        //TC1_Flag = 1;
        offset++;
    }
    else
    {
        if (g_7816Para.bProlType == 1)
        {
            terminal_send_cwt = 11;
        }
        else//T0
        {
            terminal_send_cwt = 12;
        }
    }


    //TD1
    if ((tmpTDx & BIT7) != 0)
    {
        //只支持T=0  T=1
        TD1_N = pATR[offset];
        if ((TD1_N & 0x0F) == 1)
        {
            TD1_Flag = 1;
        }
        g_7816Para.bProlType = pATR[offset] & 0x0f;
        tmpTDx = pATR[offset];
        offset++;
    }
    else
    {
        tmpTDx = 0;
        g_7816Para.bTA1 = 0x11;
    }

    //TA2-TD2
    if (tmpTDx != 0)
    {
        //TA2
        if ((tmpTDx & BIT4) != 0)
        {
            g_7816Para.bTA2Flag = 1;
            g_7816Para.bTA2 = pATR[offset];
            if ((pATR[offset] & 0x10) != 0)
            {
                return ISO7816_ATR_TA2_ERROR;
            }
            offset++;

            tmpTimer = 10086;
            if ((g_7816Para.bFlag & BIT0) && (g_7816Para.bTA1 != 0x11))
            {
                if ((g_7816Para.bTA1 & 0xF) > 1)
                {
                    tmpTimer *= di_table[g_7816Para.bTA1 & 0xF];  // * DI

                    if ((g_7816Para.bTA1 & 0x0f) == 3)
                    {
                        tmpTimer += 20;
                    }
                }

                //多加2个ETU
                tmpTimer += 20;
                //如果没有超限，设置WT时间
                terminal_rec_cwt = tmpTimer;
            }
        }
        else
        {
            g_7816Para.bTA1 = 0x11;
        }
        //TB2
        if ((tmpTDx & BIT5) != 0)
        {
            return ISO7816_ATR_TB2_ERROR;
        }

        //TC2
        if ((tmpTDx & BIT6) != 0)
        {
            if (g_7816Para.bProlType == PROTOCOL_T0)
            {
                //TC2只支持10
                if (10 != pATR[offset])
                {
                    return ISO7816_ATR_TC2_ERROR;
                }
                else
                {
                    tmpTimer = 960 *  pATR[offset] + 480;   //默认9600 + 480
                    if (g_7816Para.bFlag & BIT0)
                    {
                        tmpTimer *= di_table[g_7816Para.bTA1 & 0xF];  // * DI

                    }

                    terminal_rec_cwt = tmpTimer;

                }
            }
            offset++;
        }
        //TD2
        if ((tmpTDx & BIT7) != 0)
        {
            //TD1指明了T=1协议，TD2不支持 T=1的话报错。
            if (TD1_Flag)
            {
                if ((pATR[offset] & 0x0f) != 1)
                {
                    return ISO7816_ATR_TD2_ERROR;
                }
            }
            //支持T=1
            else if (((pATR[offset] & 0x0f) != 1) && ((pATR[offset] & 0x0f) != 14))
            {
                return ISO7816_ATR_ERROR;
            }
            tmpTDx = pATR[offset];
            offset++;
        }
        else
        {
            tmpTDx = 0;
        }
    }

    //TA3-TD3
    if (tmpTDx != 0)
    {
        //TA3
        if ((tmpTDx & BIT4) != 0)
        {
            if ((pATR[offset] < 16) || (pATR[offset] == 0xff))
            {
                return ISO7816_ATR_TA3_ERROR;
            }
            //设置卡片最大数据长度
            g_T1.bIFSC = pATR[offset];
            offset++;
        }
        //TB3
        if ((tmpTDx & BIT5) != 0)
        {
            if (g_7816Para.bProlType != 0)
            {
                tmpCWI = pATR[offset] & 0x0f;
                tmpBWI = (pATR[offset] & 0xf0) >> 4;
                if ((tmpCWI > 5) || (tmpBWI > 4))
                {
                    return ISO7816_ATR_TB3_ERROR;
                }
                if ((TC1_N > 0x1e) && (TC1_N != 0xff))
                {
                    return ISO7816_ATR_TB3_ERROR;
                }
                //2的CWI次幂
                tmpLen = 1;
                while (tmpCWI > 0)
                {
                    tmpLen = tmpLen * 2;
                    tmpCWI--;
                }
                if ((tmpLen <= (TC1_N + 1)) && (TC1_N != 0xff))
                {
                    return ISO7816_ATR_TB3_ERROR;
                }

                //设置CWI  BWI
                tmpLen = 0;
                if ((g_7816Para.bFlag & BIT0) != 0)
                {
                    tmpLen = di_table[g_7816Para.bTA1 & 0x0f];
                }
                else
                {
                    tmpLen = 1;
                }

                terminal_rec_cwt = (1 << (pATR[offset] & 0x0F)) + 4;
                terminal_rec_bwt = (1 << ((pATR[offset] & 0xF0) >> 4));
                if (terminal_rec_bwt == 0)
                {
                    terminal_rec_bwt = 1;
                }
                terminal_rec_bwt = terminal_rec_bwt * 960 * tmpLen + 11 + 960 * tmpLen + 20;

                terminal_rec_bwt_atr = terminal_rec_bwt;

#ifdef DEBUG_7816
                MyPrintf("para_1\n");
                MyPrintf("%02x", terminal_send_cwt);
                MyPrintf("\n");

                MyPrintf("%02x", terminal_rec_cwt);
                MyPrintf("\n");

                MyPrintf("%02x", terminal_rec_bwt);
                MyPrintf("\n");

                MyPrintf("%02x", terminal_rec_bgt);
                MyPrintf("\n");
#endif
            }

            offset++;
        }
        else
        {
            if (g_7816Para.bProlType == 1)
            {
                return ISO7816_ATR_TB3_ERROR;
            }
        }

        //TC3
        if ((tmpTDx & BIT6) != 0)
        {
            //TC3只支持0
            if (0 != pATR[offset])
            {
                return ISO7816_ATR_TC3_ERROR;
            }
            offset++;
        }
    }
    else
    {
        //如果是T1协议，没有TB3，则需要复位操作
        if (g_7816Para.bProlType == 1)
        {
            return ISO7816_ATR_TB3_ERROR;
        }
    }
    return OK;
}

uint16_t ISO7816_OperateSelect(uint8_t OperateMode, uint8_t vcard_set)
{
    uint16_t ret;
    switch (OperateMode)
    {
    case ISO7816_ACTIVE_CARD:
        ret = crad_in_io_filter();
        if (ret == sci_pin.detect_state)
        {
            return ISO7816_CARD_STUTES_ERROR;
        }
        if ((SYSCTRL_HWCTRL_VAL & 8) == 0)
        {
            ret = ISO7816_cold_reset(vcard_set);
        }
        else
        {
            ISO7816_warm_reset();
            ret = OK;
        }
        if (ret != OK)
        {
            return ISO7816_CARD_STUTES_ERROR;
        }

        //表示冷复位
        g_7816Para.bFlag &= (~BIT1);
        break;
    case ISO7816_WARM_RESET:
        ISO7816_warm_reset();

        //表示热复位
        g_7816Para.bFlag |= BIT1;
        break;
    case ISO7816_DEACTIVE_CARD:
        ret = crad_in_io_filter();
        if (ret == sci_pin.detect_state)
        {
            return ISO7816_CARD_STUTES_ERROR;
        }

        //IC卡下电
        iso7816_deactive();
        break;
    default:
        return ISO7816_OPERATE_MODE_ERROR;
    }
    return OK;
}


uint16_t ISO7816_DisposePPS(uint8_t TA1)
{
    uint16_t f;
    uint8_t rateL, rateH;
    uint8_t   fi, di;

    fi = GET_4_BITS_H(TA1);
    di = GET_4_BITS_L(TA1);
    if ((!fi_table[fi]) || (!di_table[di]))
    {
        return ISO7816_PARA_ERROR;
    }
    f = fi_table[fi] / di_table[di];

    rateL = (uint8_t)f;
    rateH = f >> 8;

    if (g_7816Para.bTA2Flag == 0)
    {
        f = 372;
    }
    else
    {
        if ((g_7816Para.bTA2 & 0x10) != 0)
        {
            f = 372;
        }
    }

    //设置通讯速率寄存器
    SCI7816_ETU = f;

    return OK;
}


uint16_t ISO7816_Dispose_CMD(uint8_t *pCmd, uint16_t CmdLen, uint8_t *pResp,  uint16_t *pRespLen)
{
    uint16_t  ret;
    uint16_t  tmpRespLen, tmpCmdLen;
    uint16_t  tmpOffset;
    uint8_t   tmpCMDBuf[5];
    uint8_t   tmpRECBuf[286];
    uint8_t   tmp61Flag;

    tmpOffset = 0;
    tmp61Flag = 0;
    tmpCmdLen =  CmdLen;

    if (PROTOCOL_T0 == g_7816Para.bProlType)
    {
        //处理61xx 6cxx  也可以返回给上层应用处理
        if ((g_7816Para.bFlag & BIT2) != 0) //T0 Process
        {
            memcpy(tmpCMDBuf, pCmd, 5);
            ret = ISO7816_TPDU_T0(pCmd, tmpCmdLen,  tmpRECBuf + tmpOffset, &tmpRespLen);
            COMPARE_RETURN(ret);
            tmpOffset += tmpRespLen;

            while (1)
            {
                if (tmpRespLen >= 2)
                {
                    if (tmpRECBuf[tmpOffset - 2] == 0x61)
                    {
                        tmpCMDBuf[0] = 0;
                        tmpCMDBuf[1] = 0xC0;
                        tmpCMDBuf[2] = 0;
                        tmpCMDBuf[3] = 0;
                        tmpCMDBuf[4] = tmpRECBuf[tmpOffset - 1];
                        tmpOffset -= 2;
                        ret = ISO7816_TPDU_T0(tmpCMDBuf, 5, tmpRECBuf + tmpOffset, &tmpRespLen);
                        COMPARE_RETURN(ret);

                        tmp61Flag = 1;
                        tmpOffset += tmpRespLen;

                        continue;
                    }
                    else if (tmpRECBuf[tmpOffset - 2] == 0x6C)
                    {
                        tmpCMDBuf[4] = tmpRECBuf[tmpOffset - 1];
                        tmpOffset -= 2;
                        ret = ISO7816_TPDU_T0(tmpCMDBuf, 5, tmpRECBuf + tmpOffset, &tmpRespLen);
                        COMPARE_RETURN(ret);

                        tmpOffset += tmpRespLen;

                        continue;
                    }
                    else if ((tmpRECBuf[tmpOffset - 2] == 0x90) && (tmpRECBuf[tmpOffset - 1] == 0x00))
                    {
                        break;
                    }
                    //data
                    else if ((tmpRECBuf[tmpOffset - 2] == 0x62) || (tmpRECBuf[tmpOffset - 2] == 0x63) || ((tmpRECBuf[tmpOffset - 2] & 0xF0) == 0x90))
                    {
                        if (tmp61Flag == 1)
                        {
                            break;
                        }

                        if ((g_7816Para.bFlag & BIT3) != 0)
                        {
                            break;
                        }

                        if ((CmdLen > 5) && (tmpCMDBuf[1] == 0xA4))
                        {
                            tmpCMDBuf[0] = 0;
                            tmpCMDBuf[1] = 0xC0;
                            tmpCMDBuf[2] = 0;
                            tmpCMDBuf[3] = 0;
                            tmpCMDBuf[4] = 0;
                            tmpOffset -= 2;
                            ret = ISO7816_TPDU_T0(tmpCMDBuf, 5, tmpRECBuf + tmpOffset, &tmpRespLen);
                            COMPARE_RETURN(ret);

                            tmpOffset += tmpRespLen;
                            continue;
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
                }
                else
                {
                    break;
                }


            }

            memcpy(pResp, tmpRECBuf, tmpOffset);
            *pRespLen = tmpOffset;
            return OK;
        }
        else
        {
            return ISO7816_TPDU_T0(pCmd, CmdLen, pResp, pRespLen);
        }
    }
    else
    {
        g_T1.bIFSD = 254;
        return ISO7816_TPDU_T1(pCmd, CmdLen, pResp, pRespLen);
    }
    return OK;
}

uint16_t ISO7816_LoopBack(void)
{
    uint8_t ATRLen;
    uint16_t Ret;
    uint16_t RecLen;
    uint16_t SendLen;
    uint8_t ApduCmd[280];

    uint8_t SELECT_PSE[]	= {0x00, 0xA4, 0x04, 0x00, 0x0E,
                               0x31, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00
                           };  /* SELECT_PPSE */

    atr_time_start();

TOResetCard:
#if 1
    /*if(KEY1_IO_STATE)
    {
    	delay_ms(30000);
    }
    else
    {
    	delay_ms(5000);
    }*/
    delay_ms(3000);
#endif

    //激活卡片
    Ret = ISO7816_OperateSelect(ISO7816_ACTIVE_CARD, 1);
    if (Ret == ISO7816_CARD_STUTES_ERROR)
    {
        goto TOResetCard;
    }

TOWarmResetCard:
    //接收复位信息
    Ret = ISO7816_GetAtr(&g_7816Para.aAtr[1], &ATRLen);

    //清除atr相关计算器
    if (OK != Ret)
    {
        if ((ISO7816_ATR_LEN_ERROR == Ret))
        {
            if ((g_7816Para.bFlag & BIT1) == 0)
            {
                delay_ms(10);
                ISO7816_OperateSelect(ISO7816_WARM_RESET, 1);
                goto TOWarmResetCard;
            }

        }
        iso7816_deactive();
        delay_ms(5);
        goto TOResetCard;
    }

    g_7816Para.aAtr[0] = ATRLen;
    delay_ms(50);

    //自动发送C0
    g_7816Para.bFlag |= BIT2;
    memset(&g_T1.bIFSC, 0, sizeof(g_T1));

    g_T1.bIFSC = 32;
    //ATR数据处理
    Ret = ISO7816_DisposeATR(&g_7816Para.aAtr[1], ATRLen);
    if (OK != Ret)
    {
        if ((Ret == ISO7816_ATR_TA1_ERROR) || (Ret == ISO7816_ATR_TB1_ERROR)
                || (Ret == ISO7816_ATR_TB3_ERROR) || (Ret == ISO7816_ATR_TA3_ERROR)
                || (Ret == ISO7816_ATR_TC2_ERROR) || (Ret == ISO7816_ATR_TC3_ERROR)
                || (Ret == ISO7816_ATR_TA2_ERROR) || (Ret == ISO7816_ATR_TD2_ERROR)
                || (Ret == ISO7816_ATR_TB2_ERROR))
        {
            if ((g_7816Para.bFlag & BIT1) == 0)
            {
                ISO7816_OperateSelect(ISO7816_WARM_RESET, 1);
                goto TOWarmResetCard;
            }
            else
            {
                ISO7816_OperateSelect(ISO7816_DEACTIVE_CARD, 1);
                delay_ms(10);
                goto TOResetCard;
            }
        }
        else
        {
            ISO7816_OperateSelect(ISO7816_DEACTIVE_CARD, 1);
            delay_ms(10);
            goto TOResetCard;
        }
    }

    ISO7816_config_mode_fun();

#ifdef DEBUG_7816_EMV
    MyPrintf("reset ok\n");
#endif

    //处理PPS
    if ((g_7816Para.bFlag & BIT0) == 1)
    {
        if (g_7816Para.bTA1 != 0x11)
        {
            Ret = ISO7816_DisposePPS(g_7816Para.bTA1);
            if (OK != Ret)
            {
                ISO7816_OperateSelect(ISO7816_DEACTIVE_CARD, 1);
                delay_ms(10);
                goto TOResetCard;
            }
        }
    }

    memcpy(ApduCmd, SELECT_PSE, 20);
    SendLen = 20;

    //t1
    if (g_7816Para.bProlType == PROTOCOL_T1)
    {
        uint8_t TmpCmd[5];
        //设置IFSD
        g_T1.bIFSD = 254;
        TmpCmd[0] = 254;

        Ret = ISO7816_TPDU_T1Send(TmpCmd, 1, T_S_IFS);
        if (Ret != OK)
        {
            ISO7816_OperateSelect(ISO7816_DEACTIVE_CARD, 1);
            delay_ms(10);
            goto TOResetCard;
        }
    }

TOSendCMD:
    delay_ms(2);

#ifdef DEBUG_7816_EMV
    MyPrintf("cmd\n");
#endif
    Ret = ISO7816_Dispose_CMD(ApduCmd, SendLen, g_7816Para.aRecBuff, &RecLen);
    if (OK != Ret)
    {
        if ((ISO7816_PARITY_ERROR == Ret))
        {
            ISO7816_OperateSelect(ISO7816_WARM_RESET, 1);
            delay_ms(10);
            goto TOWarmResetCard;
        }

        ISO7816_OperateSelect(ISO7816_DEACTIVE_CARD, 1);
        delay_ms(10);
        goto TOResetCard;

    }
#ifdef DEBUG_7816_EMV
    MyPrintf("cmd ok\n");
#endif
    delay_ms(2);

    if (RecLen >= 6)
    {
        if (g_7816Para.aRecBuff[1] == 0x70)
        {
            ISO7816_OperateSelect(ISO7816_DEACTIVE_CARD, 1);
            delay_ms(100);
            goto TOResetCard;
        }
        else
        {
            SendLen = RecLen - 2;
            memcpy(ApduCmd, g_7816Para.aRecBuff, SendLen);

            delay_ms(2);
            goto TOSendCMD;
        }
    }
    else
    {
        if (g_7816Para.aRecBuff[0] == 0xff)
        {
            ISO7816_OperateSelect(ISO7816_DEACTIVE_CARD, 1);
            delay_ms(10);
            goto TOResetCard;
        }
    }

    delay_ms(2);

    memcpy(ApduCmd, SELECT_PSE, 20);
    SendLen = 20;
    goto TOSendCMD ;
}
