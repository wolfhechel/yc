/*
File Name    : yc_iic.h
Author       : Yichip
Version      : V1.0
Date         : 2019/12/7
Description  : IIC encapsulation.
*/

#ifndef __YC_IIC_H__
#define __YC_IIC_H__

#include "yc3121.h"

/**
  *@brief IIC Core parameters struct
  */
typedef struct
{
    uint8_t scll;	/*!< Specifies the Clock Pulse Width Low. */
    uint8_t sclh;   /*!< Specifies the Clock Pulse Width High. */
    uint8_t stsu;   /*!< Specifies the Start Setup Time. */
    uint8_t sthd;   /*!< Specifies the Start Hold Time. */
    uint8_t sosu;   /*!< Specifies the Stop Setup Time. */
    uint8_t dtsu;   /*!< Specifies the Data Setup Time. */
    uint8_t dthd;   /*!< Specifies the Data Hold Time. */
} IIC_InitTypeDef;

/**
  * @brief	Initializes the IIC peripheral according to the specified
  *        	parameters in the IIC_InitTypeDef.
  * @param  IIC_InitStruct: pointer to a IIC_InitTypeDef structure
  *         that contains the configuration information for the specified IIC
  *         peripheral.
  * @retval None
  */
void IIC_Init(IIC_InitTypeDef *IIC_InitStruct);

/**
  * @brief	Write slave devices.
  * @param  Src: pointer to the buffer that contains the data you want to send.
  * @param  len: the length of send data
  * @note	  After writting slave device, must delay a period of time before reading slave device.
  * @retval None
  */
void IIC_SendData(uint8_t *Src, uint16_t len);

/**
  * @brief	Read slave devices.
  * @param  Src: pointer to the buffer that contains the data you want to send.
  * @param  len: the length of send data
  * @param  Dest: pointer to the buffer that contains the data received from slaver.
  * @param	Destlen: the length of received data
  * @retval None
  */
void IIC_ReceiveData(uint8_t *Src, uint16_t Srclen, uint8_t *Dest, uint16_t Destlen);

#endif
