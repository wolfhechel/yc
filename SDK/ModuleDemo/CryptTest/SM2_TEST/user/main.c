/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\CryptTest\SM2_TEST\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    15-July-2020
  * @brief   SM2 TEST program.
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
#define KEY_WORDS 256 / 32
#define KEY_BYTES 256 / 8

#if 0
    #define cp     "8542D69E4C044F18E8B92435BF6FF7DE457283915C45517D722EDB8B08F1DFC3"
    #define ca     "787968B4FA32C3FD2417842E73BBFEFF2F3C848B6831D7E0EC65228B3937E498"
    #define cb     "63E4C6D3B23B0C849CF84241484BFE48F61D59A5B16BA06E6E12D1DA27C5249A"
    #define cGx    "421DEBD61B62EAB6746434EBC3CC315E32220B3BADD50BDC4C4E6C147FEDD43D"
    #define cGy    "0680512BCBB42C07D47349D2153B70C4E5D7FDFCBFA36EA1A85841B9E46E09A2"
    #define cn     "8542D69E4C044F18E8B92435BF6FF7DD297720630485628D5AE74EE7C32E79B7"

    #define ckeyd  "1649AB77A00637BD5E2EFE283FBF353534AA7F7CB89463F208DDBC2920BB0DA0"
    #define cex    "435B39CCA8F3B508C1488AFC67BE491A0F7BA07E581A0E4849A5CF70628A7E0A"
    #define cey    "75DDBA78F15FEECB4C7895E2C1CDF5FE01DEBB2CDBADF45399CCF77BBA076A42"

#else

    #define cp     "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF"
    #define ca     "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC"
    #define cb     "28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93"
    #define cGx    "32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7"
    #define cGy    "BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0"
    #define cn     "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123"

    #define ckeyd  "9515ba696f40b38182bfbc103bf0aca2cdabc9a3276070c52a6215e4a72aabf5"
    #define cex    "94f5e736e01dc8a408174147e422b2401313d7862cf1473bacdf7b435f333fd3"
    #define cey    "cbe2a0ed344e0ee855e72685d9443a01334e2281dfc1765a9ed50cfa5a80f1fc"

#endif

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void TEST_SM2(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();

    MyPrintf("Yichip Yc3121 SM2 test Demo V1.0.\r\n");

    TEST_SM2();

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
        r[j / 2] |= d << ((j % 2) << 2);   //此处已经对读入字符串进行大小端转换，与RSA算法中读入不同
    }

    return (0);
}

void TEST_SM2(void)
{
    SM2_EllipseParaTypeDef para; //SM2国密规定椭圆曲线参数，两组可选
    SM2_PrivateKeyTypeDef key;
    uint32_t ret = 0;
    uint8_t test_ccipher[40 + 65 + 32];
    uint8_t test_cplain[] = {0x65, 0x6E, 0x63, 0x72, 0x79, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x74, 0x61, 0x6E, 0x64};
    uint8_t _test_cplain[512];
    uint32_t clen;
    uint32_t mlen;

    SM2_SignTypeDef sign;
    //此处message和id再对数据进行签名验签时需要注意大小端，此处采用的对称数据，方便测试使用
    unsigned char message[16] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12};
    unsigned char id[16] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12};
    uint8_t _Za[32];
    uint32_t _e[8];

    bn_read_string_to_bytes(para.p, KEY_WORDS * 4, cp);
    bn_read_string_to_bytes(para.a, KEY_WORDS * 4, ca);
    bn_read_string_to_bytes(para.b, KEY_WORDS * 4, cb);
    bn_read_string_to_bytes(para.n, KEY_WORDS * 4, cn);
    bn_read_string_to_bytes(para.g.x, KEY_WORDS * 4, cGx);
    bn_read_string_to_bytes(para.g.y, KEY_WORDS * 4, cGy);

    printv(para.p, KEY_WORDS * 4, "p:");
    printv(para.a, KEY_WORDS * 4, "a:");
    printv(para.b, KEY_WORDS * 4, "b:");
    printv(para.n, KEY_WORDS * 4, "n:");
    printv(para.g.x, KEY_WORDS * 4, "gx:");
    printv(para.g.y, KEY_WORDS * 4, "gy:");
    printv(&para, sizeof(para), "para:"); //应用中如使用memcpy方式拷贝，非二进制域257bit模式，需做对应偏移

    MyPrintf("\r\n*****SM2 GENKEY TEST START*****\r\n");
    ret = SM2_Genkey(&key, &para, &RAND_RandP, NULL, ECC_P256);
    //ret返回值为EKGS生成成功，为EKGF生成失败
    MyPrintf("ret = %c%c%c%c\n", ret >> 24, ret >> 16, ret >> 8, ret);
    printv(key.d, 256 / 8, "d");
    printv(key.e.x, 256 / 8, "key.e.x");
    printv(key.e.y, 256 / 8, "key.e.y");
    printv(&key.e, sizeof(key.e), "pubkey:");//应用中如使用memcpy方式拷贝公钥，非二进制域257bit模式，需做对应偏移
    MyPrintf("\r\n*****SM2 GENKEY TEST END*****\r\n");

    MyPrintf("\r\n*****SM2 ENC&DEC TEST START*****\r\n");
    printv(test_cplain, sizeof(test_cplain), "test_cplain:");
    ret = SM2_Enc(test_ccipher, &clen, test_cplain, sizeof(test_cplain), &(key.e), &para, &RAND_RandP, NULL, ECC_P256);
    if (ret != RET_SM2_ENC_SUCCESS)
        MyPrintf("  SM2_Enc:  ERROR::%c%c%c%c\n", ret, ret >> 8, ret >> 16, ret >> 24);
    else
        MyPrintf("  SM2_Enc:  SUCCESS	\n");
    printv(test_ccipher, clen, "test_ccipher:");

    MyPrintf("\n  SM2_Dec           \n");
    ret = SM2_Dec(_test_cplain, &mlen, test_ccipher, clen, &key, &para, &RAND_RandP, NULL, ECC_P256);
    if (ret != RET_SM2_DEC_SUCCESS)
        MyPrintf("  SM2_Dec:  ERROR::%c%c%c%c\n", ret, ret >> 8, ret >> 16, ret >> 24);
    else
        MyPrintf("	SM2_Dec: SUCCESS	\n");
    printv(_test_cplain, mlen, "_test_cplain:");
    MyPrintf("\r\n*****SM2 ENC&DEC TEST END*****\r\n");

    MyPrintf("\r\n*****SM2 SIGN&VERIFY TEST START*****\r\n");
    SM2_Hash_Za(_Za, id, sizeof(id), &para, &key.e, KEY_BYTES);
    printv(_Za, sizeof(_Za), "_Za:");
    SM2_Hash_e(_e, _Za, message, sizeof(message));
    printv(_e, sizeof(_e), "_e:");

    MyPrintf("\r\nsm2 sign with e test = %d\r\n", (RET_SM2_SIGN_SUCCESS == SM2_DigitalSignWith_e(&sign, _e, NULL, 0, &key, &para, &RAND_RandP, NULL, ECC_P256)));
    printv(&sign, sizeof(sign), "DigitalSignWith:");
    MyPrintf("\r\nsm2 verify with e test = %d\r\n", (RET_SM2_VERIFY_SUCCESS == SM2_VerifySignWith_e(&sign, _e, NULL, 0, &key.e, &para, &RAND_RandP, NULL, ECC_P256)));
    MyPrintf("\r\n*****SM2 SIGN&VERIFY TEST END*****\r\n");
}
/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
