/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\CryptTest\RSA_GENKEY_TEST\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    15-July-2020
  * @brief   RSA GENKEY TEST program.
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
#include "yc_rsa.h"
#include "yc_rand.h"
#include "yc_calc.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define PLAIN "1234567890"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void test_rsa_genrsakey(void);
int bn_read_string_to_bytes(uint8_t *r, uint32_t len, const char *s);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();

    MyPrintf("Yichip Yc3121 RSA_GENKEY test Demo V1.0.\r\n");

    test_rsa_genrsakey();

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

void test_rsa_genrsakey(void)
{
    RSA_PrivateKeyTypeDef pri_key;
    RSA_PublicKeyTypeDef pub_key;

    uint8_t plain[2048 / 8];
    uint8_t cipher[2048 / 8];
    uint8_t tmp0[2048 / 8];
    uint32_t ret = 0;

    MyPrintf("\r\n RSA_GenerateKey 1024");

REPEAT_1024:
    ret = RSA_GenerateKey(&pri_key, 0x10001, 1024, &RAND_RandP, NULL);
    if (ret != RET_RSA_KEYGEN_SUCCESS) goto REPEAT_1024;

    MyPrintf("\r\n RSA_GenerateKey OK ");
    memcpy_r(pri_key.p, pri_key.p, pri_key.bytes / 2);
    memcpy_r(pri_key.q, pri_key.q, pri_key.bytes / 2);
    memcpy_r(pri_key.n, pri_key.n, pri_key.bytes);

    printv(pri_key.p, pri_key.bytes / 2, "p");
    printv(pri_key.q, pri_key.bytes / 2, "q");

    printv(pri_key.n, pri_key.bytes, "n");

    MyPrintf("\n*************RSA 1024 Test In*************\n");
    pri_key.bytes = 1024 / 8; //密钥bit长度

    memcpy_r(pri_key.p, pri_key.p, pri_key.bytes / 2);
    memcpy_r(pri_key.q, pri_key.q, pri_key.bytes / 2);
    memcpy_r(pri_key.n, pri_key.n, pri_key.bytes);

    pub_key.bytes = pri_key.bytes;
    memcpy(pub_key.e, pri_key.e, sizeof(pri_key.e));
    memcpy(pub_key.n, pri_key.n, sizeof(pri_key.n));
    memcpy(pub_key.n_c, pri_key.n_c, sizeof(pri_key.n_c));
    memcpy(pub_key.n_q, pri_key.n_q, sizeof(pri_key.n_q));

    bn_read_string_to_bytes(plain, pri_key.bytes, PLAIN);
    memcpy_r(plain, plain, 128);

    MyPrintf("\r\nRSA_public ret:%x\n", RSA_Public(cipher, plain, &pub_key, &RAND_RandP, NULL));
    MyPrintf("\r\nRSA_private_crt ret:%x\n", RSA_Private_crt(tmp0, cipher, &pri_key, &RAND_RandP, NULL));
    printv(tmp0, 10, "tmp0");
    MyPrintf("\r\nTo compare test:%2x\n", (0 == memcmp(tmp0, plain, pri_key.bytes)));

    MyPrintf("\r\n RSA_GenerateKey 2048");

REPEAT_2048:
    ret = RSA_GenerateKey(&pri_key, 0x10001, 2048, &RAND_RandP, NULL);
    if (ret != RET_RSA_KEYGEN_SUCCESS) goto REPEAT_2048;

    MyPrintf("\r\n RSA_GenerateKey OK ");
    memcpy_r(pri_key.p, pri_key.p, pri_key.bytes / 2);
    memcpy_r(pri_key.q, pri_key.q, pri_key.bytes / 2);
    memcpy_r(pri_key.n, pri_key.n, pri_key.bytes);

    printv(pri_key.p, pri_key.bytes / 2, "p");
    printv(pri_key.q, pri_key.bytes / 2, "q");

    printv(pri_key.n, pri_key.bytes, "n");

    MyPrintf("\n*************RSA 2048 Test In*************\n");
    pri_key.bytes = 2048 / 8; //密钥bit长度

    memcpy_r(pri_key.p, pri_key.p, pri_key.bytes / 2);
    memcpy_r(pri_key.q, pri_key.q, pri_key.bytes / 2);
    memcpy_r(pri_key.n, pri_key.n, pri_key.bytes);

    pub_key.bytes = pri_key.bytes;
    memcpy(pub_key.e, pri_key.e, sizeof(pri_key.e));
    memcpy(pub_key.n, pri_key.n, sizeof(pri_key.n));
    memcpy(pub_key.n_c, pri_key.n_c, sizeof(pri_key.n_c));
    memcpy(pub_key.n_q, pri_key.n_q, sizeof(pri_key.n_q));

    bn_read_string_to_bytes(plain, pri_key.bytes, PLAIN);
    memcpy_r(plain, plain, 256);

    MyPrintf("\r\nRSA_public ret:%x\n", RSA_Public(cipher, plain, &pub_key, &RAND_RandP, NULL));
    MyPrintf("\r\nRSA_private_crt ret:%x\n", RSA_Private_crt(tmp0, cipher, &pri_key, &RAND_RandP, NULL));
    printv(tmp0, 10, "tmp0");
    MyPrintf("\r\nTo compare test:%2x\n", (0 == memcmp(tmp0, plain, pri_key.bytes)));
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
