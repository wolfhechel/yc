/*
File Name    : yc_iic.c
Author       : Yichip
Version      : V1.0
Date         : 2019/12/7
Description  : IIC encapsulation.
*/

#include "yc_iic.h"

#define IIC_RESTART_BIT     24
#define IIC_SCLL_BIT        0
#define IIC_SCLH_BIT        8
#define IIC_STSU_BIT        16
#define IIC_STHD_BIT        24
#define IIC_SOSU_BIT        0
#define IIC_DTSU_BIT        8
#define IIC_DTHD_BIT        16

void IIC_Init(IIC_InitTypeDef *IIC_InitStruct)
{
    IICD_DELAY = (IIC_InitStruct->sthd << IIC_STHD_BIT) | (IIC_InitStruct->stsu << IIC_STSU_BIT) | (IIC_InitStruct->sclh << IIC_SCLH_BIT) | (IIC_InitStruct->scll << IIC_SCLL_BIT);
    IICD_CTRL  = (IIC_InitStruct->dthd << IIC_DTHD_BIT) | (IIC_InitStruct->dtsu << IIC_DTSU_BIT) | (IIC_InitStruct->sosu << IIC_SOSU_BIT);
}

void IIC_SendData(uint8_t *Src, uint16_t len)
{
    IICD_CTRL &= ~(1 << IIC_RESTART_BIT);

    DMA_START(DMACH_IICD)   |= (1 << DMA_RESET_BIT);
    DMA_START(DMACH_IICD)   &= ~(1 << DMA_RESET_BIT);
    DMA_SRC_ADDR(DMACH_IICD) = (int)Src;
    DMA_LEN(DMACH_IICD)      = len << 16;
    DMA_START(DMACH_IICD)    = (1 << DMA_START_BIT);

    while (!(DMA_STATUS(DMACH_IICD) & 1))
    {
    }
}

void IIC_ReceiveData(uint8_t *Src, uint16_t Srclen, uint8_t *Dest, uint16_t Destlen)
{
    IICD_CTRL |= 1 << IIC_RESTART_BIT;

    DMA_START(DMACH_IICD)    |= (1 << DMA_RESET_BIT);
    DMA_START(DMACH_IICD)    &= ~(1 << DMA_RESET_BIT);
    DMA_SRC_ADDR(DMACH_IICD)  = (int)Src;
    DMA_DEST_ADDR(DMACH_IICD) = (int)Dest;
    DMA_LEN(DMACH_IICD)       = Srclen << 16 | Destlen;
    DMA_START(DMACH_IICD)     = (1 << DMA_START_BIT);

    while (!(DMA_STATUS(DMACH_IICD) & 1))
    {
    }
}
