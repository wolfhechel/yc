/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\BPU\RTC\use\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    19-Feb-2019
  * @brief   Yc3121 RTC interrupt test program.
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
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ONE_MINUTE 60

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void NVIC_Configuration(void);
void RTC_Configuration(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    NVIC_Configuration();
    UART_Configuration();

    MyPrintf("YcChip Yc3121 RTC Demo V1.0.\n");

    RTC_Configuration();

    while (1)
    {

    }
}

void NVIC_Configuration(void)
{
    NVIC_EnableIRQ(SEC_IRQn);
    NVIC_SetPriority(SEC_IRQn, 0);
}

void RTC_Configuration(void)
{
    uint32_t i = 0xffff;
    while (i--);

    MyPrintf("32k RtcCVal: 0x%x\n", RTC_Get_SexMax());

    RTC_Calibration();
    RTC_SetWakeUpCounter(5);
    RTC_SetRefRegister(0);

    RTC_Config(ENABLE);
    RTC_ClearITPendingBit();
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

/**
  * @brief  Watchdog interrupt service function.
  * @param  None
  * @retval None
  */
void SEC_IRQHandler(void)
{
    static uint32_t timer = 0;
    timer = RTC_GetRefRegister();
    MyPrintf("%d second.\n", timer);
    timer += 5;
    RTC_SetWakeUpCounter(timer);
    RTC_Calibration();
    RTC_ClearITPendingBit();
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
