/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\POS\pos_demo\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    24-Mar-2019
  * @brief   SPI for POS demo program.
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
#include "yc_st7789.h"
#include "yc_timer.h"
#include "yc_kscan.h"
#include "yc_systick.h"
#include "yc_power.h"
#include "yc_emv_contactless_l1.h"
#include "yc_nfc_common.h"
#include "font.h"
#include "meun.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void SysTick_Configuration(void);
extern void BEEP_End();

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	SYSCTRL_HCLKConfig(SYSCTRL_HCLK_Div_None);
	#if (BOARD_VER == EPOS_BOARD_V1_0)
	GPIO_Config(GPIOA, GPIO_Pin_2, OUTPUT_HIGH);//External LDO enable
	#endif
	UART_Configuration();
#if BEEP_PWM
	BEEP_PWM_Init();
#else
	BEEP_End();
#endif
	SysTick_Configuration();
	MyPrintf("Yichip Yc3121 POS Demo V1.0.\r\n");
	MyPrintf("build date:%s-%s\r\n",__DATE__,__TIME__);
	TFT_SPI_Init();
	#if (VERSIONS == EXIST_BT)
	app_bt_init();
	#endif
	Display_InterfaceWindow();

	while(1)
	{
		#if (VERSIONS == EXIST_BT)
		BT_Progress();
		#endif
		MeunState_Check_Update();
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
  - BaudRate = baud
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
  * @brief  SysTick initialization function.
  * @param  None
  * @retval None
  */
void SysTick_Configuration(void)
{
	SysTick_Config(CPU_MHZ/100);//10ms
}


/**
  * @brief  Systick interrupt service function.
  * @param  None
  * @retval None
  */
extern tick SystickCount;
void SYSTICK_IRQHandler(void)
{
	static uint8_t off_cnt = 0;
	
	if (off_cnt == 100)
	{
		off_cnt = 0;
		if (TRUE == Power_Keyscan())
		{
			ST7789_TFT_Clear_White(0, 0, TFT_COL, TFT_ROW);
			ST7789_TFT_ShowString(64, 104,  192,  16, "POWER OFF...", 32, 1, WHITE, OLIVE);
			MyPrintf("poweroff\n");
			delay_ms(1000);
			#if (BOARD_VER == MPOS_BOARD_V2_1)
			MyPrintf("MPOS_BOARD_V2_1 not support Keyboard_EPOS demo\r\n");
			#else
			GPIO_Config(ST7789_TFT_BL_PORT, ST7789_TFT_BL_PIN, ST7789_TFT_BL_HIGH_LIGHT?OUTPUT_LOW:OUTPUT_HIGH);
			#endif
			Power_off();
		}
	}
	off_cnt++;
	
	SystickCount++;
	if(SystickCount>=TICK_MAX_VALUE)	SystickCount=0;
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
