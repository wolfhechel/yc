/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\ADC\ADC_gpio_mode\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    14-July-2020
  * @brief   ADC gpio mode program.
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
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void ADC_Configuration(ADC_ScaleTypeDef scale);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();

    MyPrintf("Yichip Yc3121 ADC gpio mode Demo V1.0.\r\n");
    MyPrintf("1: ADC GPIO1.2v MODE !\n");
    MyPrintf("2: ADC GPIO2.4v MODE !\n");
    uint8_t uartretval = 0;
    while (1)
    {
        if(UART_IsRXFIFONotEmpty(UART0))
        {
            uartretval =  UART_ReceiveData(UART0);
            switch (uartretval)
            {
            case '1':
                MyPrintf("GPIO1.2v test\r\n");
                ADC_Configuration(ADC_Scale_1200Mv);
                MyPrintf("ADC_Result: %d mV\n", ADC_GetVoltage(ADC_CHANNEL_2));
                break;
            case '2':
                MyPrintf("GPIO2.4v test\r\n");
                ADC_Configuration(ADC_Scale_2400Mv);
                MyPrintf("ADC_Result: %d mV\n", ADC_GetVoltage(ADC_CHANNEL_2));
                break;
            default:
                break;
            }
            uartretval = 0;
        }
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
  * @brief  ADC initialization function.
  * @param  None
  * @retval None
  */
void ADC_Configuration(ADC_ScaleTypeDef scale)
{
    GPIO_Config(ADC_IO_PORT, ADC2_IO_PIN, ANALOG);  //ADC需要使用特定IO使用
    ADC_InitTypeDef ADCInitStruct;
    ADCInitStruct.ADC_Channel = ADC_CHANNEL_2;
    ADCInitStruct.ADC_Mode = ADC_GPIO;
    ADCInitStruct.ADC_Scale = scale;//只在GPIO模式下有效
    ADC_Init(&ADCInitStruct);
    MyPrintf("testing channel %d\n", ADCInitStruct.ADC_Channel);
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
