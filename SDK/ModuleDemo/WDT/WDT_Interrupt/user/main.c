/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\WDT\WDT_Interrupt\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    20-July-2020
  * @brief   Yc3121 WDT interrupt test program.
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
#include "yc_wdt.h"
#include "misc.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void NVIC_Configuration(void);
void UART_Configuration(void);
void WDT_Configuration(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    NVIC_Configuration();
    UART_Configuration();

    MyPrintf("YcChip Yc3121 WDT Demo V1.0.\n");

    WDT_Configuration();

    while (1)
    {
        /* Don't feed the dog */
        // WDT_ReloadCounter();
    }
}

/**
  * @brief  NVIC initialization function.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
    NVIC_EnableIRQ(WDT_IRQn);
    NVIC_SetPriority(WDT_IRQn, 0);
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
  * @brief  Watchdog initialization function.
  * @param  None
  * @retval None
  */
void WDT_Configuration(void)
{
    WDT_SetReload(29);//(2^(29-1))/48*1000000 = 5.69(s)
    WDT_ModeConfig(WDT_Interrupt);
    WDT_Enable();
}

/**
  * @brief  Watchdog interrupt service function.
  * @param  None
  * @retval None
  */
void WDT_IRQHandler(void)
{
    WDT_ReloadCounter();
    MyPrintf("WDT Interrupt But Not Reset\n");
    WDT_ClearITPendingBit();
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
