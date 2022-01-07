/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\CryptTest\SM4_TEST\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    15-July-2020
  * @brief   SM4 TEST program.
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
#include <string.h>
#include <stdio.h>
#include "yc3121.h"
#include "yc_gpio.h"
#include "yc_uart.h"
#include "yc_sm4.h"
#include "yc_rand.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void test_sm4(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();

    MyPrintf("Yichip Yc3121 SM4 test Demo V1.0.\r\n");

    test_sm4();

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

void test_sm4(void)
{
    uint32_t t;
    uint8_t Key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};
    uint8_t Iv[16] = {0xA3, 0xB1, 0xBA, 0xC6, 0x56, 0xAA, 0x33, 0x50, 0x67, 0x7D, 0x91, 0x97, 0xB2, 0x70, 0x22, 0xDC};
    uint8_t Plain[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};
    uint8_t Crypt[16] = {0x68, 0x1E, 0xDF, 0x34, 0xD2, 0x06, 0x96, 0x5E, 0x86, 0xB3, 0xE9, 0x4F, 0x53, 0x6E, 0x42, 0x46};
    uint8_t mPlain[16];
    uint8_t mCrypt[16];

    uint8_t xPlain[2][32] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};
    uint8_t xCrypt[32];

    MyPrintf("\n*************SM4 Test In*************\n");

    memset(mCrypt, 0, sizeof(mCrypt));
    memset(mPlain, 0, sizeof(mPlain));

    SM4_Enc(ECB, mCrypt, sizeof(mCrypt), Plain, sizeof(Plain), Key, Iv, &RAND_RandP, NULL);
    t = (!memcmp(mCrypt, Crypt, sizeof(Crypt)));
    MyPrintf("SM4 ECB Encrypt Test:%2x\n", t);

    SM4_Dec(ECB, mPlain, sizeof(mPlain), Crypt, sizeof(Crypt), Key, Iv, &RAND_RandP, NULL);
    t = (!memcmp(mPlain, Plain, sizeof(Plain)));
    MyPrintf("SM4 ECB Decrypt Test:%2x\n", t);

    //CBC
    memset(mCrypt, 0, sizeof(mCrypt));
    memset(mPlain, 0, sizeof(mPlain));

    SM4_Enc(CBC, mCrypt, sizeof(mCrypt), Plain, sizeof(Plain), Key, Iv, &RAND_RandP, NULL);
    SM4_Dec(CBC, mPlain, sizeof(mPlain), mCrypt, sizeof(mCrypt), Key, Iv, &RAND_RandP, NULL);

    t = (!memcmp(mPlain, Plain, sizeof(Plain)));
    MyPrintf("SM4 CBC Test:        %2x\n", t);

    //xCBC
    memset(xCrypt, 0, sizeof(xCrypt));

    SM4_Enc(CBC, xCrypt, sizeof(xCrypt), xPlain[0], sizeof(xPlain[0]), Key, Iv, &RAND_RandP, NULL);
    SM4_Dec(CBC, xPlain[1], sizeof(xPlain[1]), xCrypt, sizeof(xCrypt), Key, Iv, &RAND_RandP, NULL);
    t = (!memcmp(xPlain[1], xPlain[0], sizeof(xPlain[1])));
    MyPrintf("SM4 CBC xbytes Test: %2x\n", t);
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
