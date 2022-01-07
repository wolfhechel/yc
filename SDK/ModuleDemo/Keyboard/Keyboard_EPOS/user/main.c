/**
  ******************************************************************************
  * @file    ModuleDemo\Keyboard\Keyboard_EPOS\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    16-July-2020
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
#include "yc_st7789.h"
#include "yc_kscan.h"
#include "misc.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t keyvalue = 0;
uint32_t keyflag = 0x80000;

/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void NVIC_Configuration(void);
void TFT_Configuration(void);
void TIMER_Configuration(void);
void SPI_TFT_Test(void);
void key_test(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    QSPI_CTRL  |= (0x80);
    SYSCTRL_HCLK_CON = 0x0;

    UART_Configuration();
    TIMER_Configuration();
    NVIC_Configuration();
    TFT_Configuration();

    MyPrintf("Yichip Yc3121 Keyboard test Demo V1.0.\r\n");

    SPI_TFT_Test();

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
  * @brief  LCD related IO port and SPI related configuration
  * @param  None
  * @retval None
  */
void TFT_Configuration(void)
{
    #if (BOARD_VER == MPOS_BOARD_V2_1)
    MyPrintf("MPOS_BOARD_V2_1 not support Keyboard_EPOS demo\r\n");
    #else
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

    if (keyflag == 0x0001 && change == 0)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "    1   ", 32, 1, WHITE, OLIVE);
        change = 1;
    }
    if (keyflag == 0x0002 && change == 1)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "    2   ", 32, 1, WHITE, OLIVE);
        change = 2;
    }
    if (keyflag == 0x0004 && change == 2)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "    3   ", 32, 1, WHITE, OLIVE);
        change = 3;
    }
    if (keyflag == 0x0008 && change == 3)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "    4   ", 32, 1, WHITE, OLIVE);
        change = 4;
    }
    if (keyflag == 0x0010 && change == 4)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "    5   ", 32, 1, WHITE, OLIVE);
        change = 5;
    }
    if (keyflag == 0x0020 && change == 5)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "    6   ", 32, 1, WHITE, OLIVE);
        change = 6;
    }
    if (keyflag == 0x0040 && change == 6)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "    7   ", 32, 1, WHITE, OLIVE);
        change = 7;
    }
    if (keyflag == 0x0080 && change == 7)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "    8   ", 32, 1, WHITE, OLIVE);
        change = 8;
    }

    if (keyflag == 0x0100 && change == 8)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "    9   ", 32, 1, WHITE, OLIVE);
        change = 9;
    }
    if (keyflag == 0x0200 && change == 9)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "   UP   ", 32, 1, WHITE, OLIVE);
        change = 10;
    }
    if (keyflag == 0x0400 && change == 10)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "  DOWN  ", 32, 1, WHITE, OLIVE);
        change = 11;
    }
    if (keyflag == 0x0800 && change == 11)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "  MENU  ", 32, 1, WHITE, OLIVE);
        change = 12;
    }
    if (keyflag == 0x1000 && change == 12)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "   F1   ", 32, 1, WHITE, OLIVE);
        change = 13;
    }
    if (keyflag == 0x2000 && change == 13)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "   F2   ", 32, 1, WHITE, OLIVE);
        change = 14;
    }
    if (keyflag == 0x4000 && change == 14)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, " CANCEL ", 32, 1, WHITE, OLIVE);
        change = 15;
    }
    if (keyflag == 0x8000 && change == 15)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "  CLEAR ", 32, 1, WHITE, OLIVE);
        change = 16;
    }
    if (keyflag == 0x10000 && change == 16)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, " CONFIRM", 32, 1, WHITE, OLIVE);
        change = 17;
    }
    if (keyflag == 0x20000 && change == 17)
    {
        ST7789_TFT_Clear_White(96, 160, 224, 192);
        ST7789_TFT_ShowString(96, 160, 128, 16, "    0   ", 32, 1, WHITE, OLIVE);
        change = 0;
    }
}

/**
  * @brief  LCD displays test functions
  * @param  None
  * @retval None
  */
void SPI_TFT_Test(void)
{
    ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
    ST7789_TFT_ShowChinese(100, 1, Yichip_ch, 24, 5, 1, WHITE, OLIVE);
    ST7789_TFT_Draw_Piece(0, 27, TFT_COL, 28, DGRAY);
    ST7789_TFT_ShowChinese(165, 75, Welcom_ch, 36, 4, 1, WHITE, OLIVE);
    ST7789_TFT_ShowString(175, 111, 128, 16, "YC3121-C", 32, 1, WHITE, OLIVE);
    ST7789_TFT_Picture_Q(25, 80, gImage_Yichip);
    ST7789_TFT_ShowString(96, 160, 128, 16, "    0   ", 32, 1, WHITE, OLIVE);
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
            keyflag = 0x0200;
            break;
        case 40:
            keyflag = 0x0400;
            break;
        case 41:
            keyflag = 0x0800;
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
        case 45:
            keyflag = 0x8000;
            break;
        case 46:
            keyflag = 0x10000;
            break;
        case 47:
            keyflag = 0x20000;
            break;
        default:
            break;
        }
    }
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
