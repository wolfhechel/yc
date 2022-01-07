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
#include "yc_ipc.h"
#include "yc_lpm.h"
#include "bt_code.h"
#include "yc_otp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define uartBaud 115200

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);

/**
  * @brief  Level inversion function.
  * @param  None
  * @retval None
  */
void GPIO_Reverse(GPIO_TypeDef GPIOx, uint16_t GPIO_Pin)
{
	_ASSERT(ISGPIOGROUP(GPIOx));
	_ASSERT(IS_GET_GPIO_PIN(GPIO_Pin));
	if (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin))
		GPIO_ResetBits(GPIOx, GPIO_Pin);
	else
		GPIO_SetBits(GPIOx, GPIO_Pin);
}

/**
  * @brief  Configure GPIO high and low level conversion.
  * @param  None
  * @retval None
  */
void led_run(void)
{
	static uint32_t times = 0;
	times++;
	
	/* Wait a short time and flip the GPIO level */
	if (times>0x50000)
	{
		times = 0;
		
		/* When GPIO2 is high, set it as low level and vice versa. */
		GPIO_Reverse(GPIOB, GPIO_Pin_6);
	} 
}
#define UART_DEBUG

#define IPC_RX_EVENT		0x02
#define IPC_EVENT_LE_CONN		0x02
#define IPC_EVENT_LE_DISCONN	0x05
#define IPC_EVENT_SET_CMD_RSP	0x06
#define IPC_EVENT_STANDBY		0x09
#define IPC_EVENT_LE_RECEIVE_DATA	0x08

#define IPC_CMD_SET_BT_ADDR	0x00
#define IPC_CMD_SET_BLE_ADDR	0x01
#define IPC_CMD_SET_VISIBILITY	0x02

uint8_t IpcReadData(void)
{
	MyPrintf("ipc read test\n");
	HCI_TypeDef IpcRxData;
	uint8_t buf[255];
	IpcRxData.p_data = buf;
	if(IPC_ReadBtData(&IpcRxData) == SUCCESS)
	{
		if(IpcRxData.type == IPC_RX_EVENT)
		{
			MyPrintf("Ipc Receive data \n");
		}
		else
		{
			MyPrintf("Ipc Receive error data \n");
			return ERROR;
		}
		switch(IpcRxData.opcode)
		{
			case IPC_EVENT_STANDBY:
				MyPrintf("BT chip standby\n");
				//IpcSetBtMacAddr(&BleAddr);
				break;
			case IPC_EVENT_LE_CONN:
				MyPrintf("BLE CONNECT\n");
				break;
			case IPC_EVENT_LE_DISCONN:
				MyPrintf("BLE DISCONNECT\n");
				break;
			case IPC_EVENT_LE_RECEIVE_DATA:
				MyPrintf("BLE RECEIVE DATA:");
				for(int i=0;i<IpcRxData.DataLen;i++)
				{
					MyPrintf("%02x",IpcRxData.p_data[i]);
				}
				MyPrintf("\n");
				break;
			default:
				MyPrintf("error opcode is %02x\n", IpcRxData.opcode);
				break;
		}
		return SUCCESS;
	}
	else
	{
		return ERROR;
	}
	
}

uint8_t BT_Wake = 0;
void BT_IRQHandler(void)
{
#ifdef UART_DEBUG
		MyPrintf("\nBt Irq Handler=%x \n",BT_CONFIG);
#endif	
	if(BT_Wake == 1)
	{
		#ifdef UART_DEBUG
			while(IpcReadData() == SUCCESS);
		#endif
	}
	else
		BT_Wake = 1;
	
	BT_CONFIG &= (~(1<<BT_INIT_FLAG));
}






