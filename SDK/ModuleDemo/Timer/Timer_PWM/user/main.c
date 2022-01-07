/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\Timer\Timer_PWM\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    21-July-2020
  * @brief   Timer PWM test program.
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
#include "misc.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void PWM_Configuration(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();

    MyPrintf("YiChip Yc3121 PWM 5KHz Demo V1.0.\r\n");

    PWM_Configuration();

    while (1)
    {

    }
}

/**
  * @brief  Serial port 0 initialization function.
  * @param  UART_TypeDef
  * @retval None
  */
void UART_Configuration(void)
{
    UART_InitTypeDef UART_InitStruct;

    /* Configure serial ports 0 RX and TX for IO. */
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

void PWM_Configuration(void)
{
    PWM_InitTypeDef PWM_init_struct;

    PWM_init_struct.TIMx = TIM0;
    PWM_init_struct.LowLevelPeriod = 4800;
    PWM_init_struct.HighLevelPeriod = 4800;
    PWM_init_struct.SatrtLevel = OutputLow;
    GPIO_Config(GPIOA, GPIO_Pin_11, GPCFG_PWM_OUT0);
    TIM_PWMInit(&PWM_init_struct);

    /* Configure PWM for counting mode */
    TIM_ModeConfig(TIM0, TIM_Mode_PWM);

    /* The last step must be enabled */
    TIM_Cmd(TIM0, ENABLE);
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/