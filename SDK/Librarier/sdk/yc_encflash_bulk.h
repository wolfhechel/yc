/*
File Name    : yc_encflash_bulk.h
Author       : Yichip
Version      : V1.0
Date         : 2019/08/24
Description  : encflash operation flash for bulk
*/
 
#ifndef __ENCFLASH_BULK_H__
#define __ENCFLASH_BULK_H__

#include "yc3121.h"


/**
  * @brief  enc write flash for bulk init
  * @param  none
  * @return none
  */
void enc_write_flash_bulk_init(void);


/**
  * @brief  enc write flash for bulk
  * @param  flash_addr, tx buf, len
  * @param  isend:The last packet of data if TRUE,other is FALSE
  * @return none
  */
void enc_write_flash_bulk(uint32_t flash_addr, uint8_t *buf, uint32_t len,uint8_t isend);


/**
  * @brief  get qspi bulk lib version
  * @param  none
  * @return lib version:
	*											bit0~bit15:low version number
	*											bit0~bit15:high version number
  */
uint32_t qspi_bulk_GetVersion(void);

#endif
