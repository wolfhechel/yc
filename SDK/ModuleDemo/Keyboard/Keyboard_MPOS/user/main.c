/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\Keyboard\Keyboard_MPOS\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    15-July-2020
  * @brief   Keyboard test program.
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
#include "yc_uart.h"
#include "yc_timer.h"
#include "yc_lcd.h"
#include "yc_kscan.h"
#include "misc.h"
#include "word.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t keyvalue = 0;
uint16_t keyflag = 0x8000;

/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void NVIC_Configuration(void);
void LCD_Configuration(void);
void TIMER_Configuration(void);
void SPI_LCD_Test(void);
void key_test(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();
    TIMER_Configuration();
    NVIC_Configuration();
    LCD_Configuration();

    MyPrintf("Yichip Yc3121 Keyboard test Demo V1.0.\r\n");

    SPI_LCD_Test();

    while (1)
    {
        key_test();
    }
}

/**
  * @brief  NVIC configuration function.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
    NVIC_EnableIRQ(TIM0_IRQn);
    NVIC_SetPriority(TIM0_IRQn, 0);
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
		MyPrintf("BOARD_VER not support Keyboard_MPOS test\r\n");
	#endif
}

void TIMER_Configuration(void)
{
    TIM_InitTypeDef TIM_InitStruct;

    TIM_InitStruct.TIMx = TIM0;
    TIM_InitStruct.period = 7200000;
    TIM_Init(&TIM_InitStruct);

    TIM_ModeConfig(TIM0, TIM_Mode_TIMER);
    TIM_Cmd(TIM0, ENABLE);
}

void key_test(void)
{
    static unsigned char change = 0;
    unsigned char i;

    if (keyflag == 0x0001 && change == 0)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num2[12 * i], 12, LCD_RS_DATA);
        }
        change = 1;
    }
    if (keyflag == 0x0002 && change == 1)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num3[12 * i], 12, LCD_RS_DATA);
        }
        change = 2;
    }
    if (keyflag == 0x0004 && change == 2)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num4[12 * i], 12, LCD_RS_DATA);
        }
        change = 3;
    }
    if (keyflag == 0x0008 && change == 3)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num5[12 * i], 12, LCD_RS_DATA);
        }
        change = 4;
    }
    if (keyflag == 0x0010 && change == 4)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num6[12 * i], 12, LCD_RS_DATA);
        }
        change = 5;
    }
    if (keyflag == 0x0020 && change == 5)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num7[12 * i], 12, LCD_RS_DATA);
        }
        change = 6;
    }
    if (keyflag == 0x0040 && change == 6)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num8[12 * i], 12, LCD_RS_DATA);
        }
        change = 7;
    }
    if (keyflag == 0x0080 && change == 7)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num9[12 * i], 12, LCD_RS_DATA);
        }
        change = 8;
    }

    if (keyflag == 0x0100 && change == 8)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num1[12 * i], 12, LCD_RS_DATA);
        }
        for (i = 0; i < 6; i++)
        {
            set_xy(112, 5 + i);
            write_bytes_lcd(&num0[12 * i], 12, LCD_RS_DATA);
        }
        change = 9;
    }
    if (keyflag == 0x0200 && change == 9)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num1[12 * i], 12, LCD_RS_DATA);
        }
        for (i = 0; i < 6; i++)
        {
            set_xy(112, 5 + i);
            write_bytes_lcd(&num1[12 * i], 12, LCD_RS_DATA);
        }
        change = 10;
    }
    if (keyflag == 0x0400 && change == 10)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num1[12 * i], 12, LCD_RS_DATA);
        }
        for (i = 0; i < 6; i++)
        {
            set_xy(112, 5 + i);
            write_bytes_lcd(&num2[12 * i], 12, LCD_RS_DATA);
        }
        change = 11;
    }
    if (keyflag == 0x0800 && change == 11)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num1[12 * i], 12, LCD_RS_DATA);
        }
        for (i = 0; i < 6; i++)
        {
            set_xy(112, 5 + i);
            write_bytes_lcd(&num3[12 * i], 12, LCD_RS_DATA);
        }
        change = 12;
    }
    if (keyflag == 0x1000 && change == 12)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num1[12 * i], 12, LCD_RS_DATA);
        }
        for (i = 0; i < 6; i++)
        {
            set_xy(112, 5 + i);
            write_bytes_lcd(&num4[12 * i], 12, LCD_RS_DATA);
        }
        change = 13;
    }
    if (keyflag == 0x2000 && change == 13)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num1[12 * i], 12, LCD_RS_DATA);
        }
        for (i = 0; i < 6; i++)
        {
            set_xy(112, 5 + i);
            write_bytes_lcd(&num5[12 * i], 12, LCD_RS_DATA);
        }
        change = 14;
    }
    if (keyflag == 0x4000 && change == 14)
    {
        for (i = 0; i < 6; i++)
        {
            set_xy(100, 5 + i);
            write_bytes_lcd(&num1[12 * i], 12, LCD_RS_DATA);
        }
        for (i = 0; i < 6; i++)
        {
            set_xy(112, 5 + i);
            write_bytes_lcd(&nume[12 * i], 12, LCD_RS_DATA);
        }
        change = 0;
    }
}

/**
  * @brief  LCD displays test functions
  * @param  None
  * @retval None
  */
