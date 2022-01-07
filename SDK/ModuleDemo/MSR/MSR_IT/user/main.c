/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\MSR\MSR_IT\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    21-July-2020
  * @brief   MSR Interrupt test program.
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
#include "yc_msr.h"
#include "misc.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void NVIC_Configuration(void);

track_data tdata[MAX_TRACK_NUM];
int ret;
int i;
uint8_t cfmt, tflag;
int nResult = 0;
uint8_t Opcode;

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();

    MyPrintf("YcChip Yc3121 MSR Interrupt Demo V1.0.\n");


    MyPrintf("start MSR test\n");
    MSR_Init();

    MSR_ITEnable(ENABLE, TRACK_SELECT_2); //使能磁道二触发中断，
    NVIC_Configuration();
    MyPrintf("please swiping card\n");
    while (1)
    {

    }
}

/**
  * @brief  NVIC configuration function.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
    NVIC_EnableIRQ(MSR_IRQn);
    NVIC_SetPriority(MSR_IRQn, 0);
}

/**
  * @brief  Serial port initialization function.
  * @param  None
  * @retval None
  */
void UART_Configuration(void)
{
    UART_InitTypeDef UART_InitStruct;

    /* Configure serial ports RX and TX for IO. */
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

/**
  * @brief  MSR interrupt service function.
  * @param  None
  * @retval None
  */
void MSR_IRQHandler(void)
{
    MyPrintf("msr irq handler\n");
    if (MSR_DetectSwipingCard() == DETECT_SWIPING_CARD)
    {
        ret = MSR_GetDecodeData(tdata, TRACK_SELECT_1 | TRACK_SELECT_2 | TRACK_SELECT_3, &cfmt, &tflag);
        if (ret == MSR_SUCCESS)
        {
            for (i = 0; i < MAX_TRACK_NUM; i++)
            {
                MyPrintf("track%d len %d\n", i + 1, tdata[i].len);
                if ((tdata[i].len) > 0)
                {
                    MyPrintf("track%d  data:\n", i + 1, tdata[i].len);
                    UART_SendBuf(UART0, tdata[i].buf, tdata[i].len);
                    MyPrintf("\n");
                }
                MyPrintf("\n");
            }
        }

        for (i = 0; i < MAX_TRACK_NUM; i++)
        {
            if (tdata[i].len > 0)
            {
                MyPrintf("T%d OK", i + 1);
            }
            tdata[i].len = 0;
        }
        MSR_ENABLE(DISABLE);
        MSR_ENABLE(ENABLE);
        MyPrintf("please swiping card\n");
    }
    MSR_ClearIT(TRACK_SELECT_2);
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
