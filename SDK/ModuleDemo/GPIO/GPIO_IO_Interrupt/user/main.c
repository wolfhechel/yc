/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\GPIO\GPIO_IO_Interrupt\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    15-July-2020
  * @brief   GPIO Interrupt program.
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
#include "yc_exti.h"
#include "misc.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t buf[10];

/* Private function prototypes -----------------------------------------------*/
void NVIC_Configuration(void);
void UART_Configuration(void);
void GPIO_Configuration(void);
uint8_t Get_cmd(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    uint8_t tmpCmd;

    UART_Configuration();
    NVIC_Configuration();

    MyPrintf("Yichip Yc3121 GPIO Interrupt Demo V1.0.\n");

    MyPrintf("====================================\n");
    MyPrintf(" case1: EXTI_Trigger_Rising!\n");
    MyPrintf(" case2: EXTI_Trigger_Falling!\n");
    MyPrintf(" case3: EXTI_Trigger_Rising_Falling!\n");
    MyPrintf("====================================\n");

    while (1)
    {
        tmpCmd = Get_cmd();

        if (tmpCmd)
        {
            switch (tmpCmd)
            {
            case 1:
                EXTI_LineConfig(EXTI_PORT, EXTI_PIN, EXTI_Trigger_Rising);
                MyPrintf(" case1: EXTI_Trigger_Rising!\n");
                break;
            case 2:
                EXTI_LineConfig(EXTI_PORT, EXTI_PIN, EXTI_Trigger_Falling);
                MyPrintf(" case2: EXTI_Trigger_Falling!\n");
                break;
            case 3:
                EXTI_LineConfig(EXTI_PORT, EXTI_PIN, EXTI_Trigger_Rising_Falling);
                MyPrintf(" case3: EXTI_Trigger_Rising_Falling!\n");
                break;
            default:
                break;
            }
        }
    }
}

/**
  * @brief  NVIC configuration function.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
    NVIC_EnableIRQ(UART0_IRQn);
    NVIC_SetPriority(UART0_IRQn, 0);

    NVIC_EnableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 1);
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
    UART_ITConfig(UART0, UART_IT_RX, ENABLE);
    UART_SetRxITNum(UART0, 10);
    UART_SetITTimeout(UART0, 0xff);
}

/**
  * @brief  Get serial command.
  * @param  None
  * @retval None
  */
uint8_t Get_cmd(void)
{
    uint8_t tmp = 0;

    if (buf[0] == 'c' && buf[1] == 'a' && buf[2] == 's' && buf[3] == 'e')
    {
        switch (buf[4])
        {
        case '1':
            tmp = 1;
            buf[0] = '\0';
            break;
        case '2':
            tmp = 2;
            buf[0] = '\0';
            break;
        case '3':
            tmp = 3;
            buf[0] = '\0';
            break;
        default :
            tmp = 0;
            break;
        }
    }

    return tmp;
}

/**
  * @brief  UART0 interrupt service function.
  * @param  None
  * @retval None
  */
void UART0_IRQHandler(void)
{
    if (UART_IsRXFIFONotEmpty(UART0))
    {
        UART_RecvBuf(UART0, buf, 9);
    }
}

/**
  * @brief  GPIOC interrupt service function.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{
    MyPrintf("EXTI0_IRQHandler In\n");
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
