/*
File Name    : yc_iso7816.h
Author       : Yichip
Version      : V2.0
Date         : 2020/7/28
Description  : iso7816 encapsulation.
*/

#ifndef __YC_ISO7816_H__
#define __YC_ISO7816_H__

#include "yc3121.h"
#include "yc_gpio.h"

//#define DEBUG_7816_EMV          //定义时开启EMV测试时打印交互状态
//#define DEBUG_7816              //定义时开启打印交互数据
//#define EMV_L1_TEST             //定义时进行EMV测试，量产代码屏蔽此定义

typedef struct
{
    uint8_t gpio_group;
    uint16_t gpio_pin;
} GPIO_DEF;

typedef struct
{
    GPIO_DEF clk;
    GPIO_DEF io;
    GPIO_DEF reset;
    GPIO_DEF detect;
    uint8_t detect_state;		//未插卡时，卡到位IO电平状态，高为1、低为0
} SCI_PIN_Def;

extern SCI_PIN_Def sci_pin;

#define 	TIM0_CNT    TIM_CNT(0)
#define     SYSCTRL_HWCTRL_VAL  SYSCTRL_HWCTRL(0)     

#define 	ISO7816_TIMx  TIM3

#define 	CONTACT_RESET_L_CNT				contact_reset_l_cnt        //(40000-45000) 复位引脚保持低电平持续时间
#define 	CONTACT_ATR_RFB_TIME_CNT		contact_atr_rfb_time_cnt        //42019(400-40000)	等待IC回应第一个ATR字节的最大时间
#define 	CONTACT_ATR_RAB_TIME_CNT		contact_atr_rab_time_cnt  //19200etu内复位信息应答完成(TS - LAST)
#define     CONTACT_OVER_TIME_CNT           contact_over_time_cnt       //量产中最大的接收、发送超时时间，设置为3S
#define     CONTACT_OVER_TIME_VAL           3                           //量产中最大的接收、发送超时时间，设置为3S

#define 	ATR_SIZE_MAX                0x33
#define	 	REC_SIZE_MAX                0x107 //5+256+2
#define 	SEND_SIZE_MAX               0x103 //3+254+2


#define 	PROTOCOL_T0                 0
#define 	PROTOCOL_T1                 1

//错误码定义
#define 	OK                          0
#define 	NOK                         1

#define     ISO7816_POWER_OFF           0x4E
#define     ISO7816_ATR_ERROR           0x4F
#define     ISO7816_ATR_LEN_ERROR       0x50
#define     ISO7816_ATR_TCK_ERROR       0x51
#define     ISO7816_PROTOCOL_ERROE      0x53
#define     ISO7816_T1_LRC_ERROR        0x54
#define     ISO7816_CARD_STUTES_ERROR   0x56
#define     ISO7816_OPERATE_MODE_ERROR  0x57
#define     ISO7816_PARA_ERROR          0x59
#define     ISO7816_REC_TIMEOUT         0x5A
#define     ISO7816_SEND_TIMEOUT        0x5B
#define     ISO7816_ERR_NUM_OVER        0x5C
#define     ISO7816_T1_TRANSFER_ERROR   0x60

#define     ISO7816_CARD_PRESENT        0x61
#define     ISO7816_CARD_ABSENT         0x62

#define     ISO7816_ATR_TB1_ERROR       0x70
#define 	ISO7816_PROCEDURE_INS_ERROR 0x71

#define     ISO7816_PARITY_ERROR        0x73
#define     ISO7816_ATR_TA1_ERROR       0x74

#define 	ISO7816_ATR_TC2_ERROR		0x75
#define 	ISO7816_ATR_TC3_ERROR		0x76
#define 	ISO7816_ATR_TA2_ERROR		0x77
#define 	ISO7816_ATR_TC1_ERROR		0x78
#define 	ISO7816_ATR_TD2_ERROR		0x79
#define 	ISO7816_ATR_TB2_ERROR		0x80

#define     ISO7816_ATR_TA3_ERROR       0x78
#define     ISO7816_ATR_TB3_ERROR       0x79
#define     ISO7816_RET_RESPONSE_DIFFERENT  0x7C
#define     ISO7816_RET_BROKEN_CHAIN        0x7D
#define     ISO7816_RET_CHAIN               0x7E
#define     ISO7816_DATALEN_ERR             0x7F
#define     ISO7816_NOTLRC_ERROR        0x81


#define 	BIT7    ((uint8_t)1<<7)
#define 	BIT6    ((uint8_t)1<<6)
#define 	BIT5    ((uint8_t)1<<5)
#define 	BIT4    ((uint8_t)1<<4)
#define 	BIT3    ((uint8_t)1<<3)
#define 	BIT2    ((uint8_t)1<<2)
#define 	BIT1    ((uint8_t)1<<1)
#define 	BIT0    ((uint8_t)1<<0)


//***********
#define     ISO7816_ACTIVE_CARD         0x00
#define     ISO7816_WARM_RESET          0x01
#define     ISO7816_DEACTIVE_CARD       0x02

#define     ISO7816_VCC_0V              0x00
#define     ISO7816_VCC_3_3V            0x01
#define     ISO7816_VCC_1_8V            0x02

