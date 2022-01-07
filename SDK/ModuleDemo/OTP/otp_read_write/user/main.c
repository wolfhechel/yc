/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\OTP\otp_read_write\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    21-July-2020
  * @brief   otp read write test program.
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
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BLOCK_NUM   (1)
#define OTP_W_ADDR (0x1000+BLOCK_NUM*64)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void otp_read_test(void);
void otp_write_test(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();

    MyPrintf("Yc3121 otp read write Demo !\n\n");

    otp_read_test();
    otp_write_test();

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


void otp_write_test(void)
{
    int addr = OTP_W_ADDR;
    uint8_t rbuf[64] = {0};
    uint8_t wbuf[64] = {0};
    for (int i = 0; i < 64; i++)
    {
        wbuf[i] = i;
    }

    read_otp(addr, rbuf, 64);
    printv(rbuf, 64, "read otrp:");

    write_otp(addr, wbuf, 64);

    read_otp(addr, rbuf, 64);
    printv(rbuf, 64, "read otrp:");
}

void otp_read_test(void)
{
    uint8_t tmp[1024] = {0};
    uint8_t id[6];
    uint32_t i = 0;;

    read_otp(0, tmp, 10);
    for (i = 0; i < 10; i++)
    {
        MyPrintf("\n i = %d %x", i, tmp[i]);
    }

    read_chipid(id);
    for (i = 0; i < 6; i++)
    {
        MyPrintf("\n ID[%d] = %x", i, id[i]);
    }
    MyPrintf("\n life = %x", read_chip_life());
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