/*
* @brief:lpm_sleep_config
* @param:time: wakeup time (unit:second)
* @param:gpio_num: 0~47
* @param:islow_wakeup:	0:High level wakeup
												1:LOW  level wakeup
* @return: none
*/
void lpm_sleep_config(uint32_t time,uint8_t gpio_num,uint8_t islow_wakeup,uint8_t is_powerdownbt)
{
		 uint32_t tmp = 0;

		 //DISABLE RTC WAKEUP
		 setlpmval(LPM_GPIO_WKHI,17,1,0);
	
		 //set gpio wakeup level
		 setlpmval(LPM_GPIO_WKHI,18,1,islow_wakeup?1:0);
		 
		 //set gpio num
		 if(islow_wakeup)
		 {
				GPIO_CONFIG(gpio_num) = GPCFG_PU;
		 }
		 else
		 {
				GPIO_CONFIG(gpio_num) = GPCFG_PD;
		 }
		 
		 if(gpio_num<32)
		 {
			 setlpmval(LPM_GPIO_WKUP,0,32,1 << gpio_num);
		 }
		 else if(gpio_num < 48)
		 {
			 setlpmval(LPM_GPIO_WKHI,0,16,1 << (gpio_num-32));
		 }
		 else
		 {
			 setlpmval(LPM_GPIO_WKUP,0,32,0);
			 setlpmval(LPM_GPIO_WKHI,0,16,0);
		 }
		 

		 read_otp(0x13e,(uint8_t *)&tmp,2);
	   tmp = (tmp == 0) ? 0x8000:tmp;
		 
		 if(time)
		 {
			 //SET SECMAX
			 setlpmval(LPM_SECMAX,0,16,tmp);
			 //SET SLEEP CNT
			 setlpmval(LPM_WKUP_TIMER,0,32,time);
			 //clr RTC CNT
			 setlpmval(LPM_RTC_CNT,0,32,0);
			 // ENABLE RTC WAKEUP
			 setlpmval(LPM_GPIO_WKHI,17,1,1);
		 }
		 
		 if(is_powerdownbt)
		 {
					//set bt gpio wakeup  and enable wakeup
					
					disable_intr(INTR_BT);
					SYSCTRL_ROM_SWITCH = 0x98;
					HCI_TypeDef IpcRxData;
					IpcRxData.type = 1;
					IpcRxData.opcode= 0x27;
					IpcRxData.DataLen=0;
					IPC_TxData(&IpcRxData);
					delay(1000000);
		 }
		 
//		 lpm_sleep();
}

void flash_powerdown(void)
{
		((void(*)(uint8_t cmd))(0x4800+1))(0xB9);//flash power down
		((void(*)())(0x4e9c+1))();// m0 goto sleep
}

void HARD_FAULT_IRQHandler()
{
	UART_Configuration();
  MyPrintf("*************HARD_FAULT_IRQHandler !*********\n\n");
};



void gpio_all_pd(void)
{
	 for(int i=0;i<48;i++)
	{
		if(GPIO_CONFIG(i)== 0 || GPIO_CONFIG(i)== 4 || GPIO_CONFIG(i)== 5)
		{
				GPIO_CONFIG(i) |= GPCFG_PD;
		}
	}

}

void lpm_sleep_test(void);
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
//	lpm_sleep_test();
	
	__asm("CPSID i");
	SYSCTRL_ROM_SWITCH = 0x9c;
	*(volatile uint8_t *)0xf8401 =0x28;
#ifdef UART_DEBUG
	UART_Configuration();
  MyPrintf("*************M0 lpm sleep Demo !*********\n\n");
#endif
	__asm("CPSIE i");

	//step1:close trng 
	SYSCTRL_HWCTRL(11) = 0;

	//step2:enable BT
	IpcInit();
	delay(10000);
	
	
	//step3:close unused clk
	*(volatile int*) 0xf856c = 0x757ff;
	
	//step4:close unused gpio
	gpio_all_pd();
	
	
	GPIO_Config(GPIOC, GPIO_Pin_7, GPCFG_PD);
	GPIO_Config(GPIOC, GPIO_Pin_6, GPCFG_PD);
	GPIO_Config(GPIOC, GPIO_Pin_8, GPCFG_PD);
	

	while (1)
	{
		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7) == 1)
		{
				static uint8_t first = 0;
#ifdef UART_DEBUG
				MyPrintf("BT start sleep..............\n\n");
#endif
				if(first ==0)
				{
					disable_intr(INTR_BT);
					SYSCTRL_ROM_SWITCH = 0x98;

					HCI_TypeDef IpcRxData;
					IpcRxData.type = 1;
					IpcRxData.opcode= 0x27;
					IpcRxData.DataLen=0;
					IPC_TxData(&IpcRxData);
					delay(10000);
					first++;
				}					
		}
		
		
		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6) == 1)
		{
#ifdef UART_DEBUG
			MyPrintf("M0 start sleep0..............\n\n");
#endif			
				*(volatile int*) 0xf856c = 0x757ff;
				*(volatile int*) 0xf8560 = 0xe; 
		}
		
		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == 1)
		{
#ifdef UART_DEBUG
				MyPrintf("M0 start sleep1..............\n\n");
#endif		
				*(volatile int*) 0xf856c = 0x757ff;
				GPIO_Config(GPIOA, GPIO_Pin_0, GPCFG_PU|GPCFG_UART0_RXD);
//				GPIO_Config(GPIOA, GPIO_Pin_1, GPCFG_PD);
				lpm_sleep_config(0,11,0,1);
//				((void(*)(void *start_addr, uint32_t len))(0x43cf))(flash_powerdown,100);
				flash_powerdown();
		}
		
		
		
		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == 1)
		{
#ifdef UART_DEBUG
				MyPrintf("M0 Enter WFI..............\n\n");
#endif									
				__asm("WFI");
		}
		
	}
}




