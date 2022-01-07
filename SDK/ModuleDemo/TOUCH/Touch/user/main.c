/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\TOUCH\touch\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    21-July-2020
  * @brief   ADC TOUCH mode program.
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
#include "yc_adc.h"
#include "yc_timer.h"
#include "yc_touch.h"
#include "yc_kscan.h"
#include "board_config.h"


/* Private typedef -----------------------------------------------------------*/
typedef enum
{
    PAGE0 = 0,
    PAGE1,
    PAGE2,
    PAGE3
} TP_MENU_TypeDef;
/* Private define ------------------------------------------------------------*/

#define TP_X_LEFT_BORDER     (14)
#define TP_X_RIGHT_BORDER    (320 - TP_X_LEFT_BORDER)
#define TP_Y_HIGT_BORDER     (35)
#define TP_Y_LOW_BORDER      (240 - 16*3 - 5)

#define TP_CLEAR_X_LEFT      (320/2 - 5*16/2)
#define TP_CLEAR_Y_LEFT      (240 - 45)
#define TP_CLEAR_X_RIGHT     (TP_CLEAR_X_LEFT + 16*5)
#define TP_CLEAR_Y_RIGHT     (240)

#define TP_EXIT_X_LEFT      (13)
#define TP_EXIT_Y_LEFT      (240 - 45)
#define TP_EXIT_X_RIGHT     (TP_EXIT_X_LEFT + 16*4)
#define TP_EXIT_Y_RIGHT     (240)

#define TP_SAVE_X_LEFT      (320 -13-16*4)
#define TP_SAVE_Y_LEFT      (240 - 45)
#define TP_SAVE_X_RIGHT     (TP_SAVE_X_LEFT + 16*4)
#define TP_SAVE_Y_RIGHT     (240)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void UART_Configuration(void);
static void ADC_Configuration(void);
static void St7789vlcd_Configuration(void);
static void Touch_Test(void);


static void TFT_Power_scan(void)
{
    ST7789_TFT_Picture(284, 4, gImage_full_energe);
}

static void TFT_Single_scan(void)
{
    ST7789_TFT_Picture(4, 4, gImage_single_l);
}

static void Display_InterfaceWindow(void)
{
    uint8_t Show_buff1[] = "Touch demo";
    uint8_t Show_buff2[] = "Please press menu key...";

    ST7789_TFT_Clear_White(0, 0, TFT_COL, TFT_ROW);
    ST7789_TFT_ShowChinese(100, 1, Yichip_ch, 24, 5, 1, WHITE, OLIVE);
    ST7789_TFT_Draw_Piece(0, 27, TFT_COL, 28, DGRAY);
    ST7789_TFT_ShowString(70, 80, 16 * sizeof(Show_buff1), 16, Show_buff1, 32, 1, WHITE, OLIVE);
    ST7789_TFT_ShowString(10, 150, 8 * sizeof(Show_buff2), 8, Show_buff2, 16, 1, WHITE, MAROON);
    TFT_Power_scan();
    TFT_Single_scan();
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    /*enable ldo*/
    GPIO_Config(GPIOA, GPIO_Pin_2, OUTPUT_HIGH);

    UART_Configuration();

    MyPrintf("Yichip Yc3121 Touch Demo V1.0.\r\n");

    St7789vlcd_Configuration();

    Display_InterfaceWindow();

    while (1)
    {
        Touch_Test();
    }
}



