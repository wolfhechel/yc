
#ifndef __YC_SM3_H__
#define __YC_SM3_H__

#include "yc3121.h"
#include "yc_crypt.h"
#include "system.h"

#define RET_SM3_BUSY         (('R'<<24)|('S'<<16)|('3'<<8)|('B'))  //
#define RET_SM3_SUCCESS      (('R'<<24)|('S'<<16)|('3'<<8)|('S'))  //
#define RET_SM3_FAILE        (('R'<<24)|('S'<<16)|('3'<<8)|('F'))  //

#define RET_SM3_MAC_SUCCESS  (('S'<<24)|('3'<<16)|('M'<<8)|('U'))  //RET_SM3_SUCCESS
#define RET_SM4_MAC_FAILURE  (('S'<<24)|('3'<<16)|('M'<<8)|('F'))  //RET_SM3_BUSY

/**
  * @method	sm3
  * @brief	SM3 Hash Check
  * @param	output : output data buffer
  * @param	input  : input data buffer
  * @param	ibytes : size of input data
  * @retval SM3_SUCCESS or SM3_BUSY
  */
uint32_t SM3(uint8_t *output, uint8_t *input,uint32_t ibytes);

/*
 * @brief  sm3 hash check
 * @param  output     : output data buffer
 * @param  input      : input data buffer
 * @param  ibytes     : size of input data (when istep=0~1 ibytes must 64 multiples)
 * @param  totalbytes : size of all input datas
 * @param  istep      : 0~2
 * @return SM3_SUCCESS or SM3_BUSY or SM3_FAILE
 */
uint32_t SM3_Enhance(uint8_t *output, uint8_t *input,uint32_t ibytes,uint32_t totalbytes,uint32_t istep);

/*
 * @brief  sm3 hash check
 * @param  output : output data buffer
 * @param  input  : input data
 * @param  istep  : number of data
 * @param  isend  : is last data
 * @return SM3_SUCCESS or SM3_BUSY or SM3_FAILE
 */
uint32_t SM3_BYTE(uint8_t *output, uint8_t input,uint32_t istep,uint8_t isend);

/*
 * @brief  sm3-mac
 * @param  output  : output data buffer
 * @param  input   : input data  buffer
 * @param  ibytes  : size of input buffer
 * @param  key     : key buffer
 * @param  key_len : size of key buffer
 * @return SM3_MAC_SUCCESS  or SM3_MAC_FAILE
 */
uint32_t SM3_MAC(uint8_t *output, uint8_t *input,uint32_t ibytes,uint8_t *key,uint32_t key_len);

#endif