void __download_btcode(const byte *btcode)
{
	int addr, len;
	btcode += 2;
	if(GETWORD(btcode) != 0x55aa) return;
	enable_clock(CLKCLS_BT);	
	len = GETWORD(btcode);
	BT_UCODE_LO = 0;
	BT_UCODE_HI = 0;
	BT_UCODE_CTRL = 0x80;
	DMA_CONFIG(DMACH_MEMCP) = 8;
	_dmacopy((byte*)&BT_UCODE_DATA, (byte*)btcode, len);
	BT_UCODE_CTRL = 0;
	btcode += len;
	DMA_CONFIG(DMACH_MEMCP) = 0;
	
	while(GETWORD(btcode) == 0x55aa) {
		len = GETWORD(btcode);
		addr = GETWORD(btcode) | 0xc0000;
		_dmacopy((byte*)addr, (byte*)btcode, len);
		btcode += len;
	}
}
void lpm_sleep_test(void)
{
				uint32_t j = 0;
			 volatile byte *t = (byte*)0xc0000;
			 lpm_write(LPM_SECMAX, 0x80008000);
			 lpm_write(LPM_CTRL, 0xd9290f);
			 lpm_write(LPM_GPIO_WKHI,0x20000);
        SYSCTRL_HWCTRL(11) = 0;
        SYSCTRL_CLK_CLS = 0x7d7df;
        SYSCTRL_ROM_SWITCH = 8;
        BT_CONFIG = 0;
        gpio_all_pd();
        *t = 0;
        __download_btcode(bt_code);
        while(1)
				{
         while(!*t);
         switch(*t >> 1) 
					 {
									case 1 : 
									 SYSCTRL_CLK_CLS = 0; 
									 SYSCTRL_HCLK_CON = 0;
									 break;
									case 2 : 
									 SYSCTRL_CLK_CLS = 0x7d7df; 
									 SYSCTRL_HCLK_CON = 14;
									 break;
									case 3 : 
									 SYSCTRL_CLK_CLS = 0x7d7df; 
									 SYSCTRL_HCLK_CON = 2;
									 break;
									case 4 : 
										j = lpm_read(LPM_RTC_CNT);
									  lpm_write(LPM_WKUP_TIMER, j + 10);
										lpm_sleep();
									break;          
					}
         *t = 0;
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
	GPIO_Config(GPIOA, GPIO_Pin_1, GPCFG_UART0_TXD);
	GPIO_Config(GPIOA, GPIO_Pin_0, GPCFG_UART0_RXD);

	/* USARTx configured as follow:
  - BaudRate = 115200 baud  
  - Word Length = 8 Bits
  - Stop Bit = 1 Stop Bit
  - Parity = No Parity
  - Hardware flow control disabled (RTS and CTS signals)
  - Receive and transmit enabled
  */
	UART_InitStruct.BaudRate = uartBaud;			//Configure serial port baud rate, the baud rate defaults to 128000.
	UART_InitStruct.DataBits = Databits_8b;
	UART_InitStruct.StopBits = StopBits_1;
	UART_InitStruct.Parity = Parity_None;
	UART_InitStruct.FlowCtrl = FlowCtrl_None;
	UART_InitStruct.Mode = Mode_duplex;
	
	UART_Init(UART0, &UART_InitStruct);
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/