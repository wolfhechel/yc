/**
  ******************************************************************************
  * @file    3121Demo_1.0V\ModuleDemo\USB\USB_CCID\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    23-Oct-2019
  * @brief   USB CCID test program.
  ******************************************************************************
  * @attention
  *
  * COPYRIGHT 2019 Yichip Microelectronics
  *
  * The purpose of this demo is to provide guidance to customers engaged in
  * programming work to guide them smoothly to product development,
  * so as to save their time.
  *
  * Therefore, Yichip microelectronics shall not be responsible for any direct,
  * indirect or consequential damages caused by any content of this demo
  * and/or contained in this code used by the customer in its products.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "yc3121.h"
#include "yc_gpio.h"
#include "yc_uart.h"
#include "yc_ipc.h"
#include "usb_main.h"
#include "board_config.h"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);

uint8_t bt_standby = 0;

#define UART_DEBUG

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  UART_Configuration();

  IpcInit();

  MyPrintf("USB CCID Demo !\n\n");

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

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