static void Tp_menu(TP_MENU_TypeDef type)
{
    switch (type)
    {
    case PAGE0 :
        ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
        ST7789_TFT_ShowChinese(56, 36, calibrated_ch, 32, 7, 1, WHITE, RED);
        St7789_tft_DrawRectangle(115, 88, 205, 129, BLACK);
        ST7789_TFT_ShowChinese(136, 89, confirm_ch, 24, 2, 1, WHITE, MAROON);
        ST7789_TFT_ShowString(132, 113,  56,  8, (uint8_t *)"CONFIRM", 16, 1, WHITE, MAROON);
        St7789_tft_DrawRectangle(115, 136, 205, 177, BLACK);
        ST7789_TFT_ShowChinese(136, 137, cancel_ch, 24, 2, 1, WHITE, MAROON);
        ST7789_TFT_ShowString(136, 161,  48,  8, (uint8_t *)"CANCEL", 16, 1, WHITE, MAROON);
        St7789_tft_DrawRectangle(115, 184, 205, 225, BLACK);
        ST7789_TFT_ShowChinese(136, 185, clear_ch, 24, 2, 1, WHITE, MAROON);
        ST7789_TFT_ShowString(140, 209,  48,  8, (uint8_t *)"CLEAR", 16, 1, WHITE, MAROON);
        break;
    case PAGE1 :
        ST7789_TFT_ShowChinese(136, 89, confirm_ch, 24, 2, 0, YELLOW, MAROON);
        ST7789_TFT_ShowString(132, 113,  56,  8, (uint8_t *)"CONFIRM", 16, 0, YELLOW, MAROON);
        delay_ms(500);
        ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
        break;
    case PAGE2 :
        ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
        for (uint8_t i = 0; i < 5 ; i++)
        {
            St7789_tft_DrawRectangle(1 + i, 29 + i, 318 - i, 240 - 16 * 3 - i, BLACK);
        }

        ST7789_TFT_ShowString(320 - 13 - 16 * 4, 240 - 45, 16 * 4, 16, (uint8_t *)"Save", 32, 1, WHITE, PURPLE);
        ST7789_TFT_ShowString(13, 240 - 45, 5 * 16, 16, (uint8_t *)"Exit", 32, 1, WHITE, PURPLE);
        ST7789_TFT_ShowString(320 / 2 - 5 * 16 / 2, 240 - 45, 16 * 5, 16, (uint8_t *)"Clear", 32, 1, WHITE, PURPLE);
        break;
    case PAGE3 :
        ST7789_TFT_ShowString(320 / 2 - 5 * 16 / 2, 240 - 45, 16 * 5, 16, (uint8_t *)"Clear", 32, 1, YELLOW, MAROON);
        ST7789_TFT_Clear_White(5, 34, 313, 188);
        ST7789_TFT_ShowString(320 / 2 - 5 * 16 / 2, 240 - 45, 16 * 5, 16, (uint8_t *)"Clear", 32, 1, WHITE, PURPLE);
        break;
    }
}


static Boolean Touch_Cali(void)
{
    uint8_t cal_cnt = 3;
    uint8_t Show_buff1[] = "Calibration success";
    uint8_t Show_buff2[] = "Calibration failure please recalibrate";
    while (cal_cnt --)
    {
        if (Tp_Calibrate() == TRUE)
        {
            ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
            ST7789_TFT_ShowString(10, 80, 8 * sizeof(Show_buff1), 8, Show_buff1, 16, 1, WHITE, MAROON);
            delay_ms(2000);
            ST7789_TFT_Clear_White(0, 30, TFT_COL, TFT_ROW);
            return TRUE;
        }
        else
        {
            ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
            ST7789_TFT_ShowString(10, 80, 8 * sizeof(Show_buff2), 8, Show_buff2, 16, 1, WHITE, MAROON);
            delay_ms(2000);
            ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
        }
    }
}

static void Touch_Drow_Line(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    St7899_tft_DrawLine(sx, sy, ex, ey, BLACK);
    St7899_tft_DrawLine(sx - 1, sy, ex - 1, ey, BLACK);
    St7899_tft_DrawLine(sx, sy + 1, ex, ey + 1, BLACK);
    St7899_tft_DrawLine(sx + 1, sy, ex + 1, ey, BLACK);
    St7899_tft_DrawLine(sx, sy - 1, ex, ey - 1, BLACK);
}

