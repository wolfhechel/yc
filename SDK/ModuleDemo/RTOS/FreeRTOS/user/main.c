/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\RTOS\FreeRTOS\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    16-August-2021
  * @brief   FreeRTOS test program.
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
#include <string.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "platform.h"
#include "yc_gpio.h"
#include "yc_systick.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define POS_MANAGE_PRIORITY             (tskIDLE_PRIORITY + 1)
#define POS_MANAGE_STACK_SIZE           (3 * 1024)//(12 * 1024 / 4) //(3 * 1024)
#define TRACE()		MyPrintf("%s:%d runs\n", __FUNCTION__, __LINE__)
#define BUG()   \
    do {        \
        portDISABLE_INTERRUPTS();       \
        MyPrintf("BUG: failure at %s:%d in %s()!\n", __FILE__, __LINE__, __FUNCTION__); \
    } while (0)
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TaskHandle_t pos_manage_task_id = NULL;
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

    MyPrintf("FreeRTOS 9.0.0\n");
}
/**
  * @brief  hardware init
  * @param  None
  * @retval None
  */
static int hardware_init(void)
{
    system_dev_init();

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
			vTaskDelay(HZ);
	}
    /* no reach here */
    BUG();
    vTaskSuspend(NULL);
}
/**
  * @brief  vtask1
  * @param  None
  * @retval None
  */
static void vtask1(void *pvParameters)
{
    int i = 0;
    (void)pvParameters;

	TRACE();

	while (1)
    {
		MyPrintf("vtask1 %d\n", i++);
		vTaskDelay(HZ);
		}

    /* no reach here */
    BUG();
    vTaskSuspend(NULL);
}
/**
  * @brief  init the heap size
  * @param  None
  * @retval None
  */
static void  prvInitialiseHeap(void)
{
    /* The Windows demo could create one large heap region, in which case it would
    be appropriate to use heap_4.  However, purely for demonstration purposes,
    heap_5 is used instead, so start by defining some heap regions.  No
    initialisation is required when any other heap implementation is used.  See
    http://www.freertos.org/a00111.html for more information.

    The xHeapRegions structure requires the regions to be defined in start address
    order, so this just creates one big array, then populates the structure with
    offsets into the array - with gaps in between and messy alignment just for test
    purposes. */
	extern unsigned long  __heap_base[], Heap_Size[];

    const HeapRegion_t xHeapRegions[] = {
        /* Start address        Size */
        { (uint8_t *)__heap_base, (size_t)(0x2f000-(uint32_t)__heap_base)},
        { NULL, 0 }
    };

    vPortDefineHeapRegions(xHeapRegions);
}
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    hardware_init();
    //usb_main();
    /* This demo uses heap_5.c, so start by defining some heap regions.  heap_5
    is only used for test and example reasons.  Heap_4 is more appropriate.  See
    http://www.freertos.org/a00111.html for an explanation. */
	TRACE();
    prvInitialiseHeap();
    configASSERT(POS_MANAGE_PRIORITY < configTIMER_TASK_PRIORITY);
    /* Create main task for install driver and system control. */
	TRACE();
    xTaskCreate(main_task,
                "POS-MANAGE",
                POS_MANAGE_STACK_SIZE,
                NULL,
                POS_MANAGE_PRIORITY,
                &pos_manage_task_id);
    xTaskCreate(vtask1,
                "task1_func",
                POS_MANAGE_STACK_SIZE,
                NULL,
                POS_MANAGE_PRIORITY,
                &pos_manage_task_id);
    /* Start the tasks and timer running. */
	TRACE();
    vTaskStartScheduler();
    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
	TRACE();
    MyPrintf("main run end!!\n");
    return -1;
}

typedef struct {
    unsigned long   r0;
    unsigned long   r1;
    unsigned long   r2;
    unsigned long   r3;
    unsigned long   r12;
    unsigned long   lr;
    unsigned long   pc;
    unsigned long   xpsr;
} ExcpReg_t;

typedef struct {
    union
	{
        unsigned long   UBM;
        struct {
            uint8_t     MMSR;   /* MemManage fault 状态寄存器 */
            uint8_t     BFSR;   /* 总线fault 状态寄存器       */
            uint16_t    UFSR;   /* 用法fault 状态寄存器       */
        } SR;
    } FaultSR;
    unsigned long   HFSR;       /* 硬fault 状态寄存器         */
    unsigned long   DFSR;       /* 调试fault 状态寄存器       */
    unsigned long   AFSR;       /* 辅助fault 状态寄存器       */
    unsigned long   MMAR;       /* MemManage fault 地址寄存器 */
    unsigned long   BFAR;       /* 总线fault 地址寄存器       */

    ExcpReg_t       excp;       /* 异常时候CPU自动压栈的寄存器 */
    unsigned long   SP;         /* 异常前的SP */
    unsigned long   IP;         /* 异常前的IP */
    unsigned long   CONTROL;    /* 异常前的control寄存器 */
} FaultReg_t;

FaultReg_t theFault;
void HARD_FAULT_IRQHandler(void)
{
		TRACE();
		while(1);
}

void App_HardFault_ISR(FaultReg_t *theFault)
{
	MyPrintf("r0=%x  \n",theFault->excp.r0);
	MyPrintf("r1=%x  \n",theFault->excp.r1);
	MyPrintf("r2=%x  \n",theFault->excp.r2);
	MyPrintf("r3=%x  \n",theFault->excp.r3);
	MyPrintf("r12=%x \n",theFault->excp.r12);
	MyPrintf("lr=%x  \n",theFault->excp.lr);
	MyPrintf("pc=%x  \n",theFault->excp.pc);
	MyPrintf("xpsr=%x\n",theFault->excp.xpsr);
}