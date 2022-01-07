/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\RTOS\UCOS\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    16-August-2021
  * @brief   UCOS test program.
  ******************************************************************************
  * @attention
  *
  * COPYRIGHT 2021 Yichip Microelectronics
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
#include "includes.h"
#include "yc3121.h"
#include "yc_gpio.h"
#include "yc_systick.h"
#include "yc_sysctrl.h"
#include "misc.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define  MAIN_TASK_PRIO            4
#define  MAIN_TASK_STK_SIZE        1000
#define SYS_CLK_1S                 96000000
#define TRACE()		MyPrintf("%s:%d runs\n", __FUNCTION__, __LINE__)
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static OS_STK  StartTaskStk[MAIN_TASK_STK_SIZE];
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  system dev init
  * @param  None
  * @retval None
  */
static void system_dev_init(void)
{
    UART_InitTypeDef UART_InitStruct;

    /* Configure serial ports RX and TX for IO. */
    GPIO_Config(GPIOA, GPIO_Pin_1, UART0_TXD);
    GPIO_Config(GPIOA, GPIO_Pin_0, UART0_RXD);

    UART_InitStruct.BaudRate = 921600;            //Configure serial port baud rate, the baud rate defaults to 128000.
    UART_InitStruct.DataBits = Databits_8b;
    UART_InitStruct.StopBits = StopBits_1;
    UART_InitStruct.Parity   = Parity_None;
    UART_InitStruct.FlowCtrl = FlowCtrl_None;
    UART_InitStruct.Mode     = Mode_duplex;

    UART_Init(UART0, &UART_InitStruct);
    MyPrintf("UCOS II\n");
}
/**
  * @brief  hardware init
  * @param  None
  * @retval None
  */
static int hardware_init(void)
{
    system_dev_init();
    SYSCTRL_HCLKConfig(SYSCTRL_HCLK_Div_None);
    return 0;
}
/**
  * @brief  main task
  * @param  None
  * @retval None
  */
static void main_task(void *pvParameters)
{
	int i = 0;
    (void)pvParameters;

	TRACE();

	while (1)
    {
		MyPrintf("main_task %d\n", i++);
        OSTimeDly(1000);
	}
}
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    uint8_t err;
    
    hardware_init();
    MyPrintf("YC3121 UCOS II DEMO!!!\n");

    SysTick_Config(SYS_CLK_1S/OS_TICKS_PER_SEC);
    
    OSInit();
    
    OSTaskCreateExt(main_task,
                    (void *)0,
                    (OS_STK *)&StartTaskStk[MAIN_TASK_STK_SIZE - 1],
                    MAIN_TASK_PRIO,
                    MAIN_TASK_PRIO,
                    (OS_STK *)&StartTaskStk[0],
                    MAIN_TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    OSStart();

	TRACE();
    MyPrintf("main run end!!\n");
    return -1;
}
/**
  * @brief  HARD FAULT IRQHandler
  * @param  None
  * @retval None
  */
void HARD_FAULT_IRQHandler(void)
{
    TRACE();
	while(1);
}