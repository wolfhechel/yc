/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\IIC\IIC_EEPROM\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    16-July-2020
  * @brief   IIC for E2PROM(AT24C02) test program.
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
    MyPrintf("BOARD_VER not support IIC_EEPROM test\n");
  #endif
}

/**
  * @brief  E2PROM data reading and writing test.(AT24C02)
  * @param  None
  * @retval None
  */
void IIC_EEPROM_Test(void)
{
  #if (BOARD_VER == MPOS_BOARD_V2_1)
    /* AT24C02 storage capacity of 2K, 32 pages, 8 bytes per page, address length of 8 bits. */
    /* LSB 0xa0 is the control bit write operation and 0xa1 is the control bit read operation.*/
    uint8_t src_w[10] = {0xa0, 0x00, 's', 'u', 'c', 'c', 'e', 's', 's', '!'};
    uint8_t src_r[3] = {0xa0, 0x00, 0xa1};
    uint8_t dest[9] = {"None"};

    dest[8] = '\0';

    GPIO_Config(IIC_WP2_PORT, IIC_WP2_PIN, OUTPUT_LOW);
    GPIO_Config(IIC_WP128_PORT, IIC_WP128_PIN, OUTPUT_HIGH);

    IIC_SendData(src_w, 10);
    MyPrintf("Initial dest: %s\n", dest);

    IIC_ReceiveData(src_r, 3, dest, 8);
    MyPrintf("End dest: %s\n", dest);

    GPIO_Config(IIC_WP2_PORT, IIC_WP2_PIN, OUTPUT_LOW);
    GPIO_Config(IIC_WP128_PORT, IIC_WP128_PIN, OUTPUT_HIGH);
  #else
    MyPrintf("BOARD_VER not support IIC_EEPROM test\n");
  #endif
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
