/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\CryptTest\SM2_ENC&DEC_TEST\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    15-July-2020
  * @brief   SM2 ENC&DEC TEST program.
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
#include "yc_sm2.h"
#include "yc_rand.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SM2_ENC 0
#define SM2_DEC 1

#define KEY_WORDS (256 + 31) / 32
#define KEY_BYTES (256 + 7 ) / 8

#define ce     "B524F552CD82B8B028476E005C377FB19A87E6FC682D48BB5D42E3D9B9EFFE76"
#define crand  "6CB28D99385C175C94F94E934817663FC176D925DD72B727260DBAAE1FB2F96F"

#define cp     "8542D69E4C044F18E8B92435BF6FF7DE457283915C45517D722EDB8B08F1DFC3"
#define ca     "787968B4FA32C3FD2417842E73BBFEFF2F3C848B6831D7E0EC65228B3937E498"
#define cb     "63E4C6D3B23B0C849CF84241484BFE48F61D59A5B16BA06E6E12D1DA27C5249A"
#define cGx    "421DEBD61B62EAB6746434EBC3CC315E32220B3BADD50BDC4C4E6C147FEDD43D"
#define cGy    "0680512BCBB42C07D47349D2153B70C4E5D7FDFCBFA36EA1A85841B9E46E09A2"
#define cn     "8542D69E4C044F18E8B92435BF6FF7DD297720630485628D5AE74EE7C32E79B7"

#define ckeyd  "1649AB77A00637BD5E2EFE283FBF353534AA7F7CB89463F208DDBC2920BB0DA0"
#define cex    "435B39CCA8F3B508C1488AFC67BE491A0F7BA07E581A0E4849A5CF70628A7E0A"
#define cey    "75DDBA78F15FEECB4C7895E2C1CDF5FE01DEBB2CDBADF45399CCF77BBA076A42"

#define cENTLa "0090"
#define cIDa   "414C494345313233405941484F4F2E434F4D"

#define cm     "6D65737361676520646967657374"
#define cZa    "F4A38489E32B45B6F876E3AC2168CA392362DC8F23459C1D1146FC3DBFB7BC9A"
#define _cm    "F4A38489E32B45B6F876E3AC2168CA392362DC8F23459C1D1146FC3DBFB7BC9A6D65737361676520646967657374"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t test_cplain[] = {0x65, 0x6E, 0x63, 0x72, 0x79, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x74, 0x61, 0x6E, 0x64, 0x61, 0x72, 0x64};
uint8_t _test_cplain[512];
uint8_t test_ccipher[40 + 65 + 32];
uint8_t  message[19];
uint32_t mlen;
uint8_t  cipher[116];
uint32_t clen;

/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void sm2_enc_dec_test(void);
void SM2_ALU(SM2_PrivateKeyTypeDef Key, uint16_t Keylen, uint8_t *Input, uint32_t InputLen, uint8_t *Output, uint32_t *OutputLen, uint8_t Mode);

/*
 * Convert an ASCII character to digit value
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
/*
 * Import from an ASCII string to an big endian data array
 */
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
        r[j / 2] |= d << ((j % 2) << 2);
    }

    return (0);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    SM2_PrivateKeyTypeDef Key;

    UART_Configuration();

    MyPrintf("Yichip Yc3121 SM2 ENC&DEC test Demo V1.0.\r\n");

    bn_read_string_to_bytes(Key.d, 	 KEY_WORDS * 4, ckeyd);
    bn_read_string_to_bytes(Key.e.x, KEY_WORDS * 4, cex);
    bn_read_string_to_bytes(Key.e.y, KEY_WORDS * 4, cey);
    printv(Key.d, KEY_WORDS * 4, "d:");
    printv(Key.e.x, KEY_WORDS * 4, "ex:");
    printv(Key.e.y, KEY_WORDS * 4, "ey:");

    MyPrintf("\n  SM2_Enc           \n");
    SM2_ALU(Key, 256, test_cplain, sizeof(test_cplain), test_ccipher, &clen, SM2_ENC);
    MyPrintf("\n  SM2_Dec           \n");
    SM2_ALU(Key, 256, test_ccipher, clen, _test_cplain, &mlen, SM2_DEC);

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

void SM2_ALU(SM2_PrivateKeyTypeDef Key, uint16_t Keylen, uint8_t *Input, uint32_t InputLen, uint8_t *Output, uint32_t *OutputLen, uint8_t Mode)
{
    SM2_EllipseParaTypeDef para;
    SM2_PrivateKeyTypeDef *key;
    uint8_t config = 0;
    uint32_t ret;

    bn_read_string_to_bytes(para.p, KEY_WORDS * 4, cp);
    bn_read_string_to_bytes(para.a, KEY_WORDS * 4, ca);
    bn_read_string_to_bytes(para.b, KEY_WORDS * 4, cb);
    bn_read_string_to_bytes(para.n, KEY_WORDS * 4, cn);
    bn_read_string_to_bytes(para.g.x, KEY_WORDS * 4, cGx);
    bn_read_string_to_bytes(para.g.y, KEY_WORDS * 4, cGy);

    printv(para.p, KEY_WORDS * 4, "p:"); //数据取反
    printv(para.a, KEY_WORDS * 4, "a:");
    printv(para.b, KEY_WORDS * 4, "b:");
    printv(para.n, KEY_WORDS * 4, "n:");
    printv(para.g.x, KEY_WORDS * 4, "gx:");
    printv(para.g.y, KEY_WORDS * 4, "gy:");

    key = &Key;
    printv(key->d, KEY_WORDS * 4, "d:");
    printv(key->e.x, KEY_WORDS * 4, "e.x:");
    printv(key->e.y, KEY_WORDS * 4, "e.y:");
    switch (Keylen)
    {
    case 192:
        config = ECC_P192;
        break;
    case 224:
        config = ECC_P224;
        break;
    case 256:
        config = ECC_P256;
        break;
    case 163:
        config = ECC_B163;
        break;
    case 193:
        config = ECC_B193;
        break;
    case 233:
        config = ECC_B233;
        break;
    case 257:
        config = ECC_B257;
        break;
    default:
        config = ECC_P256;
        break;
    }

    if (Mode == SM2_ENC)
    {
        ret = SM2_Enc(Output, OutputLen, Input, InputLen, &(key->e), &para, &RAND_RandP, NULL, config);
        if (ret != RET_SM2_ENC_SUCCESS)
            MyPrintf("  SM2_Enc:  ERROR::%c%c%c%c\n", ret, ret >> 8, ret >> 16, ret >> 24);
        else
            MyPrintf("  SM2_Enc:  SUCCESS	\n");
        printv(test_ccipher, clen, "test_ccipher:");
    }
    else if (Mode == SM2_DEC)
    {
        ret = SM2_Dec(Output, OutputLen, Input, InputLen, key, &para, &RAND_RandP, NULL, config);
        if (ret != RET_SM2_DEC_SUCCESS)
            MyPrintf("  SM2_Dec:  ERROR::%c%c%c%c\n", ret, ret >> 8, ret >> 16, ret >> 24);
        else
            MyPrintf("	SM2_Dec: SUCCESS	\n");
        printv(_test_cplain, mlen, "_test_cplain:");
    }
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
