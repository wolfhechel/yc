/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\POWER\POWER_off\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    21-July-2020
  * @brief   POWER off program.
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
#include "yc_power.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t num = 0;

/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void BEEP_Toggle(void);
void BEEP_Off(void);
void Power_off_test(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();

    MyPrintf("Yichip Yc3121 POWER off Demo V1.0.\r\n");

    while (1)
    {
        BEEP_Toggle();
        Power_off_test();
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

void BEEP_Toggle(void)
{
    num++;
    if (num == 0x1ffff)
    {
        num = 0;
        if (GPIO_ReadInputDataBit(BEEP_PORT, BEEP_PIN))
            GPIO_ResetBits(BEEP_PORT, BEEP_PIN);
        else
            GPIO_SetBits(BEEP_PORT, BEEP_PIN);
    }
}

void BEEP_Off(void)
{
    GPIO_Config(BEEP_PORT, BEEP_PIN, OUTPUT_LOW);
}

void Power_off_test(void)
{
    uint8_t ret;

    if (TRUE == Power_Keyscan())
    {
        /* 如果power_key需要做独立按键可进行长短按设计 */
        delay_ms(1000);
        if (TRUE == Power_Keyscan())
        {
            BEEP_Off();
            MyPrintf("poweroff\n");
            /* 操作Power_off函数之前先进行关闭LCD等外设的操作 */
            ret = Power_off(); //如果插入外电，则不会关机，返回FALSE
        }
    }
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
