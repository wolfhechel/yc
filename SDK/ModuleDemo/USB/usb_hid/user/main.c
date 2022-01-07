/**
  ******************************************************************************
  * @file    3121Demo_1.0V\ModuleDemo\UART\UART_TX_Sendbuf\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    23-Oct-2019
  * @brief   USB HID test program.
  ******************************************************************************
  * @attention
  *
  * COPYRIGHT 2019 Yichip Microelectronics
  *
  * The purpose of this demo is to provide guidance to customers engaged in
    * programming work to guide them smoothly to product development,
    * so as to save their time.
    *
    * Therefore, Yichip microelectronics shall not be responsible for any direct,
    * indirect or consequential damages caused by any content of this demo
    * and/or contained in this code used by the customer in its products.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "yc3121.h"
#include "yc_gpio.h"
#include "yc_uart.h"
#include "yc_ipc.h"
#include "usb_main.h"
#include "halUSB.h"
#include "board_config.h"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
#define UART_DEBUG
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
extern USB_OTG_CORE_HANDLE  USB_OTG_dev;
USB_OTG_CORE_HANDLE *pdev;
uint8_t  rev_data[64];
volatile uint8_t tx_sendflag = 1;
int main(void)
{
    uint8_t  len, uartretval;
    uint8_t  tx_sendbuffer[128] = {0};
    uint32_t count = 0xFFFFFFFF;
    UART_Configuration();

    IpcInit();
    MyPrintf("YICHIP SCPU HID Device Demo V1.0.\n");
    MyPrintf(" 1: HID send data 64byte\n");

    halUsbInit(USB_MODE_HID);

    while (1)
    {
        if (UART_IsRXFIFONotEmpty(UART0))
        {
            uartretval = UART_ReceiveData(UART0);

            switch (uartretval)
            {
            case '1':
                while (count --)
                {
                    if (tx_sendflag == 1)
                    {
                        /* ep1 send data*/
                        tx_sendflag = 0;
                        DCD_EP_Tx(&USB_OTG_dev, HID_IN_EP, tx_sendbuffer, 64);
                    }
                }
                break;
            default:
                break;
            }
            uartretval = 0;
        }

        /* receive data and send data*/
        if (halUsbDataAck() == HAVE_DATA)
        {
            len = halUsbReceive(rev_data, 64);

            if (len == HID_EPOUT_SIZE)
            {
                halUsbSend(rev_data, len);
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
    UART_InitStruct.BaudRate = UARTBAUD;            //Configure serial port baud rate, the baud rate defaults to 128000.
    UART_InitStruct.DataBits = Databits_8b;
    UART_InitStruct.StopBits = StopBits_1;
    UART_InitStruct.Parity = Parity_None;
    UART_InitStruct.FlowCtrl = FlowCtrl_None;
    UART_InitStruct.Mode = Mode_duplex;

    UART_Init(UART0, &UART_InitStruct);
}


/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
