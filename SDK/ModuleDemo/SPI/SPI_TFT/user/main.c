/**
  ******************************************************************************
  * @file    ModuleDemo\SPI\SPI_TFT\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    16-Apr-2020
  * @brief   SPI for TFT320*240(st7789) program.
  ******************************************************************************
  * @attention
  *
  * COPYRIGHT 2020 Yichip Microelectronics
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
#include "yc_spi.h"
#include "yc_st7789.h"
#include "font.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define uartBaud 921600

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void TFT_Configuration(void);
void SPI_TFT_Test(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    SYSCTRL_HCLK_CON = 0x0;
    GPIO_Config(GPIOA, GPIO_Pin_2, OUTPUT_HIGH);//3.3v EN
    UART_Configuration();
    TFT_Configuration();

    MyPrintf("Yichip Yc3121 TFT Demo V1.0.\r\n");

    SPI_TFT_Test();

    uint8_t str[25] = {"YC3121-C1234,567891234567"};
    #if (BOARD_VER == EPOS_BOARD_V1_0)
    ST7789_TFT_ShowString(175, 136, 25 * 16, 16, str, 32, 1, WHITE, OLIVE);
    #endif
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
    - BaudRate = 921600 baud
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
  * @brief  TFT related IO port and SPI related configuration
  * @param  None
  * @retval None
  */
void TFT_Configuration(void)
{
  #if (BOARD_VER == EPOS_BOARD_V1_0)
    St7789TFT_InitTypedef St7789TFT_InitStruct;

    St7789TFT_InitStruct.SPIx = ST7789VTFTSPI;
    St7789TFT_InitStruct.IO_TFT_SDA_PIN = ST7789_TFT_SDA_PIN;
    St7789TFT_InitStruct.IO_TFT_SDA_Port = ST7789_TFT_SDA_PORT;
    St7789TFT_InitStruct.IO_TFT_SCL_PIN = ST7789_TFT_SCL_PIN;
    St7789TFT_InitStruct.IO_TFT_SCL_Port = ST7789_TFT_SCL_PORT;
    St7789TFT_InitStruct.IO_TFT_CS_PIN = ST7789_TFT_CS_PIN;
    St7789TFT_InitStruct.IO_TFT_CS_Port = ST7789_TFT_CS_PORT;
    St7789TFT_InitStruct.IO_TFT_RES_PIN = ST7789_TFT_RST_PIN;
    St7789TFT_InitStruct.IO_TFT_RES_Port = ST7789_TFT_RST_PORT;
    St7789TFT_InitStruct.IO_TFT_BL_PIN = ST7789_TFT_BL_PIN;
    St7789TFT_InitStruct.IO_TFT_BL_Port = ST7789_TFT_BL_PORT;
    St7789TFT_InitStruct.IO_TFT_A0_PIN = ST7789_TFT_A0_PIN;
    St7789TFT_InitStruct.IO_TFT_A0_Port = ST7789_TFT_A0_PORT;

    ST7789_TFT_GpioInit(&St7789TFT_InitStruct);
    ST7789_TFT_Init();
  #else
		MyPrintf("BOARD_VER not support SPI_TFT test\r\n");
	#endif
}

/**
  * @brief  TFT displays test functions
  * @param  None
  * @retval None
  */
void SPI_TFT_Test(void)
{
  #if (BOARD_VER == EPOS_BOARD_V1_0)
    ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
    ST7789_TFT_ShowChinese(100, 1, Yichip_ch, 24, 5, 1, WHITE, OLIVE);
    ST7789_TFT_Draw_Piece(0, 27, TFT_COL, 28, DGRAY);
    ST7789_TFT_ShowChinese(165, 95, Welcom_ch, 36, 4, 1, WHITE, OLIVE);
    ST7789_TFT_ShowString(175, 136, 128, 16, "YC3121-C", 32, 1, WHITE, OLIVE);
    ST7789_TFT_Picture_Q(25, 105, gImage_Yichip);
  #else
		MyPrintf("BOARD_VER not support SPI_TFT test\r\n");
	#endif
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
