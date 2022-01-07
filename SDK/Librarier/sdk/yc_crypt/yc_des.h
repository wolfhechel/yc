#ifndef __YC_DES_H__
#define __YC_DES_H__

#include "yc3121.h"
#include "yc_crypt.h"
#include "yc_calc.h"

#define RET_DES_SUCCESS            (('R'<<24)|('D'<<16)|('S'<<8)|('C'))  //加密成功
#define RET_DES_FAILURE            (('R'<<24)|('D'<<16)|('F'<<8)|('A'))  //加密失败
#define RET_DES_KEY_IS_OK          (('R'<<24)|('D'<<16)|('K'<<8)|('O'))  //密钥正确
#define RET_DES_KEY_IS_NULL        (('R'<<24)|('D'<<16)|('K'<<8)|('N'))  //密钥指针为空
#define RET_DES_KEY_CRC_ERROR      (('R'<<24)|('D'<<16)|('K'<<8)|('C'))  //密钥 CRC 错误
#define RET_DES_KEY_READBACK_ERROR (('R'<<24)|('D'<<16)|('K'<<8)|('R'))  //密钥回读错误
#define RET_DES_IV_CRC_ERROR       (('R'<<24)|('D'<<16)|('I'<<8)|('C'))  //IVCRC 错误
#define RET_DES_PACK_MODE_ERROR    (('R'<<24)|('D'<<16)|('P'<<8)|('E'))  //分组模式错误
#define RET_DES_OUTBUF_TOO_SMALL   (('R'<<24)|('D'<<16)|('O'<<8)|('S'))  //输出缓冲不足
#define RET_DES_INPUT_SIZE_ERROR   (('R'<<24)|('D'<<16)|('I'<<8)|('S'))  //输入数据 len 错误
#define RET_DES_RESULT_CHECK_ERROR (('R'<<24)|('D'<<16)|('R'<<8)|('C'))  //输出结果错误

#define DES_KEY_SIZE 8
#define DES_IV_SIZE  8

typedef uint8_t DES_KeyTypeDef[DES_KEY_SIZE];
typedef uint8_t DES_IvTypeDef[DES_IV_SIZE];

typedef uint8_t DES_RandTypeDef[4];
typedef uint8_t DES_FakeTypeDef[8];

#define DES_TYPE_DES_ENC  (('D'<<24)|('T'<<16)|('D'<<8)|('E'))  //DES 加密模式
#define DES_TYPE_DES_DEC  (('D'<<24)|('T'<<16)|('D'<<8)|('D'))  //DES 解密模式
#define DES_TYPE_TDES_ENC (('D'<<24)|('T'<<16)|('T'<<8)|('E'))  //TDES 加密模式
#define DES_TYPE_TDES_DEC (('D'<<24)|('T'<<16)|('T'<<8)|('D'))  //TDES 解密模式

typedef struct
{
	DES_KeyTypeDef k1;
	DES_KeyTypeDef k2;
	DES_KeyTypeDef k3;
}TDES_KeyTypeDef;

/**
  * @method	mh_des_enc
  * @brief	DES encrypt function
  * @param	pack_mode		:block encryption mode ECB or CBC
  * @param	output			:the pointer of output data buffer
  * @param	obytes			:the size of output data buffer
							("oBytes" must be bigger or equal then "iBytes")
  * @param	input			:the pointer of input data buffer
  * @param	ibytes			:the size of input data buffer
							("iBytes" must be an integer multiple of key length)
  * @param	key				:DES key
  * @param	iv				:initialization vector
  * @param	f_rng			:true random number generation function point
  * @param	p_rng			:true random number generation para
  * @retval					:DES return value
  */
uint32_t DES_Enc(PACK_ModeTypeDef pack_mode,
					uint8_t *output, uint32_t obytes,
					uint8_t *input, uint32_t ibytes,
					DES_KeyTypeDef key,	DES_IvTypeDef iv,
					rng_callback f_rng, void *p_rng);

/**
  * @method	DES_Decrypt
  * @brief	DES decrypt function
  * @param	pack_mode		:block encryption mode ECB or CBC
  * @param	output			:the pointer of output data buffer
  * @param	oBytes			:the size of output data buffer
							("oBytes" must be bigger or equal then "iBytes")
  * @param	input			:the pointer of input data buffer
  * @param	iBytes			:the size of input data buffer
							("iBytes" must be an integer multiple of key length)
  * @param	key				:DES key
  * @param	iv				:initialization vector
  * @param	f_rng			:true random number generation function point
  * @param	p_rng			:true random number generation para
  * @retval					:DES return value
  */
uint32_t DES_Dec(PACK_ModeTypeDef pack_mode,
					uint8_t *output, uint32_t obytes,
					uint8_t *input, uint32_t ibytes,
					DES_KeyTypeDef key,	DES_IvTypeDef iv,
					rng_callback f_rng, void *p_rng);

/**
  * @method	TDES_Encrypt
  * @brief	TDES encrypt function
  * @param	pack_mode		:block encryption mode ECB or CBC
  * @param	output			:the pointer of output data buffer
  * @param	oBytes			:the size of output data buffer
							("oBytes" must be bigger or equal then "iBytes")
  * @param	input			:the pointer of input data buffer
  * @param	iBytes			:the size of input data buffer
							("iBytes" must be an integer multiple of key length)
  * @param	key				:DES key
  * @param	iv				:initialization vector
  * @param	f_rng			:true random number generation function point
  * @param	p_rng			:true random number generation para
  * @retval					:TDES return value
  */
uint32_t TDES_Enc(PACK_ModeTypeDef pack_mode,
					uint8_t *output, uint32_t obytes,
					uint8_t *input, uint32_t ibytes,
					TDES_KeyTypeDef *key, DES_IvTypeDef iv,
					rng_callback f_rng, void *p_rng);
/**
  * @method	TDES_Decrypt
  * @brief	TDES decrypt function
  * @param	pack_mode		:block encryption mode ECB or CBC
  * @param	output			:the pointer of output data buffer
  * @param	oBytes			:the size of output data buffer
							("oBytes" must be bigger or equal then "iBytes")
  * @param	input			:the pointer of input data buffer
  * @param	iBytes			:the size of input data buffer
							("iBytes" must be an integer multiple of key length)
  * @param	key				:DES key
  * @param	iv				:initialization vector
  * @param	f_rng			:true random number generation function point
  * @param	p_rng			:true random number generation para
  * @retval					:TDES return value
  */
uint32_t TDES_Dec(PACK_ModeTypeDef pack_mode,
					uint8_t *output, uint32_t obytes,
					uint8_t *input, uint32_t ibytes,
					TDES_KeyTypeDef *key, DES_IvTypeDef iv,
					rng_callback f_rng, void *p_rng);


//struct
typedef struct
{
	uint8_t *result;  //输出数据指针
	uint8_t *content; //输入数据指针
	uint8_t *key1;    //密钥1指针
	uint8_t *key2;    //密钥2指针
	uint8_t *key3;    //密钥3指针
	uint8_t *iv;      //cbc模块初始向量指针
	uint8_t *rand;    //随机数2字节
	uint8_t *fakekey; //假秘钥8字节
	uint8_t config;   //des运算配置
	uint32_t length;  //cbc模式下的数据字节数
}des_para;



#endif