#define     ISO7816_TYPE_T0             0x00
#define     ISO7816_TYPE_T1             0x01

#define     ISO7816_NULL_T0             0x60

#define     COMPARE_RETURN(ret);        if(OK != ret)        \
                                        {                   \
                                            return  ret;    \
                                        }
#define     GET_4_BITS_H(a)            (((a) >>4)& 0x0F)
#define     GET_4_BITS_L(a)            ((a)& 0x0F)

#define  R_BLK                                 0x80      /*R-BLK的高2位*/
#define  S_BLK                                 0xC0      /*S-BLK的高2位*/
#define  S_RQS_MSK                             0x3F      /*S-BLK request  低6位mask*/
#define  S_RSP_MSK                             0x23      /*S-BLK response 低6位mask*/
#define  IFSC                                  0xFE      /*信息域size*/
#define  HALF_FIFO                             8         /*半空*/
#define  M_BIT                                 0x20      /*表示chain状态*/
#define  NS_BIT                                0x40      /*I-BLK序列号位*/
#define  NR_BIT                                0x10      /*R-BLK序列号位*/

#define  ERR_PARITY                            1         /*奇偶校验错*/
#define  ERR_CHKSUM                            2         /*校验出错*/
#define  ERR_PCB                               4         /*协议控制字节错*/
#define  ERR_LEN                               8         /*长度字节错*/
#define  ERR_RELVNT_S                          16        /*有S request, 无s response*/
#define  ERR_FF								   32		/*接收到数据长度为FF*/
/*7816发送接收*/
#define RECV                                  1
#define SEND                                  0
#define I_BLOCK_C                             0
#define R_BLOCK_C                             1
#define S_BLOCK_C                             2
#define T_S_IFS                                 2          /*调整信息域的S-BLK*/
#define T_S_ABORT                               3          /*打断CHAIN的S-BLK*/
#define T_S_WTS                                 4          /*请求延长等待时的S-BLK*/

#define T1_HEAD_LEN                             3		   /*帧头长度*/
#define ERR_SEND_CNT                            3		   /*错误重发次数*/


//相关全局变量参数
typedef struct
{
    uint8_t   bIFSC;
    uint8_t   bIFSD;
    uint8_t   bNum;
    uint8_t   bNumCard;
    uint8_t   bLink;
    uint8_t   bEDCType;
    uint16_t  wtx;
    uint8_t   bFlgSBSnd;
    uint8_t   bFlgSRec;
    uint8_t   bFlgIRec;
    uint8_t   bRecRBLink;
    uint8_t 	bErrCnt;
    uint8_t 	bErrNotify;
    uint8_t 	bFlagSendChain;
    uint8_t 	aRBlk[5];
} st_T1_Parameters;

//ATR等7816需要用的参数
typedef struct
{
    uint8_t bFlag;
    uint8_t bTA1;		 						//TA1的值
    uint8_t bTA2;								//TA2的值
    uint8_t bTA2Flag;							//TA2是否存在标志
    uint8_t bProlType;						//协议类型标志 0为T0  1为T1
    uint8_t aAtr[ATR_SIZE_MAX];				//ATR缓存区
    uint8_t aRecBuff[REC_SIZE_MAX];			//接收数据缓存区
} st_7816_Para;

//跟T1有关的参数
extern st_T1_Parameters g_T1;
//跟7816协议参数有关的参数
extern st_7816_Para g_7816Para;

extern unsigned int terminal_send_cwt;
extern unsigned int terminal_rec_cwt;
extern unsigned int terminal_rec_bwt;


extern uint16_t fi_table[16];
extern uint16_t di_table[16];

extern unsigned int terminal_rec_bwt_atr;
extern unsigned int terminal_change_rec_bwt_num;

extern uint32_t period_7816;
extern uint32_t contact_reset_l_cnt;
extern uint32_t contact_atr_rfb_time_cnt;
extern uint32_t contact_atr_rab_time_cnt;
extern uint32_t contact_over_time_cnt;

extern volatile unsigned int time0_cnt_last;

void atr_time_start(void);
uint16_t ISO7816_OperateSelect(uint8_t OperateMode, uint8_t vcard_set);
uint16_t ISO7816_GetAtr(uint8_t *pAtr, uint8_t *pLength);
void iso7816_deactive(void);
uint16_t ISO7816_DisposeATR(uint8_t *pATR, uint8_t ATRLen);
void ISO7816_config_mode_fun(void);
uint16_t ISO7816_DisposePPS(uint8_t TA1);
uint16_t ISO7816_TPDU_T1Send(uint8_t *pCommand, uint16_t CommandLength, uint8_t Type);
uint16_t ISO7816_Dispose_CMD(uint8_t *pCmd, uint16_t CmdLen, uint8_t *pResp,  uint16_t *pRespLen);
//state 0-vcard off  1-3.3 vcard   2-1.8 vcard
uint8_t ISO7816_cold_reset(uint8_t state);
void ISO7816_warm_reset(void);
//卡检测
uint8_t detect_io_state(void);
uint16_t ISO7816_LoopBack(void);

#endif	/* __YC_ISO7816_H__ */
