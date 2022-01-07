/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\CryptTest\SHA_TEST\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    15-July-2020
  * @brief   SHA TEST program.
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
#include "yc_sha.h"
#include "yc_rand.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void test_sha(void);
int bn_read_string_to_bytes(uint8_t *r, uint32_t len, const char *s);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();

    MyPrintf("Yichip Yc3121 SHA test Demo V1.0.\r\n");

    test_sha();

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

void test_sha(void)
{
    const char *cc1_160 = "39F37D135FA66858516859BEF7E32F6C40D23640";
    const char *cc1_224 = "CBAE2F3E2A35AD314C8433509129F1B9AFC902E2BE144AF565CEF4EB";
    const char *cc1_256 = "346A578A87CFFC251AE7B0FE9ED08607C3537B691CFE7F521F6AD7D5AF015D90";
    const char *cc1_384 = "FBEBF75AD47A69F30729A9C2620904AFBD1BBCC237645E0CCA474AAD36320ED008FC5D9E2A2208C5FDF37F967A0BAEE9";
    const char *cc1_512 = "4E5B77FEF46B48EFD195BB5478B82CB70F203BF5600345B9B7C00567444C029E2E85143A321C7E88C558F564B2D856FB4CFB6979F2D37A27A5531E469809AC6A";

    #define inputlen        (1024 * 10)
    int i = 0;

    unsigned char c_512[inputlen + 1] = {0};

    unsigned char c1_160[160 / 8] = {0};
    unsigned char c1_224[224 / 8] = {0};
    unsigned char c1_256[256 / 8] = {0};
    unsigned char c1_384[384 / 8] = {0};
    unsigned char c1_512[512 / 8] = {0};

    unsigned char exp_c1_160[160 / 8] = {0};
    unsigned char exp_c1_224[224 / 8] = {0};
    unsigned char exp_c1_256[256 / 8] = {0};
    unsigned char exp_c1_384[384 / 8] = {0};
    unsigned char exp_c1_512[512 / 8] = {0};

    MyPrintf("\n*************SHA Test In*************\n");

    bn_read_string_to_bytes(exp_c1_160, sizeof(exp_c1_160), cc1_160);
    bn_read_string_to_bytes(exp_c1_224, sizeof(exp_c1_224), cc1_224);
    bn_read_string_to_bytes(exp_c1_256, sizeof(exp_c1_256), cc1_256);
    bn_read_string_to_bytes(exp_c1_384, sizeof(exp_c1_384), cc1_384);
    bn_read_string_to_bytes(exp_c1_512, sizeof(exp_c1_512), cc1_512);

    for (i = 0; i < inputlen; i++)
        c_512[inputlen - i - 1] = i;

    memset(c1_160, 0, sizeof(c1_160));
    memset(c1_224, 0, sizeof(c1_224));
    memset(c1_256, 0, sizeof(c1_256));
    memset(c1_384, 0, sizeof(c1_384));
    memset(c1_512, 0, sizeof(c1_512));
    SHA(SHATYPE_1, c1_160, c_512, inputlen);
    MyPrintf("SHA c1_160 test:%2x\n", 0 == (memcmp(c1_160, exp_c1_160, sizeof(exp_c1_160))));
    SHA(SHATYPE_224, c1_224, c_512, inputlen);
    MyPrintf("SHA c1_224 test:%2x\n", 0 == (memcmp(c1_224, exp_c1_224, sizeof(exp_c1_224))));
    SHA(SHATYPE_256, c1_256, c_512, inputlen);
    MyPrintf("SHA c1_256 test:%2x\n", 0 == (memcmp(c1_256, exp_c1_256, sizeof(exp_c1_256))));
    SHA(SHATYPE_384, c1_384, c_512, inputlen);
    MyPrintf("SHA c1_384 test:%2x\n", 0 == (memcmp(c1_384, exp_c1_384, sizeof(exp_c1_384))));
    SHA(SHATYPE_512, c1_512, c_512, inputlen);
    MyPrintf("SHA c1_512 test:%2x\n", 0 == (memcmp(c1_512, exp_c1_512, sizeof(exp_c1_512))));
}

/*
 * Import from an ASCII string to an big endian data array
 */

static int bn_get_digit(uint32_t *d, char c)
{
    *d = 255;

    if (c >= 0x30 && c <= 0x39) *d = c - 0x30;
    if (c >= 0x41 && c <= 0x46) *d = c - 0x37;
    if (c >= 0x61 && c <= 0x66) *d = c - 0x57;

    if (*d >= (uint32_t) 16)
        return (-1);

    return (0);
}

int bn_read_string_to_bytes(uint8_t *r, uint32_t len, const char *s)
{
    uint32_t i, j, slen;
    uint32_t d;

    slen = strlen(s);

    if ((len * 2) < slen)
        return (-1);

    memset(r, 0, len);

    len = len - 1;

    for (i = slen, j = 0; i > 0; i--, j++)
    {
        if (-1 == bn_get_digit(&d, s[i - 1]))
            return (-1);
        r[len - j / 2] |= d << ((j % 2) << 2);
    }

    return (0);
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
