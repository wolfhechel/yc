/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\SPI\SPI_FLASH\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    21-July-2020
  * @brief   SPI for FLASH(W25Q16) program.
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
#define W25X_WriteEnable      0x06
#define W25X_WriteDisable     0x04
#define W25X_ReadStatusReg    0x05
#define W25X_WriteStatusReg   0x01
#define W25X_ReadData         0x03
#define W25X_FastReadData     0x0B
#define W25X_FastReadDual     0x3B
#define W25X_PageProgram      0x02
#define W25X_BlockErase       0xD8
#define W25X_SectorErase      0x20
#define W25X_ChipErase        0xC7
#define W25X_PowerDown        0xB9
#define W25X_ReleasePowerDown 0xAB
#define W25X_DeviceID         0xAB
#define W25X_ManufactDeviceID 0x90
#define W25X_JedecDeviceID    0x9F

//W25Q80 ID  0XEF13
//W25Q16 ID  0XEF14
//W25Q32 ID  0XEF15
//W25Q32 ID  0XEF16
#define W25Q80 0XEF13
#define W25Q16 0XEF14
#define W25Q32 0XEF15
#define W25Q64 0XEF16

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t W25Q16_ReadBuff[16] = {0};
uint16_t FlashID = 0;

/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void SPI_Configuration(void);
void W25Q16_WriteEnable(void);
void W25Q16_WaitBusyState(void);
void Spi_W25X_ChipErase(uint32_t Addr);
uint16_t Spi_W25Q16_ReadDeviceID(void);
void Spi_W25Q16_PageWrite(uint32_t Addr);
void Spi_W25Q16_ReadData(uint32_t Addr, uint32_t Len, uint8_t *p);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();
    SPI_Configuration();

    GPIO_Config(GPIOC, GPIO_Pin_8, OUTPUT_HIGH);
    GPIO_Config(GPIOC, GPIO_Pin_6, OUTPUT_HIGH);
    GPIO_Config(GPIOB, GPIO_Pin_8, OUTPUT_HIGH);

    FlashID = Spi_W25Q16_ReadDeviceID();
    MyPrintf("FlashID = 0x%04x\n", FlashID);
    Spi_W25X_ChipErase(0x00);
    Spi_W25Q16_PageWrite(0x00);
    Spi_W25Q16_ReadData(0x00, 16, W25Q16_ReadBuff);

    for (int i = 0; i < 15; i++)
        MyPrintf("%d\n", W25Q16_ReadBuff[i]);

    while (1)
    {
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
    GPIO_Config(GPIOC, GPIO_Pin_9, SPID0_NCS);
    GPIO_Config(GPIOB, GPIO_Pin_0, SPID0_SCK);

    SPI_InitStruct.Mode = SPI_Mode_Master;
    SPI_InitStruct.BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_InitStruct.CPHA = SPI_CPHA_First_Edge;
    SPI_InitStruct.CPOL = SPI_CPOL_Low;
    SPI_InitStruct.RW_Delay = 33;

    SPI_Init(SPI0, &SPI_InitStruct);
}

/**
  * @brief  W25Q16_WriteEnable
  * @param  None
  * @retval None
  */
void W25Q16_WriteEnable(void)
{
    uint8_t TxBuff = W25X_WriteEnable;

    SPI_SendAndReceiveData(SPI0, &TxBuff, 1, 0, 0);
}

/**
  * @brief  W25Q16_WaitBusyState
  * @param  None
  * @retval None
  */
void W25Q16_WaitBusyState(void)
{
    uint8_t tmp = 0x01;
    uint8_t TxBuff = W25X_ReadStatusReg;
    while (tmp & 0x01)
    {
        SPI_SendAndReceiveData(SPI0, &TxBuff, 1, &tmp, 1);
    }
}

/**
  * @brief  Spi_W25X_ChipErase
  * @param  Addr
  * @retval None
  */
void Spi_W25X_ChipErase(uint32_t Addr)
{
    W25Q16_WriteEnable();
    uint8_t TxBuff[4] = {W25X_ChipErase, Addr >> 16, Addr >> 8, Addr};
    SPI_SendAndReceiveData(SPI0, TxBuff, 4, 0, 0);
    W25Q16_WaitBusyState();
}

/**
  * @brief  Spi_W25Q16_ReadDeviceID
  * @param  None
  * @retval ID
  */
uint16_t Spi_W25Q16_ReadDeviceID(void)
{
    uint16_t ID = 0;
    uint8_t RxBuff[2] = {0};
    uint8_t TxBuff[6] = {W25X_ManufactDeviceID, 0x00, 0x00, 0x00};
    SPI_SendAndReceiveData(SPI0, TxBuff, 4, RxBuff, 2);
    ID |= RxBuff[0] << 8;
    ID |= RxBuff[1];
    return ID;
}

/**
  * @brief  Spi_W25Q16_PageWrite
  * @param  Addr
  * @retval None
  */
void Spi_W25Q16_PageWrite(uint32_t Addr)
{

    uint8_t TxBuff[20] = {W25X_PageProgram, Addr >> 16, Addr >> 8, Addr, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    W25Q16_WriteEnable();

    SPI_SendAndReceiveData(SPI0, TxBuff, 19, 0, 0);

    W25Q16_WaitBusyState();
}

/**
  * @brief  Spi_W25Q16_ReadData
  * @param  Addr, Len,  *p
  * @retval None
  */
void Spi_W25Q16_ReadData(uint32_t Addr, uint32_t Len, uint8_t *p)
{
    uint8_t TxBuff[4] = {W25X_ReadData, Addr >> 16, Addr >> 8, Addr};

    SPI_SendAndReceiveData(SPI0, TxBuff, 4, p, Len);
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
