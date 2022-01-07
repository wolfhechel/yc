/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\QSPI\READ_FLASH_SIZE\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    16-July-2020
  * @brief   QSPI read flash size test program.
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
#include "yc_otp.h"
#include "yc_qspi.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    yc_qspi_flash_size_enum flash_size = YC_QFLASH_SIZE_UNKNOW;

    UART_Configuration();

    MyPrintf("YC3121 QSPI read flash size Demo !\r\n");

    flash_size = read_flash_size();

    switch (flash_size)
    {
    case YC_QFLASH_SIZE_UNKNOW:
        MyPrintf("YC3121 flash size is UNKNOW!\r\n");
        break;
    case YC_QFLASH_SIZE_512KB:
        MyPrintf("YC3121 flash size is 512K!\r\n");
        break;
    case YC_QFLASH_SIZE_1MB:
        MyPrintf("YC3121 flash size is 1M!\r\n");
        break;
    case YC_QFLASH_SIZE_4MB:
        MyPrintf("YC3121 flash size is 4M!\r\n");
        break;
    default:
        MyPrintf("YC3121 flash size is read error!\r\n");
        break;
    }

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
