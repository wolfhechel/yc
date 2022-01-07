/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\LPM_SLEEP\LPM_TEST\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    4-Dec-2019
  * @brief   LPM test program.
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
#include "yc_trng.h"
#include "yc_sysctrl.h"
#include "misc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define uartBaud 115200

#define IPC_RX_EVENT               0x02
#define IPC_EVENT_LE_CONN          0x02
#define IPC_EVENT_LE_DISCONN       0x05
#define IPC_EVENT_SET_CMD_RSP      0x06
#define IPC_EVENT_STANDBY          0x09
#define IPC_EVENT_LE_RECEIVE_DATA  0x08

#define IPC_CMD_SET_BT_ADDR        0x00
#define IPC_CMD_SET_BLE_ADDR       0x01
#define IPC_CMD_SET_VISIBILITY     0x02

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
uint8_t IpcReadData(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();

    MyPrintf("Yichip Yc3121 LPM test Demo V1.0.\r\n");

    //step1:close trng
    Disable_Trng();

    //step2:enable BT
    IpcInit();
    NVIC_EnableIRQ(BT_IRQn);

    //step3:close unused clk
    SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_INTR | SYSCTRL_AHBPeriph_SHA | \
                            SYSCTRL_AHBPeriph_CRC | SYSCTRL_AHBPeriph_PWM | \
                            SYSCTRL_AHBPeriph_WDT | SYSCTRL_AHBPeriph_USB | \
                            SYSCTRL_AHBPeriph_SPI | SYSCTRL_AHBPeriph_DES | \
                            SYSCTRL_AHBPeriph_RSA | SYSCTRL_AHBPeriph_ASE | \
                            SYSCTRL_AHBPeriph_7816 | SYSCTRL_AHBPeriph_SM4 | \
                            SYSCTRL_AHBPeriph_7811 | SYSCTRL_AHBPeriph_ADC7811 | \
                            SYSCTRL_AHBPeriph_CP, DISABLE);

    //step4:close unused gpio
    GPIO_Unused_Pd();

    GPIO_Config(GPIOC, GPIO_Pin_7, PULL_DOWN);
    GPIO_Config(GPIOC, GPIO_Pin_8, PULL_DOWN);
    GPIO_Config(GPIOC, GPIO_Pin_9, PULL_DOWN);

    static uint8_t first = 0;

    while (1)
    {
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7) == 1 && first == 0)
        {
            MyPrintf("BT start sleep..............\n\n");

            BT_Hibernate();
            NVIC_DisableIRQ(BT_IRQn);
            //			IpcInit(); 通过Ipcinit唤醒BT
            //			NVIC_EnableIRQ(BT_IRQn);
            first++;
        }
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8) == 1)
        {
            MyPrintf("M0 start sleep0..............\n\n");
            Chip_Speedstep();
            //		SYSCTRL_HCLKConfig(SYSCTRL_HCLK_Div2); 恢复时钟即可恢复正常
        }
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 1)
        {
            MyPrintf("M0 start sleep1..............\n\n");
            CM0_Sleep(0, 1 << 11, 0, 0, 1);
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
    GPIO_Config(GPIOA, GPIO_Pin_1, UART0_TXD);
    GPIO_Config(GPIOA, GPIO_Pin_0, UART0_RXD);

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
    UART_InitStruct.Parity   = Parity_None;
    UART_InitStruct.FlowCtrl = FlowCtrl_None;
    UART_InitStruct.Mode     = Mode_duplex;

    UART_Init(UART0, &UART_InitStruct);
}

uint8_t IpcReadData(void)
{
    MyPrintf("ipc read test\n");
    HCI_TypeDef IpcRxData;
    uint8_t buf[255];
    IpcRxData.p_data = buf;

    if (IPC_ReadBtData(&IpcRxData) == SUCCESS)
    {
        if (IpcRxData.type == IPC_RX_EVENT)
        {
            MyPrintf("Ipc Receive data \n");
        }
        else
        {
            MyPrintf("Ipc Receive error data \n");
            return ERROR;
        }
        switch (IpcRxData.opcode)
        {
        case IPC_EVENT_STANDBY:
            MyPrintf("BT chip standby\n");
            break;
        case IPC_EVENT_LE_CONN:
            MyPrintf("BLE CONNECT\n");
            break;
        case IPC_EVENT_LE_DISCONN:
            MyPrintf("BLE DISCONNECT\n");
            break;
        case IPC_EVENT_LE_RECEIVE_DATA:
            MyPrintf("BLE RECEIVE DATA:");
            for (int i = 0; i < IpcRxData.DataLen; i++)
            {
                MyPrintf("%02x", IpcRxData.p_data[i]);
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

void BT_IRQHandler(void)
{
    MyPrintf("\nBt Irq Handler=%x \n", BT_CONFIG);

    while (IpcReadData() == SUCCESS);

    BT_CONFIG &= (~(1 << BT_INIT_FLAG));
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
