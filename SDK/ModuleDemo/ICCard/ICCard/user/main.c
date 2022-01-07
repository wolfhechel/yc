/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\ICCard\ICCard\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    15-July-2020
  * @brief   ICCard test program.
  ******************************************************************************
  * @attention
  *
  * COPYRIGHT 2019 Yichip Microelectronics
  *
  * The purpose of this firmware is to provide guidance to customers engaged in
  * programming work to guide them smoothly to product development,
  * so as to save their time.
  *
  * Therefore, Yichip microelectronics shall not be responsible for any direct,
  * indirect or consequential damages caused by any content of this gu jiang
  * and/or contained in this code used by the customer in its products.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "yc3121.h"
#include "yc_gpio.h"
#include "yc_uart.h"
#include "yc_timer.h"
#include "yc_7816.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
uint16_t ISO7816_loopBack(void);

void ISO7816_IO_Config(void)
{
    sci_pin.clk.gpio_group = GPIOC;
    sci_pin.clk.gpio_pin = GPIO_Pin_13;

    sci_pin.io.gpio_group = GPIOC;
    sci_pin.io.gpio_pin = GPIO_Pin_14;

    sci_pin.reset.gpio_group = GPIOC;
    sci_pin.reset.gpio_pin = GPIO_Pin_15;

    sci_pin.detect.gpio_group = DET_PORT;
    sci_pin.detect.gpio_pin = DET_PIN;
		GPIO_Config(DET_PORT, DET_PIN, PULL_DOWN);
    sci_pin.detect_state = 1;

    period_7816 = (CPU_MHZ / 4000000 / 2) - 1;
    contact_reset_l_cnt = 42000 * (CPU_MHZ / 1000000) / 4;
    contact_atr_rfb_time_cnt = 47000 * (CPU_MHZ / 1000000) / 4;
    contact_atr_rab_time_cnt = (20164 * (CPU_MHZ / 1000000) / 4) * 372;
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();

    MyPrintf("Yc3121 7816 Demo!\r\n");

    ISO7816_IO_Config();

    while (1)
    {
        ISO7816_loopBack();
    }
}

/**
  * @brief  Serial port 0 initialization function.
  * @param  None
  * @retval None
  */
void UART_Configuration(void)
{
    UART_InitTypeDef UART_InitStruct;

    /* Configure serial ports 0 RX and TX for IO. */
    GPIO_Config(UART0_TX_PORT, UART0_TX_PIN, UART0_TXD);
    GPIO_Config(UART0_RX_PORT, UART0_RX_PIN, UART0_RXD);

    /* USARTx configured as follow:
    - BaudRate = 115200 baud
    - Word Length = 8 Bits
    - Stop Bit = 1 Stop Bit
    - Parity = No Parity
    - Hardware flow control disabled (RTS and CTS signals)
    - Receive and transmit enabled
    */
    UART_InitStruct.BaudRate = UARTBAUD;			//Configure serial port baud rate, the baud rate defaults to 128000.
    UART_InitStruct.DataBits = Databits_8b;
    UART_InitStruct.StopBits = StopBits_1;
    UART_InitStruct.Parity = Parity_None;
    UART_InitStruct.FlowCtrl = FlowCtrl_None;
    UART_InitStruct.Mode = Mode_duplex;

    UART_Init(UART0, &UART_InitStruct);
}

uint16_t iso7816_init(uint8_t vcc_sel, uint8_t *out_atr, uint8_t *atrlen)
{
    uint8_t ATRLen;
    uint16_t Ret;
    uint8_t vcard_num = 0;;
    vcard_num = vcc_sel;
TOResetCard:
    atr_time_start();
    //激活卡片
    Ret = ISO7816_OperateSelect(ISO7816_ACTIVE_CARD, vcard_num);
    if (Ret != OK)
    {
        return Ret;
    }
TOWarmResetCard:
    //接收复位信息
    Ret = ISO7816_GetAtr(&g_7816Para.aAtr[1], &ATRLen);
#ifdef DEBUG_7816
    MyPrintf("ISO7816_GetAtr Result = %x\n", Ret);
#endif
    //清除atr相关计算器
    if (OK != Ret)
    {
        if ((ISO7816_ATR_LEN_ERROR == Ret))
        {
            if ((g_7816Para.bFlag & BIT1) == 0)
            {
                delay_ms(5);
                ISO7816_OperateSelect(ISO7816_WARM_RESET, vcard_num);
                goto TOWarmResetCard;
            }

        }
        iso7816_deactive();
        return Ret;
    }
    //goto TOResetCard;
    g_7816Para.aAtr[0] = ATRLen;
    memcpy(out_atr, g_7816Para.aAtr + 1, ATRLen);
    *atrlen = ATRLen;
    //delay_ms(50);
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
                ISO7816_OperateSelect(ISO7816_WARM_RESET, vcard_num);
                goto TOWarmResetCard;
            }
            else
            {
                ISO7816_OperateSelect(ISO7816_DEACTIVE_CARD, vcard_num);
                delay_ms(10);
                goto TOResetCard;
            }
        }
        else
        {
            ISO7816_OperateSelect(ISO7816_DEACTIVE_CARD, vcard_num);
            delay_ms(10);
            goto TOResetCard;
        }
    }

    ISO7816_config_mode_fun();

    if ((g_7816Para.bFlag & BIT0) == 1)
    {
        if (g_7816Para.bTA1 != 0x11)
        {
            Ret = ISO7816_DisposePPS(g_7816Para.bTA1);
            if (OK != Ret)
            {
                ISO7816_OperateSelect(ISO7816_DEACTIVE_CARD, vcard_num);
                delay_ms(10);
                goto TOResetCard;
            }
        }
    }
    return 0;
}

uint16_t ISO7816_loopBack(void)
{
    uint8_t ATRLen;
    uint16_t SendLen;
    uint16_t RecLen;
    uint8_t ApduCmd[280];
    uint16_t Ret;
    uint8_t SELECT_PSE[]	= {0x00, 0xA4, 0x04, 0x00, 0x0E,
                               0x31, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00
                           };  /* SELECT_PPSE */

TOResetCard:
    Ret = iso7816_init(1, &g_7816Para.aAtr[1], &ATRLen);
    if (Ret != OK)
    {
        ISO7816_OperateSelect(ISO7816_DEACTIVE_CARD, 1);
        delay_ms(10);
        goto TOResetCard;
    }
    memcpy(ApduCmd, SELECT_PSE, 20);
TOSendCMD:

    SendLen = 20;
    Ret = ISO7816_Dispose_CMD(ApduCmd, SendLen, g_7816Para.aRecBuff, &RecLen);
    if (OK != Ret)
    {
        if ((ISO7816_PARITY_ERROR == Ret))
        {
            ISO7816_OperateSelect(ISO7816_WARM_RESET, 1);
            delay_ms(10);
            goto TOResetCard;
        }

        ISO7816_OperateSelect(ISO7816_DEACTIVE_CARD, 1);
        delay_ms(10);
        goto TOResetCard;

    }
    MyPrintf("cmd ok\n");
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

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