void Touch_Display(void)
{
    uint8_t keyCode = 0;
    Tp_Sign_Para_TypeDef Sign_Para = {0};

    ADC_Configuration();

    /*Select whether to calibrate*/

    Tp_menu(PAGE0);
    while (1)
    {
        keyCode = KEY_Scan();
        if (keyCode == KEY_CONFIRM)
        {
            Tp_menu(PAGE1);
            Touch_Cali();
            break;
        }
        else if (keyCode == KEY_CANCEL)
        {
            /*Use the default calibration parameters*/
            Constant.kx = TP_CONSTANT_X;
            Constant.ky = TP_CONSTANT_Y;
            fac.kx = TP_FAC_KX;
            fac.ky = TP_FAC_KY;
            break;
        }
        else if (keyCode == KEY_CLEAR)
        {
            return ;
        }
    }

    /*Start the signature*/

    Tp_menu(PAGE2);

    while (1)
    {
        if (Tp_DetectPressing() == TRUE)
        {
            Tp_GetPoint(&Sign_Para.nx, &Sign_Para.ny);

            TP_PRINT("Sign_Para.nx :%d,Sign_Para.ny :%d\n", Sign_Para.nx, Sign_Para.ny);
            TP_PRINT("Sign_Para.lx :%d,Sign_Para.ly :%d\n", Sign_Para.lx, Sign_Para.ly);

            if (Tp_DetectPressing() == FALSE)
            {
                Sign_Para.pressflag = 0;
            }
            /*Continuous press*/
            if (Sign_Para.pressflag == 2)
            {
                if (Sign_Para.nx > TP_X_LEFT_BORDER && Sign_Para.nx < TP_X_RIGHT_BORDER && \
                        Sign_Para.ny > TP_Y_HIGT_BORDER && Sign_Para.ny < TP_Y_LOW_BORDER)
                {
                    if (Tp_AbsolouteDifference(Sign_Para.nx, Sign_Para.lx) < 30 && Tp_AbsolouteDifference(Sign_Para.ny, Sign_Para.ly) < 30)
                    {
                        Touch_Drow_Line(Sign_Para.nx, Sign_Para.ny, Sign_Para.lx, Sign_Para.ly);
                    }
                    Sign_Para.lx = Sign_Para.nx;
                    Sign_Para.ly = Sign_Para.ny;
                }
            }
            /*first press*/

            else if (Sign_Para.nx > TP_X_LEFT_BORDER && Sign_Para.nx < TP_X_RIGHT_BORDER && \
                     Sign_Para.ny > TP_Y_HIGT_BORDER && Sign_Para.ny < TP_Y_LOW_BORDER)
            {
                Sign_Para.pressflag ++;
                Sign_Para.lx = Sign_Para.nx;
                Sign_Para.ly = Sign_Para.ny;
            }
            /*press clear*/

            else if (Sign_Para.nx > TP_CLEAR_X_LEFT && Sign_Para.nx < TP_CLEAR_X_RIGHT && \
                     Sign_Para.ny > TP_CLEAR_Y_LEFT && Sign_Para.ny < TP_CLEAR_Y_RIGHT)
            {
                Tp_menu(PAGE3);
            }
            /*press exit*/

            else if (Sign_Para.nx > TP_EXIT_X_LEFT && Sign_Para.nx < TP_EXIT_X_RIGHT && \
                     Sign_Para.ny > TP_EXIT_Y_LEFT && Sign_Para.ny < TP_EXIT_Y_RIGHT)
            {
                return ;
            }

            /*Save exit*/
            else if (Sign_Para.nx > TP_SAVE_X_LEFT && Sign_Para.nx < TP_SAVE_X_RIGHT && \
                     Sign_Para.ny > TP_SAVE_Y_LEFT && Sign_Para.ny < TP_SAVE_Y_RIGHT)
            {
                return ;
            }
        }
        else
        {
            /*no press*/
            Sign_Para.pressflag = 0;
        }
    }
}

static void Touch_Test(void)
{
    uint8_t keyCode = 0;
    uint8_t Show_buff2[] = "Whether or not to continue";
    while (keyCode != KEY_MENU)
    {
        keyCode = KEY_Scan();
    }
    while (1)
    {
        Touch_Display();
        ST7789_TFT_Clear_White(0, 0, TFT_COL, TFT_ROW);
        ST7789_TFT_ShowChinese(100, 1, Yichip_ch, 24, 5, 1, WHITE, OLIVE);
        ST7789_TFT_Draw_Piece(0, 27, TFT_COL, 28, DGRAY);
        TFT_Power_scan();
        TFT_Single_scan();
        ST7789_TFT_ShowString(10, 80, 8 * sizeof(Show_buff2), 8, Show_buff2, 16, 1, WHITE, MAROON);
        ST7789_TFT_ShowString(0, 180, 20 * 16,  16, (uint8_t *)"CANCEL       CONFIRM", 32, 1, WHITE, OLIVE);
        while (1)
        {
            keyCode = KEY_Scan();
            if (keyCode == KEY_CANCEL)
            {
                Display_InterfaceWindow();
                return;
            }
            else if (keyCode == KEY_CONFIRM)
            {
                break;
            }
        }
    }
}

/**
  * @brief  Serial port 0 initialization function.
  * @param  None
  * @retval None
  */
static void UART_Configuration(void)
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
    UART_InitStruct.BaudRate = UARTBAUD;            //Configure serial port baud rate, the baud rate defaults to 128000.
    UART_InitStruct.DataBits = Databits_8b;
    UART_InitStruct.StopBits = StopBits_2;
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

static void St7789vlcd_Configuration(void)
{
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

    /* Initialize the LCD screen */
    ST7789_TFT_Init();
}
/**
  * @brief  ADC initialization function.
  * @param  None
  * @retval None
  */
static void ADC_Configuration(void)
{
    ADC_InitTypeDef ADCInitStruct;
    ADCInitStruct.ADC_Mode = ADC_GPIO;
    ADCInitStruct.ADC_Channel = ADC_CHANNEL_6;
    ADC_Init(&ADCInitStruct);
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
