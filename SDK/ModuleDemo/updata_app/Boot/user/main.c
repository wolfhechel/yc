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
#include "usb_main.h"
#include "halUSB.h"
#include "yc_qspi.h"
#include "yc_encflash_bulk.h"
#include "ota_ble.h"
#include "yc_bt.h"
#include "yc_systick.h"
#include "yc_qspi.h"
#include "otausb.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);

extern void start_app(void);
uint8_t  rx_buffer_boot[32] = {0};
extern _Bool  bBacktoAppFlag;

uint32_t APPWRITEADDR = 0x1010000;
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();
    MyPrintf("YC3121 OTA BOOT Demo\n");
    enc_read_flash(0x1000120, rx_buffer_boot, 32);
    delay_ms(20);
    if ((rx_buffer_boot[0] == 0xAA) && (rx_buffer_boot[1] == 0x55) && (rx_buffer_boot[2] == 0xAA) && (rx_buffer_boot[3] == 0x55))
    {
        /*USB OTA 0xAA55AA55*/
        IpcInit();

        enc_write_flash_bulk_init();

        enc_earse_flash_app_area(APPWRITEADDR, 320 * 1024);

        MyPrintf("USB HID OTA Demo !\n\n");

        halUsbInit(USB_MODE_HID);

        while (1)
        {
            Cmd_check();
            BackToApp(bBacktoAppFlag);
        }
    }
    else if ((rx_buffer_boot[0] == 0xA5) && (rx_buffer_boot[1] == 0x5A) && (rx_buffer_boot[2] == 0xA5) && (rx_buffer_boot[3] == 0x5A))
    {
        /*BT OTA 0xA55AA55A*/
        enc_write_flash_bulk_init();
        enc_earse_flash_app_area(APPWRITEADDR, 320 * 1024);
        MyPrintf("BT OTA Demo !\n\n");
        BleInit();
        while (1)
        {
            BT_Progress();
            Ble_Cmd_check();
            if (BleOtaUpdateStatus())
            {
                disable_intr(INTR_BT);
                MyPrintf("Run App\n");
                delay_ms(10);
                enc_erase_flash_32byte(0x1000120);
                start_app();
            }

        }
    }
    else
    {
        MyPrintf("Run App\n");
        start_app();
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

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
