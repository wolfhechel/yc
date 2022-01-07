/*
File Name    : spi.h
Author       : Yichip
Version      : V1.1
Date         : 2018/12/06
Description  : SPI encapsulation.
*/

#ifndef __SPI_H__
#define __SPI_H__

#include "yc3121.h"

typedef enum
{
    SPI0 = 1,
    SPI1 = 2
} SPIx_TypeDef;
#define IS_SPIx(SPIx)  (SPIx==SPI0 || SPIx== SPI1)

/** @
  * @defgroup SPI_mode
  */
#define SPI_Mode_Master    ((uint8_t)0<<3)
#define SPI_Mode_Slave     ((uint8_t)1<<3)
#define IS_SPI_MODE(MODE)  (((MODE) == SPI_Mode_Master) || \
                            ((MODE) == SPI_Mode_Slave))

/** @
  * @defgroup SPI_CPOL
  */
#define  SPI_CPOL_Low      ((uint8_t)0<<5)
#define  SPI_CPOL_High     ((uint8_t)1<<5)

#define IS_SPI_CPOL(CPOL)  (((CPOL) == SPI_CPOL_High) || \
                            ((CPOL) == SPI_CPOL_Low))

/** @
  * @defgroup SPI_CPHA
  */
#define SPI_CPHA_First_Edge   ((uint8_t)0<<4)
#define SPI_CPHA_Second_Edge  ((uint8_t)1<<4)

#define IS_SPI_CPHA(CPHA)     (((CPHA) == SPI_CPHA_First_Edge) || \
                               ((CPHA) == SPI_CPHA_Second_Edge))

/** @
  * @defgroup SPI_BaudSpeed
  */
#define SPI_BaudRatePrescaler_1               0
#define SPI_BaudRatePrescaler_2               1
#define SPI_BaudRatePrescaler_4               2
#define SPI_BaudRatePrescaler_8               3
#define SPI_BaudRatePrescaler_16              4
#define SPI_BaudRatePrescaler_32              5
#define SPI_BaudRatePrescaler_64              6
#define SPI_BaudRatePrescaler_128             7
#define IS_SPI_BAUDRATE_PRESCALER(PRESCALER)  (((PRESCALER) == SPI_BaudRatePrescaler_1) || \
                                               ((PRESCALER) == SPI_BaudRatePrescaler_2) || \
                                               ((PRESCALER) == SPI_BaudRatePrescaler_4) || \
                                               ((PRESCALER) == SPI_BaudRatePrescaler_8) || \
                                               ((PRESCALER) == SPI_BaudRatePrescaler_16) || \
                                               ((PRESCALER) == SPI_BaudRatePrescaler_32) || \
                                               ((PRESCALER) == SPI_BaudRatePrescaler_64) || \
                                               ((PRESCALER) == SPI_BaudRatePrescaler_128) )

/** @
  * @defgroup SPI_BaudSpeed
  */
#define IS_SPI_RW_Delay(x)   (x>0 && x<127)

/**
  * @brief  SPI Init structure definition
  */
typedef struct
{
    uint8_t Mode;                /*!< Specifies the SPI operating mode. This parameter
                                    can be a value of @ref SPI_mode */

    uint8_t CPOL;               /*!< Specifies the serial clock steady state.
                                    This parameter can be a value of @ref SPI_Clock_Polarity */

    uint8_t CPHA;                /*!< Specifies the clock active edge for the bit capture.
                                    This parameter can be a value of @ref SPI_Clock_Phase */

    uint8_t BaudRatePrescaler;   /*!< Specifies the Baud Rate prescaler value which will be
                                    used to configure the transmit and receive SCK clock.
                                    This parameter can be a value of @ref SPI_BaudRate_Prescaler.
                                    @note The communication clock is derived from the master
                                    clock. The slave clock does not need to be set. */

    uint8_t RW_Delay;           /*!< Specifies the Delay time between send  and receive data,the
                                    value must be 0 to 127 */
} SPI_InitTypeDef;

/**
  * @brief  Initializes the SPIx peripheral according to the specified
  *         parameters in the SPI_InitStruct .
  * @param  SPIx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPI0, SPI1
  * @param  SPI_InitStruct: pointer to a USART_InitTypeDef structure
  *         that contains the configuration information for the specified SPI
  *         peripheral.
  * @retval None
  */
void SPI_Init(SPIx_TypeDef SPIx, SPI_InitTypeDef *SPI_InitStruct);

/**
  * @brief  Transmits one data via SPI DMA.
  * @param  SPIx: Select the SPI or the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPI0, SPI1.
  * @param  data: the data you want transmit.
  * @retval None
  */
void SPI_SendData(SPIx_TypeDef SPIx, uint8_t data);

/**
  * @brief  Transmits datas via SPI DMA.
  * @param  SPIx: Select the SPIx or the SPIx peripheral.
  *         This parameter can be one of the following values:
  *         SPI0, SPI1.
  * @param  buf: pointer to a buf that contains the data you want send.
  * @param  len: the buf length
  * @retval None
  */
void  SPI_SendBuff(SPIx_TypeDef SPIx, uint8_t *buff, int len);

/**
  * @brief  Send data first then  recerive data.
  *
  * @param  SPIx: Select the SPI or the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPI0, SPI1.
  * @param: TxBuff: pointer to a TxBuff  that contains the data you want send.
  * @param: TxLen: the length of send datas
  * @param: RxBuff: pointer to a TxBuff  that contains the data you want receive.
  * @param: RxLen: the length of receive datas
  * @retval None
  */
void SPI_SendAndReceiveData(SPIx_TypeDef SPIx, uint8_t *TxBuff, uint16_t TxLen, uint8_t *RxBuff, uint16_t RxLen);

#endif
