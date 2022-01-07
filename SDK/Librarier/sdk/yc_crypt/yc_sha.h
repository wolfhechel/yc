#ifndef __YC_SHA_H__
#define __YC_SHA_H__

#include "yc3121.h"
#include "yc_crypt.h"

typedef enum
{
	SHATYPE_1	= 0,
	SHATYPE_224 = 1,
	SHATYPE_256 = 2,
	SHATYPE_384 = 3,
	SHATYPE_512 = 4,
	SHATYPE_SM3 = 5
}SHA_ModeTypeDef;

#define RET_SHA_BUSY        (('R'<<24)|('S'<<16)|('B'<<8)|('U'))  //硬件单元处于忙状态
#define RET_SHA_MODE_ERR    (('R'<<24)|('S'<<16)|('M'<<8)|('E'))  //输入计算类型错误
#define RET_SHA_SUCCESS     (('R'<<24)|('S'<<16)|('S'<<8)|('U'))  //计算成功

/**
  * @method	sha
  * @brief	Secure Hash Alogrithm
  * @param	mode   : SHA mode: SHA_1/SHA_224/SHA_256/SHA_384/SHA_512
  * @param	output : output data buffer
  * @param	input  : input data buffer
  * @param	ibytes : size of input data
  * @retval SHA_TYPE_ERR or SHA_CRYPT_BUSY
  */
uint32_t SHA(SHA_ModeTypeDef mode, uint8_t *output, uint8_t *input,uint32_t ibytes);

/**
  * @method	sha_sec
  * @brief	Secure Hash Alogrithm Rand Copy
  * @param	mode   : SHA mode: SHA_256/SHA_384/SHA_512
  * @param	output : output data buffer
  * @param	input  : input data buffer
  * @param	ibytes : size of input data
  * @param	f_rng  : true random number generation function point
  * @param	p_rng  : true random number generation para
  * @retval SHA_TYPE_ERR or SHA_CRYPT_BUSY
  */
uint32_t SHA_Sec(SHA_ModeTypeDef mode,  uint8_t *output, uint8_t *input,uint32_t ibytes,
							rng_callback f_rng, void *p_rng);

/**
  * @method	sha
  * @brief	Secure Hash Alogrithm
  * @param	mode		:SHA mode: SHA_1/SHA_224/SHA_256/SHA_384/SHA_512
  * @param	input		:input data buffer
  * @param	ibytes		:size of input data
  * @retval 			:SHA_TYPE_ERR or SHA_CRYPT_BUSY
  */
uint32_t sha_first(SHA_ModeTypeDef mode,uint8_t *output,uint8_t *input,uint32_t ibytes);

/**
  * @method	sha
  * @brief	Secure Hash Alogrithm
  * @param	mode		:SHA mode: SHA_1/SHA_224/SHA_256/SHA_384/SHA_512
  * @param	input		:input data buffer
  * @param	ibytes		:size of input data
  * @retval 			:SHA_TYPE_ERR or SHA_CRYPT_BUSY
  */
uint32_t sha_mid(SHA_ModeTypeDef mode,uint8_t *output,uint8_t *input,uint32_t ibytes);

/**
  * @method	sha
  * @brief	Secure Hash Alogrithm
  * @param	mode		:SHA mode: SHA_1/SHA_224/SHA_256/SHA_384/SHA_512
  * @param	output		:output data buffer
  * @param	input		:input data buffer
  * @param	ibytes		:size of input data
  * @retval 			:SHA_TYPE_ERR or SHA_CRYPT_BUSY
  */
uint32_t sha_last(SHA_ModeTypeDef mode,uint8_t *output,uint8_t *input,uint32_t ibytes);


/**
  * @method	SHA_Enhance
  * @brief	Secure Hash Alogrithm
  * @param	mode		:SHA mode: SHA_1/SHA_224/SHA_256/SHA_384/SHA_512
  * @param	output		:output data buffer
  * @param	input		:input data buffer
  * @param	ibytes		:size of input data
  * @param     totallen		:size of totallen
  * @param     step               :step = 0 head,  step = 1 middle,  step = 2 tail
  * @retval 			:SHA_TYPE_ERR or SHA_CRYPT_BUSY
  */
uint32_t SHA_Enhance(SHA_ModeTypeDef mode,  uint8_t *output, uint8_t *input,uint32_t ibytes, uint32_t totallen,
						uint8_t step);
#endif

