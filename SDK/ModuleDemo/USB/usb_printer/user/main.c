/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\UART\UART_TX_SendData\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    4-Dec-2019
  * @brief   UART TX SendData test program.
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
#include "yc_sysctrl.h"
#include "usb.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define uartBaud 115200

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void UART_SendData_test(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();
    
    MyPrintf("UART TX SendData Demo !\n\n");
    
    SYSCTRL_HCLKConfig(SYSCTRL_HCLK_Div_None);
    
    usb_main();
	

	while (1)
	{
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

	/* Configure serial ports RX and TX for IO. */
	GPIO_Config(GPIOA, GPIO_Pin_1, UART0_TXD);
	GPIO_Config(GPIOA, GPIO_Pin_0, UART0_RXD);

	/* USARTx configured as follow:
  - BaudRate = 115200 baud  
  - Word Length = 8 Bits
  - Stop Bit = 1 Stop Bit
  - Parity = No Parity
  - Hardware flow control disabled (RTS and CTS signals)
  - Receive and transmit enabled
  */
	UART_InitStruct.BaudRate = uartBaud;			//Configure serial port baud rate, the baud rate defaults to 128000.
	UART_InitStruct.DataBits = Databits_8b;
	UART_InitStruct.StopBits = StopBits_1;
	UART_InitStruct.Parity = Parity_None;
	UART_InitStruct.FlowCtrl = FlowCtrl_None;
	UART_InitStruct.Mode = Mode_duplex;

	UART_Init(UART0, &UART_InitStruct);
}


/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