void SPI_LCD_Test(void)
{
    unsigned char i;
    GPIO_Config(GPIOA, GPIO_Pin_2, OUTPUT_HIGH);
    clr_screen();

    for (i = 0; i < 3; i++)
    {
        /* Set the starting pixel X and Y positions */
        set_xy(4, i + 1);
        write_bytes_lcd(&Bmp001[24 * i], 24, LCD_RS_DATA);
    }
    for (i = 0; i < 3; i++)
    {
        set_xy(28, i + 1);
        write_bytes_lcd(&Bmp002[24 * i], 24, LCD_RS_DATA);
    }
    for (i = 0; i < 3; i++)
    {
        set_xy(52, i + 1);
        write_bytes_lcd(&Bmp003[24 * i], 24, LCD_RS_DATA);
    }
    for (i = 0; i < 3; i++)
    {
        set_xy(76, i + 1);
        write_bytes_lcd(&Bmp004[24 * i], 24, LCD_RS_DATA);
    }
    for (i = 0; i < 3; i++)
    {
        set_xy(100, i + 1);
        write_bytes_lcd(&Bmp005[24 * i], 24, LCD_RS_DATA);
    }
    for (i = 0; i < 3; i++)
    {
        set_xy(100, 5 + i);
        write_bytes_lcd(&num1[12 * i], 12, LCD_RS_DATA);
    }

    for (i = 0; i < 3; i++)
    {
        set_xy(16, 5 + i);
        write_bytes_lcd(&Test_K[84 * i], 84, LCD_RS_DATA);
    }
}

/**
  * @brief  TIM0 Interrupt service function.
  * @param  None
  * @retval None
  */
void TIMER0_IRQHandler(void)
{
    keyvalue = KEY_Scan();
    if (keyvalue)
    {
        switch (keyvalue)
        {
        case 30:
            keyflag = 0x0001;
            break;
        case 31:
            keyflag = 0x0002;
            break;
        case 32:
            keyflag = 0x0004;
            break;
        case 33:
            keyflag = 0x0008;
            break;
        case 34:
            keyflag = 0x0010;
            break;
        case 35:
            keyflag = 0x0020;
            break;
        case 36:
            keyflag = 0x0040;
            break;
        case 37:
            keyflag = 0x0080;
            break;
        case 38:
            keyflag = 0x0100;
            break;
        case 39:
            keyflag = 0x0800;
            break;
        case 40:
            keyflag = 0x0400;
            break;
        case 41:
            keyflag = 0x0200;
            break;
        case 42:
            keyflag = 0x1000;
            break;
        case 43:
            keyflag = 0x2000;
            break;
        case 44:
            keyflag = 0x4000;
            break;
        default:
            break;
        }
    }
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
