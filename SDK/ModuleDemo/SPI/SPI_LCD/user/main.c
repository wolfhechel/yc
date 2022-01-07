/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\SPI\SPI_LCD\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    21-July-2020
  * @brief   SPI for LCD12864(SH1106) program.
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
#include "yc_lcd.h"
#include "yc_timer.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
unsigned char Bmp001[24 * 6] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C,
    0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x80, 0xC0, 0x60, 0x63, 0x32, 0x3E, 0x3E, 0xF6, 0xF2, 0x32,
    0x32, 0x32, 0xF2, 0xF2, 0x32, 0x32, 0x33, 0xF0, 0xF0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x0C, 0x44, 0x66, 0x63, 0x33, 0x31, 0x18, 0x1C,
    0x0E, 0x07, 0x03, 0x60, 0x60, 0x60, 0x70, 0x3F, 0x07, 0x00, 0x00, 0x00
}; /* 易 */
unsigned char Bmp002[24 * 6] =
{
    0x00, 0x00, 0x40, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0xFC, 0xFC, 0x00, 0x00,
    0x00, 0xFC, 0xFC, 0x00, 0x00, 0x80, 0xC0, 0x70, 0x60, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xC0, 0xC0, 0x61, 0x63, 0x31, 0x30, 0xFF, 0xFF, 0x00, 0x00,
    0x00, 0xFF, 0xFF, 0x20, 0x31, 0x71, 0x60, 0xE0, 0xC0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x61, 0x60, 0x30, 0x30, 0x18, 0x0E, 0x07, 0x01, 0x00, 0x00,
    0x00, 0x3F, 0x3F, 0x30, 0x20, 0x20, 0x20, 0x20, 0x38, 0x3C, 0x08, 0x00
}; /* 兆 */
unsigned char Bmp004[24 * 6] =
{
    0x00, 0x00, 0xE0, 0xE0, 0xE0, 0x60, 0x60, 0x60, 0x60, 0x60, 0xFE, 0xFE,
    0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0xFF, 0xFF,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x07, 0x07, 0x01, 0x01, 0x01, 0x01, 0x01, 0x3F, 0x7F,
    0x61, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x60, 0x60, 0x78, 0x10, 0x00
}; /* 微 */
unsigned char Bmp003[24 * 6] =
{
    0x00, 0x40, 0x60, 0x30, 0x1C, 0xCC, 0x80, 0xF0, 0xF0, 0x00, 0xFC, 0xFC,
    0x00, 0xF0, 0x00, 0x00, 0xF0, 0xFC, 0x84, 0x80, 0x80, 0x80, 0xC0, 0x00,
    0x00, 0x20, 0x38, 0x1C, 0xFF, 0x03, 0x00, 0x0D, 0xCD, 0x4D, 0x4D, 0xCD,
    0xCD, 0x0D, 0x19, 0x0F, 0xFF, 0xC0, 0x00, 0xFF, 0x3F, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x10, 0x3C, 0x0F, 0x00, 0x00, 0x0F,
    0x0F, 0x6C, 0x76, 0x30, 0x18, 0x0F, 0x0F, 0x19, 0x30, 0x70, 0x20, 0x00
}; /* 电 */
unsigned char Bmp005[24 * 6] =
{
    0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x98, 0x98, 0xD8, 0x78, 0x78, 0x38, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xFF,
    0xFF, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x60, 0x7F,
    0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
}; /* 子 */

/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void LCD_Configuration(void);
void SPI_LCD_Test(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();
    LCD_Configuration();

    MyPrintf("Yichip Yc3121 LCD Demo V1.0.\r\n");

    GPIO_Config(GPIOA, GPIO_Pin_2, OUTPUT_HIGH);
    GPIO_Config(GPIOC, GPIO_Pin_8, OUTPUT_HIGH);
    GPIO_Config(GPIOC, GPIO_Pin_9, OUTPUT_HIGH);
    GPIO_Config(GPIOB, GPIO_Pin_8, OUTPUT_HIGH);

    SPI_LCD_Test();

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
  * @brief  LCD related IO port and SPI related configuration
  * @param  None
  * @retval None
  */
void LCD_Configuration(void)
{
	#if (BOARD_VER == MPOS_BOARD_V2_1)
    LCD_InitTypedef LCD_ConfigureStructure;

    LCD_ConfigureStructure.SPIx = LCD_SPI;
    LCD_ConfigureStructure.IO_LCD_SDA_Port = LCDSDA_PORT;
    LCD_ConfigureStructure.IO_LCD_SDA_PIN  = LCDSDA_PIN;
    LCD_ConfigureStructure.IO_LCD_SCL_Port = LCDSCL_PORT;
    LCD_ConfigureStructure.IO_LCD_SCL_PIN  = LCDSCL_PIN;
    LCD_ConfigureStructure.IO_LCD_CS_Port  = LCDCS_PORT;
    LCD_ConfigureStructure.IO_LCD_CS_PIN   = LCDCS_PIN;
    LCD_ConfigureStructure.IO_LCD_RES_Port = LCDRST_PORT;
    LCD_ConfigureStructure.IO_LCD_RES_PIN  = LCDRST_PIN;
    LCD_ConfigureStructure.IO_LCD_A0_Port  = LCDA0_PORT;
    LCD_ConfigureStructure.IO_LCD_A0_PIN   = LCDA0_PIN;
    LCD_ConfigureStructure.IO_LCD_BL_Port  = LCDBL_PORT;
    LCD_ConfigureStructure.IO_LCD_BL_PIN   = LCDBL_PIN;

    lcd_io_init(&LCD_ConfigureStructure);

    /* Initialize the LCD screen */
    init_lcd();
	#else
		MyPrintf("BOARD_VER not support SPI_LCD test\r\n");
	#endif
}

/**
  * @brief  LCD displays test functions
  * @param  None
  * @retval None
  */
void SPI_LCD_Test(void)
{
	#if (BOARD_VER == MPOS_BOARD_V2_1)
    unsigned char i;

    clr_screen();

    for (i = 0; i < 6; i++)
    {
        /* Set the starting pixel X and Y positions */
        set_xy(4, i + 2);
        write_bytes_lcd(&Bmp001[24 * i], 24, LCD_RS_DATA);
    }
    for (i = 0; i < 6; i++)
    {
        set_xy(28, i + 2);
        write_bytes_lcd(&Bmp002[24 * i], 24, LCD_RS_DATA);
    }
    for (i = 0; i < 6; i++)
    {
        set_xy(52, i + 2);
        write_bytes_lcd(&Bmp003[24 * i], 24, LCD_RS_DATA);
    }
    for (i = 0; i < 6; i++)
    {
        set_xy(76, i + 2);
        write_bytes_lcd(&Bmp004[24 * i], 24, LCD_RS_DATA);
    }
    for (i = 0; i < 6; i++)
    {
        set_xy(100, i + 2);
        write_bytes_lcd(&Bmp005[24 * i], 24, LCD_RS_DATA);
    }
	#else
		MyPrintf("BOARD_VER not support SPI_LCD test\r\n");
	#endif
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
