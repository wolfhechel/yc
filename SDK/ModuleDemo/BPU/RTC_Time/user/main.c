/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\BPU\RTC_TIME\use\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    15-July-2020
  * @brief   Yc3121 RTC TIME test program.
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
#include "yc_rtc.h"
#include "yc_lpm.h"
#include "misc.h"
#include "time.h"
#include "yc_qspi.h"
#include "board_config.h"
#include "stdlib.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define RTC_OFFSET_ADR 0x1000000+0x40000

/* Private variables ---------------------------------------------------------*/
uint8_t Timebuf[15] = {'2', '0', '2', '0', \
                       '0', '3', '1', '4', \
                       '2', '3', '5', '9', \
                       '3', '6', 0
                      };
time_t g_time_offset = 0;

/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void NVIC_Configuration(void);
void RTC_Configuration(void);
uint8_t GetDeviceTime(void);
uint8_t SetDeviceTime(void);

void UART_Cmd_Menu(void)
{
	MyPrintf("-------------RTC UART CMD----------------\n");
	MyPrintf("-------------'0':init RTC----------------\n");
	MyPrintf("-------------'1':set date and time-------\n");
	MyPrintf("-------------'2':get date and time-------\n");
	MyPrintf("example:2019-08-20 08:06:06 is:20190820080606\n");
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();
    int i = 0;
    uint8_t uart_command = 0xff;
    MyPrintf("YcChip Yc3121 RTC Demo V1.0.\n");
    UART_Cmd_Menu();
    g_time_offset =     qspi_flash_read(RTC_OFFSET_ADR, (uint8_t *)(&g_time_offset), 14);

    while (1)
    {
        if (UART_IsRXFIFONotEmpty(UART0))
        {
            uart_command = UART_ReceiveData(UART0);
        }
        switch (uart_command)
        {
        case '1':
            i = 0;
            MyPrintf("please input time year+mouth+day+hour+second ;format: 20190206125359;\n");
            while (1)
            {
                if (UART_IsRXFIFONotEmpty(UART0))
                {
                    Timebuf[i] = UART_ReceiveData(UART0);
                    if (i < 13) i++;
                    else break;
                }
            }

            SetDeviceTime();
            MyPrintf("set device time succ\n");
            uart_command = 0xff;
            break;

        case '2':
            GetDeviceTime();
            uart_command = 0xff;
            break;
        case '0':
            RTC_Configuration();
            MyPrintf("Init RTC\n");
            uart_command = 0xff;
            break;
        }
    }
}


void RTC_Configuration(void)
{
    RTC_Calibration();//RTCУ׼
    RTC_SetRefRegister(0);
    RTC_Config(ENABLE);
}

/**
  * @brief  Serial port initialization function.
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


/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/

uint8_t SetDeviceTime(void)
{
    char bufYear[5] = {0};
    char bufMonth[3] = {0};
    char bufDay[3] = {0};
    char bufHour[3] = {0};
    char bufMin[3] = {0};
    char bufSec[3] = {0};
    short hour, min, sec, year, month, day;
    struct tm bTime;

    memcpy(bufYear, Timebuf, 4);
    memcpy(bufMonth, Timebuf + 4, 2);
    memcpy(bufDay, Timebuf + 6, 2);
    memcpy(bufHour, Timebuf + 8, 2);
    memcpy(bufMin, Timebuf + 10, 2);
    memcpy(bufSec, Timebuf + 12, 2);

    year = atoi((const char *)bufYear);
    month = atoi((const char *)bufMonth);
    day = atoi((const char *)bufDay);
    hour = atoi((const char *)bufHour);
    min = atoi((const char *)bufMin);
    sec = atoi((const char *)bufSec);

    bTime.tm_year = year - 1900;
    bTime.tm_mon = month - 1;
    bTime.tm_mday = day;

    bTime.tm_hour = hour;
    bTime.tm_min = min;
    bTime.tm_sec = sec;


    RTC_SetRefRegister(0);
    g_time_offset = mktime(&bTime) - RTC_GetRefRegister();

    qspi_flash_sectorerase(RTC_OFFSET_ADR);
    qspi_flash_write(RTC_OFFSET_ADR, (uint8_t *)(&g_time_offset), 4);
    return 0 ;
}


uint8_t Redata[100] = {0};
unsigned short  VarLen = 0;



uint8_t GetDeviceTime(void)
{
    char buf[16] = {0};
    time_t now = 0;
    struct tm *pTime;

    now = RTC_GetRefRegister();

    qspi_flash_read(RTC_OFFSET_ADR, (uint8_t *)(&g_time_offset), 4);

    now = now  + g_time_offset;
    pTime = localtime(&now);
    MyPrintf("%04d-%02d-%02d %02d:%02d:%02d", pTime->tm_year + 1900, pTime->tm_mon + 1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
    return FALSE;
}
