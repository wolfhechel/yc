/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\UART\UART_TX_Sendbuf\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    15-July-2019
  * @brief   UART TX SendBuf test program.
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
#include "yc_systick.h"
#include "yc_qspi.h"
#include "yc_encflash_bulk.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void TIMER_Configuration(void);
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

uint8_t tx_buffer_boot[32] = {0};

int main(void)
{
	UART_Configuration();
	MyPrintf("YC3121 OTA APP Demo\n");
    SysTick_Config(CPU_MHZ/100);//10ms
    TIMER_Configuration();
	MyPrintf("/*************************/\n");
	MyPrintf("Please input 'B' or 'U'\n");
	MyPrintf("B: BLE OTA Updata\n");
	MyPrintf("U: USB HID OTA Updata\n");
    MyPrintf("/************************/\n");
	uint8_t uartretval;
	while (1)
	{
        if(UART_IsRXFIFONotEmpty(UART0))
        {
            uartretval = UART_ReceiveData(UART0);
            if(uartretval == 'U')
            {
                MyPrintf("This is USB OTA Update Init\n");
                enc_erase_flash_32byte(0x1000120);
                tx_buffer_boot[0] = 0xAA;
                tx_buffer_boot[1] = 0x55;
                tx_buffer_boot[2] = 0xAA;
                tx_buffer_boot[3] = 0x55;

                for (int i = 4; i < 32; i++)
                {
                    tx_buffer_boot[i] = 0xFF;
                }
                enc_write_flash(0x1000120, tx_buffer_boot, 32);
                soft_reset();
            }
            else if(uartretval == 'B')
            {
                MyPrintf("This is BLE OTA Update Init\n");
                enc_erase_flash_32byte(0x1000120);
                tx_buffer_boot[0] = 0xA5;
                tx_buffer_boot[1] = 0x5A;
                tx_buffer_boot[2] = 0xA5;
                tx_buffer_boot[3] = 0x5A;
                for (int i = 4; i < 32; i++)
                {
                    tx_buffer_boot[i] = 0xFF;
                }
                enc_write_flash(0x1000120, tx_buffer_boot, 32);
                soft_reset();
            }
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

void TIMER_Configuration(void)
{
    TIM_InitTypeDef TIM_struct;
    TIM_struct.TIMx=TIM0;
    TIM_struct.period=CPU_MHZ;
    TIM_Init(&TIM_struct);
    TIM_Cmd(TIM_struct.TIMx, ENABLE);
    NVIC_EnableIRQ(TIM0_IRQn);
}

extern tick SystickCount;
void SYSTICK_IRQHandler(void)
{
    SystickCount++;
    if(SystickCount>=TICK_MAX_VALUE)
    {
        SystickCount=0;
    }
}

void TIMER0_IRQHandler(void)
{
    MyPrintf("TIMER0_IRQHandler irq[SystickCount=%d]\r\n",SystickCount);
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
