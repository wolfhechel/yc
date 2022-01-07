/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\SPI\SPI_SLAVER\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    7--2020
  * @brief   SPI for SPI_SLAVER program.
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
#include "yc_spi.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t Data = 0;
uint8_t M_rxbuff[1] = {0};
uint8_t M_txbuff[1] = {0};
uint8_t S_rxbuff[1] = {0};
uint8_t S_txbuff[1] = {0};

/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void SPI_Configuration(void);
void SPI_Slaver_Test(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();
    SPI_Configuration();

    MyPrintf("Ychip 3121 SPI_SLAVER Demo !\n");

    GPIO_Config(GPIOC, GPIO_Pin_6, OUTPUT_HIGH);
    GPIO_Config(GPIOC, GPIO_Pin_8, OUTPUT_HIGH);
    GPIO_Config(GPIOC, GPIO_Pin_9, OUTPUT_HIGH);
    GPIO_Config(GPIOB, GPIO_Pin_8, OUTPUT_HIGH);

    M_txbuff[0] = 0xaa;
    S_txbuff[0] = 0x55;
    M_rxbuff[0] = 0;
    S_rxbuff[0] = 0;

    while (1)
    {
        if (UART_IsRXFIFONotEmpty(UART0))
        {
            Data = UART_ReceiveData(UART0);  // send 0xA1
        }
        SPI_Slaver_Test();
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
  * @brief  SPI 0 initialization function.
  * @param  None
  * @retval None
  */
void SPI_Configuration(void)
{
    SPI_InitTypeDef SPI_InitStruct;

    GPIO_Config(GPIOB, GPIO_Pin_2, SPID0_MISO);
    GPIO_Config(GPIOB, GPIO_Pin_1, SPID0_MOSI);
    GPIO_Config(GPIOC, GPIO_Pin_8, SPID0_NCS);
    GPIO_Config(GPIOB, GPIO_Pin_0, SPID0_SCK);

    SPI_InitStruct.Mode = SPI_Mode_Master;
    SPI_InitStruct.BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStruct.CPHA = SPI_CPHA_First_Edge;
    SPI_InitStruct.CPOL = SPI_CPOL_Low;
    SPI_InitStruct.RW_Delay = 30;

    SPI_Init(SPI0, &SPI_InitStruct);

    GPIO_Config(GPIOC, GPIO_Pin_7, 	SPID1_MISO);
    GPIO_Config(GPIOC, GPIO_Pin_10, SPID1_MOSI);
    GPIO_Config(GPIOC, GPIO_Pin_9,  SPID1_NCSIN);
    GPIO_Config(GPIOC, GPIO_Pin_11, SPID1_SCKIN);

    SPI_InitStruct.Mode = SPI_Mode_Slave;
    SPI_InitStruct.BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStruct.CPHA = SPI_CPHA_First_Edge;
    SPI_InitStruct.CPOL = SPI_CPOL_Low;
    SPI_InitStruct.RW_Delay = 30;

    SPI_Init(SPI1, &SPI_InitStruct);

    DMA_SRC_ADDR(SPI1)  = (int)S_txbuff;
    DMA_DEST_ADDR(SPI1) = (int)S_rxbuff;
    DMA_LEN(SPI1)       = (1 << 16) | 1;
}

void SPI_Slaver_Test(void)
{
    if (Data == 0xA1)
    {
        Data = 0x00;
        SPI_SendAndReceiveData(SPI0, M_txbuff, 1, M_rxbuff, 1);
        MyPrintf("\nMASTER T:%x\r\n", M_txbuff[0]);
        MyPrintf("MASTER R:%x\r\n", M_rxbuff[0]);
        MyPrintf("slave T:%x\r\n", S_txbuff[0]);
        MyPrintf("slave R:%x\r\n", S_rxbuff[0]);
        M_txbuff[0] += 1;
        S_txbuff[0] += 1;
    }
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
