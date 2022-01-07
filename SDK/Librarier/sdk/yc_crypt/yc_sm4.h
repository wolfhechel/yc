#ifndef __YC_SM4_H__
#define __YC_SM4_H__

#include "yc3121.h"
#include "yc_crypt.h"
#include "system.h"

#define SM4_FLOW_CONTROL

#define RET_SM4_SUCCESS             (('S'<<24)|('4'<<16)|('S'<<8)|('U'))  //计算成功
#define RET_SM4_FAILURE             (('S'<<24)|('4'<<16)|('S'<<8)|('F'))  //计算错误
#define RET_SM4_KEY_IS_NULL         (('S'<<24)|('4'<<16)|('K'<<8)|('N'))
#define RET_SM4_PACK_MODE_ERROR     (('S'<<24)|('4'<<16)|('P'<<8)|('E'))
#define RET_SM4_OUTBUF_TOO_SMALL    (('S'<<24)|('4'<<16)|('O'<<8)|('S'))
#define RET_SM4_INPUT_SIZE_ERROR    (('S'<<24)|('4'<<16)|('P'<<8)|('S'))
#define RET_SM4_KEY_READBACK_ERROR  (('S'<<24)|('4'<<16)|('K'<<8)|('R'))
#define RET_SM4_RESULT_CHECK_ERROR  (('S'<<24)|('4'<<16)|('R'<<8)|('C'))


#define SM4_TYPE_DES_ENC            (('4'<<24)|('T'<<16)|('D'<<8)|('E'))
#define SM4_TYPE_DES_DEC            (('4'<<24)|('T'<<16)|('D'<<8)|('D'))


#define SM4_KEY_SIZE  (128 / 8)
#define SM4_IV_SIZE   (128 / 8)

typedef uint8_t SM4_KeyTypeDef[SM4_KEY_SIZE];
typedef uint8_t SM4_IvTypeDef[SM4_IV_SIZE];

/**
  * @method	SM4_Enc
  * @brief	sm4 encrypt function
  * @param	pack_mode : block encryption mode ECB or CBC
  * @param	output    : the pointer of output data buffer
  * @param	obytes    : the size of output data buffer
  *                     ("obytes" must be bigger or equal then "iBytes")
  * @param	input     : the pointer of input data buffer
  * @param	ibytes    : the size of input data buffer
  *                     ("ibytes" must be an integer multiple of key length)
  * @param	key       : SM4 key
  * @param	iv        : initialization vector
  * @retval SM4 return value
  */
uint32_t SM4_Enc(PACK_ModeTypeDef pack_mode,
                 uint8_t *output, uint32_t obytes,
                 uint8_t *input, uint32_t ibytes,
                 SM4_KeyTypeDef key, SM4_IvTypeDef iv,
                 rng_callback f_rng, void *p_rng);

/**
  * @method	SM4_Dec
  * @brief	SM4 decrypt function
  * @param	pack_mode : block encryption mode ECB or CBC
  * @param	output    : the pointer of output data buffer
  * @param	obytes    : the size of output data buffer
  *                     ("obytes" must be bigger or equal then "iBytes")
  * @param	input     : the pointer of input data buffer
  * @param	ibytes    : the size of input data buffer
  *                     ("ibytes" must be an integer multiple of key length)
  * @param	key       : SM4 key
  * @param	iv        : initialization vector
  * @retval SM4 return value
  */
uint32_t SM4_Dec(PACK_ModeTypeDef pack_mode,
                 uint8_t *output, uint32_t obytes,
                 uint8_t *input, uint32_t ibytes,
                 SM4_KeyTypeDef key, SM4_IvTypeDef iv,
                 rng_callback f_rng, void *p_rng);

#endif
