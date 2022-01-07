/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\IIC\IIC_EEPROM_128\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    4-Dec-2019
  * @brief   IIC for E2PROM(AT24C128) test program.
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
#include "yc_iic.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
IIC_InitTypeDef IIC_ConfigureStructure;
IIC_InitTypeDef *IIC_CONFIGCB = &IIC_ConfigureStructure;

/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void IIC_Configuration(void);
void IIC_EEPROM_Test(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();
    IIC_Configuration();

    MyPrintf("Yichip Yc3121 IIC Demo V1.0.\n");

    IIC_EEPROM_Test();

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

/**
  * @brief  E2PROM IIC bus initialization.(AT24C02)
  * @param  None
  * @retval None
  */
void IIC_Configuration(void)
{
  #if (BOARD_VER == MPOS_BOARD_V2_1)
    /* Configure the IIC bus data and clock IO. */
    GPIO_Config(IIC_SDA_PORT, IIC_SDA_PIN,  IIC_SDA);
    GPIO_Config(IIC_SCL_PORT, IIC_SCL_PIN, IIC_SCL);

    /* Configure the IIC bus rate */
    IIC_CONFIGCB->scll = 236;
    IIC_CONFIGCB->sclh = 236;
    IIC_CONFIGCB->stsu = 118;
    IIC_CONFIGCB->sthd = 118;
    IIC_CONFIGCB->sosu = 118;
    IIC_CONFIGCB->dtsu = 118;
    IIC_CONFIGCB->dthd = 0;

    IIC_Init(IIC_CONFIGCB);
  #else
    MyPrintf("BOARD_VER not support IIC_EEPROM_128 test\n");
  #endif
}

/**
  * @brief  E2PROM data reading and writing test.(AT24C128)
  * @param  None
  * @retval None
  */
void IIC_EEPROM_Test(void)
{
  #if (BOARD_VER == MPOS_BOARD_V2_1)
    uint16_t i, j;
    uint8_t src_w[67] = {0};
    uint8_t src_r[4] = {0};
    uint8_t dest[64] = {0};

    src_w[0] = 0xa0;
    src_r[0] = 0xa0;
    src_r[3] = 0xa1;

    GPIO_Config(IIC_WP2_PORT, IIC_WP2_PIN, OUTPUT_HIGH);
    GPIO_Config(IIC_WP128_PORT, IIC_WP128_PIN, OUTPUT_LOW);

    for (i = 0; i < 1; i++)
    {
        src_w[1] = i * 64 >> 8;
        src_w[2] = i * 64 & 0xff;
        src_r[1] = i * 64 >> 8;
        src_r[2] = i * 64 & 0xff;

        for (j = 0; j < 64; j++)
        {
            src_w[3 + j] = (i + j + 2) % 256;
        }

        IIC_SendData(src_w, 67);
        MyPrintf("write:\r\n");
        for(int i=0;i<64;i++) MyPrintf("%02x ",src_w[3+i]);
        MyPrintf("\r\n");
        for (uint32_t i = 0xfffff; i > 0; i--);

        IIC_ReceiveData(src_r, 4, dest, 64);
        for (uint32_t i = 0xfffff; i > 0; i--);
        MyPrintf("read:\r\n");
        for(int i=0;i<64;i++) MyPrintf("%02x ",dest[i]);
        MyPrintf("\r\n");
    }

    GPIO_Config(IIC_WP2_PORT, IIC_WP2_PIN, OUTPUT_HIGH);
    GPIO_Config(IIC_WP128_PORT, IIC_WP128_PIN, OUTPUT_HIGH);
  #else
    MyPrintf("BOARD_VER not support IIC_EEPROM_128 test\n");
  #endif
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
