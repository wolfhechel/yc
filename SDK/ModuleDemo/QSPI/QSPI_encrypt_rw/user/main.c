/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\QSPI\QSPI_encrypt_rw\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    21-July-2020
  * @brief   QSPI encrypt rw test program.
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
void test_enc_write_flash_32byte(uint32_t base_addr);
void test_enc_write_flash_32K(uint32_t base_addr);
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();

    MyPrintf("YC3121 QSPI Encrypt read write Demo !\n\n");

    test_enc_write_flash_32byte(0x1020000);

    test_enc_write_flash_32K(0x1030000);

    MyPrintf("TEST END!\r\n");

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

void test_enc_write_flash_32byte(uint32_t base_addr)
{
    uint8_t wbuf[32] = {0};
    Boolean isblank = FALSE;
    //check flash is blank
    isblank = flash_blank_check(base_addr & 0xffffff, (base_addr + 0x20) & 0xffffff);
    MyPrintf("\r\nflash blank check(%x~%x)= %d", base_addr, base_addr + 0x20, isblank);

    enc_erase_flash_32byte(base_addr);

    for (int i = 0; i < 32; i++)
    {
        wbuf[i] = i;
    }
    printv(wbuf, 32, "wbuf:");

    enc_write_flash(base_addr, wbuf, 32);
    prefetch((volatile uint32_t *)base_addr, (volatile uint32_t *)(base_addr + 32));

    //read new data
    printv((volatile uint8_t *)(base_addr), 32, "base_addr NEW data:");
    for (int i = 0; i < 32; i++)
    {
        if (wbuf[i] != *(volatile uint8_t *)(base_addr + i))
        {
            MyPrintf("Error wbuf[%d] = %x\r\n", i, *(volatile uint8_t *)(base_addr + i));
        }
    }
}

void test_enc_write_flash_32K(uint32_t base_addr)
{
    uint8_t wbuf[256] = {0};
    uint8_t rbuf[256] = {0};
    uint32_t i = 0, j = 0;
    uint32_t addr = 0;

    enc_erase_flash_32k(base_addr);

    for (j = 0; j < 32 * 1024; j += 256)
    {
        addr = base_addr + j;

        for (i = 0; i < 256; i++)
        {
            wbuf[i] = i;
        }

        enc_write_flash(addr, wbuf, 256);

        enc_read_flash(addr, rbuf, 256);

        for (i = 0; i < 256; i++)
        {
            if (rbuf[i] != wbuf[i])
            {
                MyPrintf("\n test error rbuf[%d]=%x wbuf[%d]=%x", i, rbuf[i], i, wbuf[i]);
            }
        }
    }
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
