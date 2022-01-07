/*
File Name    : spi.c
Author       : Yichip
Version      : V1.1
Date         : 2019/12/06
Description  : SPI encapsulation.
*/
#include "yc_spi.h"

void SPI_Init(SPIx_TypeDef SPIx, SPI_InitTypeDef *SPI_InitStruct)
{
#define BITDELAY 8
    uint32_t regspictrl = 0;

    _ASSERT(IS_SPI_MODE(SPI_InitStruct->Mode));
    _ASSERT(IS_SPI_BAUDRATE_PRESCALER(SPI_InitStruct->BaudRatePrescaler));
    _ASSERT(IS_SPI_CPOL(SPI_InitStruct->CPOL));
    _ASSERT(IS_SPI_CPHA(SPI_InitStruct->CPHA));
    _ASSERT(IS_SPI_RW_Delay(SPI_InitStruct->RW_Delay));

    regspictrl = SPI_InitStruct->Mode | \
                 SPI_InitStruct->BaudRatePrescaler | \
                 SPI_InitStruct->CPOL | \
                 SPI_InitStruct->CPHA | \
                 (SPI_InitStruct->RW_Delay) << BITDELAY;

    switch (SPIx)
    {
    case SPI0:
        SPID0_CTRL = regspictrl;
        break;
    case SPI1:
        SPID1_CTRL = regspictrl;
        break;
    }
}

void SPI_SendData(SPIx_TypeDef SPIx, uint8_t data)
{
    volatile unsigned char pdata;
    pdata = data;
    DMA_SRC_ADDR(SPIx) = (int)(&pdata);
    DMA_LEN(SPIx)      =  1 << 16;
    DMA_START(SPIx)    = (1 << DMA_START_BIT);
    while (!(DMA_STATUS(SPIx) & 1));
}

void SPI_SendBuff(SPIx_TypeDef SPIx, uint8_t *buff, int len)
{
    if (len < 1)
    {
        return;
    }

    DMA_SRC_ADDR(SPIx) = (int)buff;
    DMA_LEN(SPIx)      = (len << 16);
    DMA_START(SPIx)    = (1 << DMA_START_BIT);
    while (!(DMA_STATUS(SPIx) & 1));
}

void SPI_SendAndReceiveData(SPIx_TypeDef SPIx, uint8_t *TxBuff, uint16_t TxLen, uint8_t *RxBuff, uint16_t RxLen)
{
    if (TxLen < 1)
    {
        return;
    }

    DMA_SRC_ADDR(SPIx)  = (int)TxBuff;
    DMA_DEST_ADDR(SPIx) = (int)RxBuff;
    DMA_LEN(SPIx)       = (TxLen << 16) | RxLen;
    DMA_START(SPIx)     = (1 << DMA_START_BIT);
    while (!(DMA_STATUS(SPIx) & 1));
}
