/**
  ******************************************************************************
  * @file    3121Demo_1.0V\ModuleDemo\CryptTest\AES_TEST\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    15-July-2020
  * @brief   AES TEST program.
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
#include "yc_aes.h"
#include "yc_rand.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void test_aes(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();

    MyPrintf("Yichip Yc3121 AES test Demo V1.0.\r\n");

    test_aes();

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

void test_aes(void)
{
    uint32_t t;

    uint8_t plain[] = {0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96, 0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93, 0x17, 0x2A,
                       0xAE, 0x2D, 0x8A, 0x57, 0x1E, 0x03, 0xAC, 0x9C, 0x9E, 0xB7, 0x6F, 0xAC, 0x45, 0xAF, 0x8E, 0x51
                      };

    uint8_t key128[] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};
    uint8_t key192[] = {0x8E, 0x73, 0xB0, 0xF7, 0xDA, 0x0E, 0x64, 0x52, 0xC8, 0x10, 0xF3, 0x2B, 0x80, 0x90, 0x79, 0xE5,
                        0X62, 0xF8, 0xEA, 0xD2, 0x52, 0x2C, 0x6B, 0x7B
                       };
    uint8_t key256[] = {0x60, 0x3D, 0xEB, 0x10, 0x15, 0xCA, 0x71, 0xBE, 0x2B, 0x73, 0xAE, 0xF0, 0x85, 0x7D, 0x77, 0x81,
                        0x1F, 0x35, 0x2C, 0x07, 0x3B, 0x61, 0x08, 0xD7, 0x2D, 0x98, 0x10, 0xA3, 0x09, 0x14, 0xDF, 0xF4
                       };

    uint8_t iv[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    uint8_t eCipher[3][32] = {{
            0x3A, 0xD7, 0x7B, 0xB4, 0x0D, 0x7A, 0x36, 0x60, 0xA8, 0x9E, 0xCA, 0xF3, 0x24, 0x66, 0xEF, 0x97,
            0xF5, 0xD3, 0xD5, 0x85, 0x03, 0xB9, 0x69, 0x9D, 0xE7, 0x85, 0x89, 0x5A, 0x96, 0xFD, 0xBA, 0xAF
        },
        {
            0xBD, 0x33, 0x4F, 0x1D, 0x6E, 0x45, 0xF2, 0x5F, 0xF7, 0x12, 0xA2, 0x14, 0x57, 0x1F, 0xA5, 0xCC,
            0x97, 0x41, 0x04, 0x84, 0x6D, 0x0A, 0xD3, 0xAD, 0x77, 0x34, 0xEC, 0xB3, 0xEC, 0xEE, 0x4E, 0xEF
        },
        {
            0xF3, 0xEE, 0xD1, 0xBD, 0xB5, 0xD2, 0xA0, 0x3C, 0x06, 0x4B, 0x5A, 0x7E, 0x3D, 0xB1, 0x81, 0xF8,
            0x59, 0x1C, 0xCB, 0x10, 0xD4, 0x10, 0xED, 0x26, 0xDC, 0x5B, 0xA7, 0x4A, 0x31, 0x36, 0x28, 0x70
        }
    };

    uint8_t cCipher[3][32] = {{
            0x76, 0x49, 0xAB, 0xAC, 0x81, 0x19, 0xB2, 0x46, 0xCE, 0xE9, 0x8E, 0x9B, 0x12, 0xE9, 0x19, 0x7D,
            0x50, 0x86, 0xCB, 0x9B, 0x50, 0x72, 0x19, 0xEE, 0x95, 0xDB, 0x11, 0x3A, 0x91, 0x76, 0x78, 0xB2
        },
        {
            0x4F, 0x02, 0x1D, 0xB2, 0x43, 0xBC, 0x63, 0x3D, 0x71, 0x78, 0x18, 0x3A, 0x9F, 0xA0, 0x71, 0xE8,
            0xB4, 0xD9, 0xAD, 0xA9, 0xAD, 0x7D, 0xED, 0xF4, 0xE5, 0xE7, 0x38, 0x76, 0x3F, 0x69, 0x14, 0x5A
        },
        {
            0xF5, 0x8C, 0x4C, 0x04, 0xD6, 0xE5, 0xF1, 0xBA, 0x77, 0x9E, 0xAB, 0xFB, 0x5F, 0x7B, 0xFB, 0xD6,
            0x9C, 0xFC, 0x4E, 0x96, 0x7E, 0xDB, 0x80, 0x8D, 0x67, 0x9F, 0x77, 0x7B, 0xC6, 0x70, 0x2C, 0x7D
        }
    };

    uint8_t cipher[32];
    uint8_t mplain[32];

    MyPrintf("\n*************AES Test In*************\n");

    memset(cipher, 0, sizeof(cipher));
    memset(mplain, 0, sizeof(mplain));
    AES_Enc(ECB, cipher, sizeof(cipher), plain, sizeof(plain), key128, AES_128, iv, &RAND_RandP, NULL);
    AES_Dec(ECB, mplain, sizeof(mplain), cipher, sizeof(cipher), key128, AES_128, iv, &RAND_RandP, NULL);
    t = (!memcmp(&eCipher[0][0], cipher, sizeof(cipher)) && !memcmp(mplain, plain, sizeof(plain)));
    MyPrintf("AES ECB key128 test: %2x\n", t);

    memset(cipher, 0, sizeof(cipher));
    memset(mplain, 0, sizeof(mplain));
    AES_Enc(ECB, cipher, sizeof(cipher), plain, sizeof(plain), key192, AES_192, iv, &RAND_RandP, NULL);
    AES_Dec(ECB, mplain, sizeof(mplain), cipher, sizeof(cipher), key192, AES_192, iv, &RAND_RandP, NULL);
    t = (!memcmp(&eCipher[1][0], cipher, sizeof(cipher)) && !memcmp(mplain, plain, sizeof(plain)));
    MyPrintf("AES ECB key192 test: %2x\n", t);

    memset(cipher, 0, sizeof(cipher));
    memset(mplain, 0, sizeof(mplain));
    AES_Enc(ECB, cipher, sizeof(cipher), plain, sizeof(plain), key256, AES_256, iv, &RAND_RandP, NULL);
    AES_Dec(ECB, mplain, sizeof(mplain), cipher, sizeof(cipher), key256, AES_256, iv, &RAND_RandP, NULL);
    t = (!memcmp(&eCipher[2][0], cipher, sizeof(cipher)) && !memcmp(mplain, plain, sizeof(plain)));
    MyPrintf("AES ECB key256 test: %2x\n", t);

    memset(cipher, 0, sizeof(cipher));
    memset(mplain, 0, sizeof(mplain));
    AES_Enc(CBC, cipher, sizeof(cipher), plain, sizeof(plain), key128, AES_128, iv, &RAND_RandP, NULL);
    AES_Dec(CBC, mplain, sizeof(mplain), cipher, sizeof(cipher), key128, AES_128, iv, &RAND_RandP, NULL);
    t = (!memcmp(&cCipher[0][0], cipher, sizeof(cipher)) && !memcmp(mplain, plain, sizeof(plain)));
    MyPrintf("AES CBC key128 test: %2x\n", t);

    memset(cipher, 0, sizeof(cipher));
    memset(mplain, 0, sizeof(mplain));
    AES_Enc(CBC, cipher, sizeof(cipher), plain, sizeof(plain), key192, AES_192, iv, &RAND_RandP, NULL);
    AES_Dec(CBC, mplain, sizeof(mplain), cipher, sizeof(cipher), key192, AES_192, iv, &RAND_RandP, NULL);
    t = (!memcmp(&cCipher[1][0], cipher, sizeof(cipher)) && !memcmp(mplain, plain, sizeof(plain)));
    MyPrintf("AES CBC key192 test: %2x\n", t);

    memset(cipher, 0, sizeof(cipher));
    memset(mplain, 0, sizeof(mplain));
    AES_Enc(CBC, cipher, sizeof(cipher), plain, sizeof(plain), key256, AES_256, iv, &RAND_RandP, NULL);
    AES_Dec(CBC, mplain, sizeof(mplain), cipher, sizeof(cipher), key256, AES_256, iv, &RAND_RandP, NULL);
    t = (!memcmp(&cCipher[2][0], cipher, sizeof(cipher)) && !memcmp(mplain, plain, sizeof(plain)));
    MyPrintf("AES CBC key256 test: %2x\n", t);
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
