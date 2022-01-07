/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\UART\UART_Interrupt\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    21-July-2020
  * @brief   UART Interrupt test test program.
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
#include "misc.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t p[10] ={'1','2','3','4','5','6','7','8','9','0'};

/* Private function prototypes -----------------------------------------------*/
void NVIC_Configuration(void);
void UART_Configuration(void);
void UART_IT_PrintfMenu(void);
uint8_t GetCmd(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    uint8_t tmpCmd;

    NVIC_Configuration();
    UART_Configuration();

    MyPrintf("UART Interrupt test Demo !\n\n");
    UART_IT_PrintfMenu();

    while (1)
    {
        tmpCmd = 0;
        tmpCmd = GetCmd();

        if (tmpCmd)
        {
            switch (tmpCmd)
            {
            case 'R':
            {
                MyPrintf("TEST UART1 RX INTERRUPT!\n");
                UART_ITConfig(UART1, UART_IT_RX, ENABLE);
                UART_SetRxITNum(UART1, 10);
                UART_SetITTimeout(UART1, 0xff);
                UART_SendBuf(UART1, p,sizeof(p));
                break;
            }
            case 'T':
            {
                MyPrintf("TEST UART1 TX INTERRUPT!\n");
                UART_ITConfig(UART1, UART_IT_TX, ENABLE);
                UART_SendBuf(UART1, p,sizeof(p));
                break;
            }
            default:
            {
                UART_IT_PrintfMenu();
            }
            }
        }
    }
}

/**
  * @brief  NVIC configuration function.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
    NVIC_EnableIRQ(UART1_IRQn);
    NVIC_SetPriority(UART1_IRQn, 1);
}

/**
  * @brief  Serial port initialization function.
  * @param  UART_TypeDef
  * @retval None
  */
void UART_Configuration(void)
{
    UART_InitTypeDef UART_InitStruct;

    /* Configure serial ports 0 RX and TX for IO. */
    GPIO_Config(GPIOA, GPIO_Pin_1, UART0_TXD);
    GPIO_Config(GPIOA, GPIO_Pin_0, UART0_RXD);

    /* Configure serial ports 1 RX and TX for IO. */
    GPIO_Config(GPIOC, GPIO_Pin_7, UART1_TXD);
    GPIO_Config(GPIOC, GPIO_Pin_8, UART1_RXD);

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
    UART_Init(UART1, &UART_InitStruct);
}

/**
  * @brief  The test menu.
  * @param  None
  * @retval None
  */
void UART_IT_PrintfMenu(void)
{
    MyPrintf("=========================================\n");
    MyPrintf("R:set UART_IT_RX\n");
    MyPrintf("T:set UART_IT_TX\n");
    MyPrintf("=========================================\n");
}

/**
  * @brief  Get Cmd.
  * @param  None
  * @retval None
  */
uint8_t GetCmd(void)
{
    uint8_t tmpCmd = 0;

    if (UART_IsRXFIFONotEmpty(UART0))
        tmpCmd = UART_ReceiveData(UART0);

    return tmpCmd;
}

/**
  * @brief  UART1 interrupt service function.
  * @param  None
  * @retval None
  */
void UART1_IRQHandler(void)
{
    if (UART_GetITIdentity(UART1) == UART_IT_RX)
    {
        UART_ClearIT(UART1);
        MyPrintf("UART1 RX data:\n");
        while(UART_IsRXFIFONotEmpty(UART1))
        {
            MyPrintf("%c",UART_ReceiveData(UART1));
        }
        MyPrintf("\nUART1 RX INTERRUPT test successful!\n");
    }
    else if (UART_GetITIdentity(UART1) == UART_IT_TX)
    {
        UART_ClearIT(UART1);
        MyPrintf("UART1 TX INTERRUPT test successful!\n");
    }
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
