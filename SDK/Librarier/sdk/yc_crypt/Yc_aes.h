#ifndef __YC_AES_H__
#define __YC_AES_H__

#include "yc3121.h"
#include "yc_crypt.h"
#include "yc_calc.h"

#define RET_AES_SUCCESS          (('R'<<24)|('A'<<16)|('S'<<8)|('C'))  //加密成功
#define RET_AES_FAILURE          (('R'<<24)|('A'<<16)|('F'<<8)|('A'))  //加密失败
#define RET_AES_KEY_IS_NULL      (('R'<<24)|('A'<<16)|('K'<<8)|('N'))  //密钥为空
#define RET_AES_KEY_IS_OK        (('R'<<24)|('A'<<16)|('K'<<8)|('O'))  //密钥正确
#define RET_AES_KEY_SIZE_ERROR   (('R'<<24)|('A'<<16)|('K'<<8)|('S'))  //密钥长度错误
#define RET_AES_PACK_MODE_ERROR  (('R'<<24)|('A'<<16)|('P'<<8)|('E'))  //分组模式错误
#define RET_AES_OUTBUF_TOO_SMALL (('R'<<24)|('A'<<16)|('O'<<8)|('S'))  //输出缓冲区不足
#define RET_AES_INPUT_SIZE_ERROR (('R'<<24)|('A'<<16)|('I'<<8)|('S'))  //输入数据长度错误

#define AES_IV_SIZE	(16)

typedef enum
{
	AES_128	= 128,
	AES_192 = 192,
	AES_256 = 256
}AES_KeySizeTypeDef;

typedef uint8_t AES_IvKeyTypeDef[AES_IV_SIZE];

/**
  * @method	AES_Enc
  * @brief	AES encrypt function
  * @param	pack_mode :block encryption mode ECB or CBC
  * @param	output    :the pointer of output data buffer
  * @param	oBytes    :the size of output data buffer
	*                   ("oBytes" must be bigger or equal then "iBytes")
  * @param	input     :the pointer of input data buffer
  * @param	iBytes    :the size of input data buffer
  *                   ("iBytes" must be an integer multiple of key length)
  * @param	key       :AES key
  * @param	kSize     :the size of AES key: AES_128 AES_192 or AES_256
  * @param	iv        :initialization vector
  * @retval AES return value
  */

uint32_t AES_Enc(PACK_ModeTypeDef pack_mode,
                 uint8_t *output, uint32_t obytes,
                 uint8_t *input, uint32_t ibytes,
                 uint8_t *key, AES_KeySizeTypeDef ksize,
                 AES_IvKeyTypeDef iv,
                 rng_callback f_rng, void *p_rng);

/**
  * @method	AES_Decrypt
  * @brief	AES decrypt function
  * @param	pack_mode :block encryption mode ECB or CBC
  * @param	output    :the pointer of output data buffer
  * @param	oBytes    :the size of output data buffer
  *                    ("oBytes" must be bigger or equal then "iBytes")
  * @param	input     :the pointer of input data buffer
  * @param	iBytes    :the size of input data buffer
  *                    ("iBytes" must be an integer multiple of key length)
  * @param	key       :AES key
  * @param	kSize     :the size of AES key: AES_128 AES_192 or AES_256
  * @param	iv        :initialization vector
  * @retval AES return value
  */
uint32_t AES_Dec(PACK_ModeTypeDef pack_mode,
                 uint8_t *output, uint32_t obytes,
                 uint8_t *input, uint32_t ibytes,
                 uint8_t *key, AES_KeySizeTypeDef ksize,
                 AES_IvKeyTypeDef iv,
                 rng_callback f_rng, void *p_rng);

#endif
