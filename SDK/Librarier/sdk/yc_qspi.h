/*
File Name    : qspi.h
Author       : Yichip
Version      : V1.0
Date         : 2019/08/24
Description  : QSPI operation.
*/

#ifndef __QSPI_H__
#define __QSPI_H__

#include "yc3121.h"
#include "yc_otp.h"
#include "misc.h"

typedef enum {
  YC_QFLASH_SIZE_UNKNOW = 0,
  YC_QFLASH_SIZE_512KB = 1,
  YC_QFLASH_SIZE_1MB = 2,
  YC_QFLASH_SIZE_4MB = 3,
} yc_qspi_flash_size_enum;

/**
  * @brief  qspi flash sectorerase(4k)
  * @param  flash_addr
  * @return none
  */
uint8_t qspi_flash_sectorerase(uint32_t flash_addr);

/*
 * @brief:block erase  32K
 * @param:flash addr
 * @return: ERROR ,SUCCESS
 */
uint8_t qspi_flash_blockerase32k(uint32_t flash_addr);

/**
  * @brief  qspi flash write
  * @param  flash_addr, tx buf, len
  * @return none
  */
uint8_t qspi_flash_write(uint32_t flash_addr, uint8_t *buf, uint32_t len);

/**
  * @brief  qspi flash read
  * @param  flash_addr, rx buf, len
  * @return none
  */
uint8_t qspi_flash_read(uint32_t flash_addr, uint8_t *buf,uint32_t len);


/**
  * @brief  enc erase flash 32yte
  * @param  flash_addr
  * @return none
  */
void enc_erase_flash_32byte(uint32_t flash_addr);


/**
  * @brief  enc erase flash 32k
  * @param  flash_addr
  * @return none
  */
void enc_erase_flash_32k(uint32_t flash_addr);

/**
  * @brief  enc erase flash app area
  * @param  addr:MCU every 32k start addr
  * @param  len:erase len
  * @return none
  */
uint8_t enc_earse_flash_app_area(uint32_t addr,uint32_t len);

/**
  * @brief  enc write flash
  * @param  flash_addr, tx buf, len
  * @return none
  */
void enc_write_flash(uint32_t flash_addr,uint8_t *buf, uint32_t len);


/**
  * @brief  enc read flash
  * @param  flash_addr, rx buf, len
  * @return none
  */
void enc_read_flash(uint32_t flash_addr, uint8_t *buf, uint32_t len);

/**
  * @brief  enc read flash fast(You must sure that the read area is written in enc)
  * @param  flash_addr, rx buf, len
  * @return SUCCESS or ERROR
  */
uint8_t enc_read_flash_fast(uint32_t flash_addr, uint8_t *buf, uint32_t len);

/**
  * @brief  检测地址区域是否为空
  * @param  startaddr, len
  * @return none
  */
Boolean flash_blank_check(uint32_t startaddr,uint32_t len);


/**
  * @brief  prefetch
  * @param  start_addr, end_addr
  * @return none
  */
void prefetch(void *start_addr, void *end_addr);


/**
  * @brief  clear app info(reset into bootload)
  * @param  none
  * @return none
  */
void app_enable_download(void);


/**
  * @brief  clear app disable download flag(reset and gpio1(UART0_TX)to gnd into bootload)
  * @param  none
  * @return none
  */
void app_clear_disable_download_flag(void);

/**
  * @brief  update sign info
  * @param  uint32_t boot_sign_info_addr:only boot sign info addr(sign info len is 288 byte)
  * @return none
  */
void updateSignInfo(uint32_t boot_sign_info_addr);


/**
  * @brief  get qspi lib version
  * @param  none
  * @return lib version:
	*											bit0~bit15:low version number
	*											bit0~bit15:high version number
  */
uint32_t qspi_GetVersion(void);

/**
  * @brief  get qspi flash size
  * @param  none
  * @return yc_qspi_flash_size_enum
  */
yc_qspi_flash_size_enum read_flash_size(void);

#endif
