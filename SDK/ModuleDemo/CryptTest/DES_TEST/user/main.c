/**
  ******************************************************************************
  * @file    3121Demo_1.0V\ModuleDemo\CryptTest\DES_TEST\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    15-July-2020
  * @brief   DES TEST program.
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
#include "yc_des.h"
#include "yc_rand.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void test_des(void);
void test_tdes(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();

    MyPrintf("Yichip Yc3121 DES\\TDES test Demo V1.0.\r\n");

    test_des();
    test_tdes();

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

void test_des(void)
{
    uint32_t t;
    PACK_ModeTypeDef modes[2] = {ECB, CBC};
    char *cmodes[2] = {"ECB", "CBC"};
    uint8_t xPlain[2][1024];
    uint8_t xCrypt[1024];

    uint8_t PLAIN[8] = {0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96};

    DES_KeyTypeDef KEY = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    uint8_t IV[8] = {0xF6, 0x9F, 0x24, 0x45, 0xDF, 0x4F, 0x9B, 0x17};

    uint8_t cCrypt[8] = {0x2D, 0x12, 0x1F, 0x90, 0xFC, 0xF6, 0x86, 0x31};
    uint8_t eCrypt[8] = {0x72, 0x77, 0xA0, 0x0D, 0xC1, 0xC1, 0xC3, 0x6B};

    uint8_t plain[8] = {0};
    uint8_t cipher[8] = {0};

    uint8_t m = 0;

    MyPrintf("\n*************DES Test In*************\n");

    memset(plain, 0, sizeof(plain));
    memset(cipher, 0, sizeof(cipher));
    DES_Enc(ECB, cipher, sizeof(cipher), PLAIN, sizeof(PLAIN), KEY, NULL, &RAND_RandP, NULL);
    t = (!memcmp(eCrypt, cipher, sizeof(cipher)));
    MyPrintf("DES_Encrypt ECB test: %2x\n", t);

    DES_Dec(ECB, plain, sizeof(plain), cipher, sizeof(cipher), KEY, NULL, &RAND_RandP, NULL);
    t = (!memcmp(plain, PLAIN, sizeof(PLAIN)));
    MyPrintf("DES_Decrypt ECB test: %2x\n", t);

    memset(plain, 0, sizeof(plain));
    memset(cipher, 0, sizeof(cipher));
    DES_Enc(CBC, cipher, sizeof(cipher), PLAIN, sizeof(PLAIN), KEY, IV, &RAND_RandP, NULL);
    t = (!memcmp(cCrypt, cipher, sizeof(cipher)));
    MyPrintf("DES_Encrypt CBC test: %2x\n", t);

    DES_Dec(CBC, plain, sizeof(plain), cipher, sizeof(cipher), KEY, IV, &RAND_RandP, NULL);
    t = (!memcmp(plain, PLAIN, sizeof(PLAIN)));
    MyPrintf("DES_Decrypt CBC test: %2x\n", t);

    for (m = 0; m < 2; m++)
    {
        char s[30] = {0};
        memset(xPlain, 0, sizeof(xPlain));

        RAND_Rand(xPlain[0], sizeof(xPlain[0]));
        DES_Enc(modes[m], xCrypt, sizeof(xCrypt), xPlain[0], sizeof(xPlain[0]), KEY, NULL, &RAND_RandP, NULL);
        DES_Dec(modes[m], xPlain[1], sizeof(xPlain[1]), xCrypt, sizeof(xCrypt), KEY, NULL, &RAND_RandP, NULL);

        if (!memcmp(xPlain[1], xPlain[0], sizeof(cipher)))
        {
            MyPrintf("%2x, TDES %d %s Test\n", t, sizeof(xPlain[0]), cmodes[m]);
        }
        else
        {
            MyPrintf("%2x, %s", t, s);
        }
    }
}

void test_tdes(void)
{
    uint32_t t;
    PACK_ModeTypeDef modes[2] = {ECB, CBC};
    char *cmodes[2] = {"ECB", "CBC"};

    uint8_t PLAIN[8] = {0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96};

    TDES_KeyTypeDef KEY = {{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
        {0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01},
        {0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23}
    };

    uint8_t IV[8] = {0xF6, 0x9F, 0x24, 0x45, 0xDF, 0x4F, 0x9B, 0x17};

    uint8_t eCrypt[2][8] = {{0x06, 0xED, 0xE3, 0xD8, 0x28, 0x84, 0x09, 0x0A},
        {0x71, 0x47, 0x72, 0xF3, 0x39, 0x84, 0x1D, 0x34}
    };

    uint8_t cCrypt[2][8] = {{0x74, 0x01, 0xCE, 0x1E, 0xAB, 0x6D, 0x00, 0x3C},
        {0x20, 0x79, 0xC3, 0xD5, 0x3A, 0xA7, 0x63, 0xE1}
    };

    uint8_t plain[8];
    uint8_t cipher[8];

    uint8_t xPlain[2][1024];
    uint8_t xCrypt[1024];

    uint32_t m;

    MyPrintf("\n*************TDES Test In*************\n");

    memset(plain, 0, sizeof(plain));
    memset(cipher, 0, sizeof(cipher));
    TDES_Enc(modes[0], cipher, sizeof(cipher), PLAIN, sizeof(PLAIN), &KEY, NULL, &RAND_RandP, NULL);
    t = (!memcmp(&eCrypt[1][0], cipher, sizeof(cipher)));
    MyPrintf("TDES_Encrypt ECB test: %2x\n", t);

    TDES_Dec(modes[0], plain, sizeof(plain), cipher, sizeof(cipher), &KEY, NULL, &RAND_RandP, NULL);
    t = (!memcmp(plain, PLAIN, sizeof(PLAIN)));
    MyPrintf("TDES_Decrypt ECB test: %2x\n", t);

    memset(plain, 0, sizeof(plain));
    memset(cipher, 0, sizeof(cipher));
    TDES_Enc(modes[1], cipher, sizeof(cipher), PLAIN, sizeof(PLAIN), &KEY, IV, &RAND_RandP, NULL);
    t = (!memcmp(&cCrypt[1][0], cipher, sizeof(cipher)));
    MyPrintf("TDES_Encrypt CBC test: %2x\n", t);

    TDES_Dec(modes[1], plain, sizeof(plain), cipher, sizeof(cipher), &KEY, IV, &RAND_RandP, NULL);
    t = (!memcmp(plain, PLAIN, sizeof(PLAIN)));
    MyPrintf("TDES_Decrypt CBC test: %2x\n", t);

    for (m = 0; m < 2; m++)
    {
        char s[30] = {0};
        memset(xPlain, 0, sizeof(xPlain));

        RAND_Rand(xPlain[0], sizeof(xPlain[0]));
        TDES_Enc(modes[m], xCrypt, sizeof(xCrypt), xPlain[0], sizeof(xPlain[0]), &KEY, NULL, &RAND_RandP, NULL);
        TDES_Dec(modes[m], xPlain[1], sizeof(xPlain[1]), xCrypt, sizeof(xCrypt), &KEY, NULL, &RAND_RandP, NULL);

        if (!memcmp(xPlain[1], xPlain[0], sizeof(cipher)))
        {
            MyPrintf("%2x, TDES %d %s Test\n", t, sizeof(xPlain[0]), cmodes[m]);
        }
        else
        {
            MyPrintf("%2x, %s", t, s);
        }
    }
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
