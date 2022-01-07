/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\Timer\Timer_Interrupt\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    21-July-2020
  * @brief   Timer Interrupt test program.
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
volatile  uint32_t tick_Timer[9] = {0};

/* Private function prototypes -----------------------------------------------*/
void NVIC_Configuration(void);
void UART_Configuration(void);
void TIMER_Configuration(void);
uint32_t TIMM0_GetTick(uint32_t TIMx);
void TIMM0_Mdelay(uint32_t TIMx, uint32_t delay);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    NVIC_Configuration();
    UART_Configuration();
    TIMER_Configuration();

    MyPrintf("YiChip Yc3121 Timer Demo V1.0.\r\n");

    while (1)
    {
        TIMM0_Mdelay(TIM0, 1000);
        MyPrintf("current Timer_%d_GetTick tick = %d \n", TIM0, TIMM0_GetTick(TIM0));
        TIMM0_Mdelay(TIM1, 1000);
        MyPrintf("current Timer_%d_GetTick tick = %d \n", TIM1, TIMM0_GetTick(TIM1));
        TIMM0_Mdelay(TIM2, 1000);
        MyPrintf("current Timer_%d_GetTick tick = %d \n", TIM2, TIMM0_GetTick(TIM2));
        TIMM0_Mdelay(TIM3, 1000);
        MyPrintf("current Timer_%d_GetTick tick = %d \n", TIM3, TIMM0_GetTick(TIM3));
        TIMM0_Mdelay(TIM4, 1000);
        MyPrintf("current Timer_%d_GetTick tick = %d \n", TIM4, TIMM0_GetTick(TIM4));
        TIMM0_Mdelay(TIM5, 1000);
        MyPrintf("current Timer_%d_GetTick tick = %d \n", TIM5, TIMM0_GetTick(TIM5));
        TIMM0_Mdelay(TIM6, 1000);
        MyPrintf("current Timer_%d_GetTick tick = %d \n", TIM6, TIMM0_GetTick(TIM6));
        TIMM0_Mdelay(TIM7, 1000);
        MyPrintf("current Timer_%d_GetTick tick = %d \n", TIM7, TIMM0_GetTick(TIM7));
        TIMM0_Mdelay(TIM8, 1000);
        MyPrintf("current Timer_%d_GetTick tick = %d \n", TIM8, TIMM0_GetTick(TIM8));
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

    NVIC_EnableIRQ(TIM1_IRQn);
    NVIC_SetPriority(TIM1_IRQn, 0);

    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, 0);

    NVIC_EnableIRQ(TIM3_IRQn);
    NVIC_SetPriority(TIM3_IRQn, 0);

    NVIC_EnableIRQ(TIM4_IRQn);
    NVIC_SetPriority(TIM4_IRQn, 0);

    NVIC_EnableIRQ(TIM5_IRQn);
    NVIC_SetPriority(TIM5_IRQn, 0);

    NVIC_EnableIRQ(TIM6_IRQn);
    NVIC_SetPriority(TIM6_IRQn, 0);

    NVIC_EnableIRQ(TIM7_IRQn);
    NVIC_SetPriority(TIM7_IRQn, 0);

    NVIC_EnableIRQ(TIM8_IRQn);
    NVIC_SetPriority(TIM8_IRQn, 0);
}

/**
  * @brief  Serial port 0 initialization function.
  * @param  None
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

/**
  * @brief  TIM configuration function.
  * @param  None
  * @retval None
  */
void TIMER_Configuration(void)
{
    TIM_InitTypeDef TIM_InitStruct;

    TIM_InitStruct.period = 48000;

    TIM_InitStruct.TIMx = TIM0;
    TIM_Init(&TIM_InitStruct);

    TIM_InitStruct.TIMx = TIM1;
    TIM_Init(&TIM_InitStruct);

    TIM_InitStruct.TIMx = TIM2;
    TIM_Init(&TIM_InitStruct);

    TIM_InitStruct.TIMx = TIM3;
    TIM_Init(&TIM_InitStruct);

    TIM_InitStruct.TIMx = TIM4;
    TIM_Init(&TIM_InitStruct);

    TIM_InitStruct.TIMx = TIM5;
    TIM_Init(&TIM_InitStruct);

    TIM_InitStruct.TIMx = TIM6;
    TIM_Init(&TIM_InitStruct);

    TIM_InitStruct.TIMx = TIM7;
    TIM_Init(&TIM_InitStruct);

    TIM_InitStruct.TIMx = TIM8;
    TIM_Init(&TIM_InitStruct);

    /* Configure timer for counting mode */
    TIM_ModeConfig(TIM0, TIM_Mode_TIMER);
    TIM_ModeConfig(TIM1, TIM_Mode_TIMER);
    TIM_ModeConfig(TIM2, TIM_Mode_TIMER);
    TIM_ModeConfig(TIM3, TIM_Mode_TIMER);
    TIM_ModeConfig(TIM4, TIM_Mode_TIMER);
    TIM_ModeConfig(TIM5, TIM_Mode_TIMER);
    TIM_ModeConfig(TIM6, TIM_Mode_TIMER);
    TIM_ModeConfig(TIM7, TIM_Mode_TIMER);
    TIM_ModeConfig(TIM8, TIM_Mode_TIMER);

    /* The last step must be enabled */
    TIM_Cmd(TIM0, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
    TIM_Cmd(TIM5, ENABLE);
    TIM_Cmd(TIM6, ENABLE);
    TIM_Cmd(TIM7, ENABLE);
    TIM_Cmd(TIM8, ENABLE);
}

uint32_t TIMM0_GetTick(uint32_t TIMx)
{
    return tick_Timer[TIMx];
}

void TIMM0_Mdelay(uint32_t TIMx, uint32_t delay)
{
    uint32_t tick = tick_Timer[TIMx];
    TIM_Cmd((TIM_NumTypeDef)TIMx, ENABLE);
    if ((tick + delay) < tick_Timer[TIMx])
    {
        while ((tick_Timer[TIMx] - delay) < tick);
    }
    else
    {
        while ((tick + delay) > tick_Timer[TIMx]);
    }
    TIM_Cmd((TIM_NumTypeDef)TIMx, DISABLE);
}

/**
  * @brief  TIM0~TIM8 Interrupt service function.
  * @param  None
  * @retval None
  */
void TIMER0_IRQHandler(void)
{
    tick_Timer[0]++;
}

void TIMER1_IRQHandler(void)
{
    tick_Timer[1]++;
}

void TIMER2_IRQHandler(void)
{
    tick_Timer[2]++;
}

void TIMER3_IRQHandler(void)
{
    tick_Timer[3]++;
}

void TIMER4_IRQHandler(void)
{
    tick_Timer[4]++;
}

void TIMER5_IRQHandler(void)
{
    tick_Timer[5]++;
}

void TIMER6_IRQHandler(void)
{
    tick_Timer[6]++;
}

void TIMER7_IRQHandler(void)
{
    tick_Timer[7]++;
}

void TIMER8_IRQHandler(void)
{
    tick_Timer[8]++;
}


/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
