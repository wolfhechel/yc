#ifndef __YC_RSA_H__
#define __YC_RSA_H__

#include "yc_calc.h"

#define MIN_RSA_MODULUS_BITS  1024
#define MAX_RSA_MODULUS_BITS  2048
#define MAX_RSA_MODULUS_BYTES ((MAX_RSA_MODULUS_BITS + 7) / 8)
#define MAX_RSA_PRIME_BITS    ((MAX_RSA_MODULUS_BITS + 1) / 2)
#define MAX_RSA_PRIME_BYTES   ((MAX_RSA_PRIME_BITS + 7) / 8)

typedef struct
{
	uint32_t bytes;                       // RSA密钥长度字节数，即 RSA 密钥为 bytes * 8bits ，由于固定参数 MAX_RSA_MODULUS_BYTES 的限定，bytes 最大值为 256
	uint8_t  e[4];                        // RSA 公钥指数
	uint8_t  p[MAX_RSA_PRIME_BYTES];	    // RSA 素数 p
	uint32_t p_xor;                       // p 异或校验值
	uint8_t  q[MAX_RSA_PRIME_BYTES];      // RSA 素数 q
	uint32_t q_xor;                       // q 异或校验值
	uint8_t  n[MAX_RSA_MODULUS_BYTES];    // RSA 模数 n
	uint8_t  d[MAX_RSA_MODULUS_BYTES];    // RSA 私钥指数
	uint8_t  dp[MAX_RSA_PRIME_BYTES];     // d mod (p-1)
	uint8_t  dq[MAX_RSA_PRIME_BYTES];     // d mod (q-1)
	uint8_t  qp[MAX_RSA_PRIME_BYTES];     // q^-1 mod p
	uint8_t  n_c[MAX_RSA_MODULUS_BYTES];  // 模数 n 参数 C，用于 n 的模幂运算
	uint8_t  n_q[4];                      // 模数 n 参数 Q，用于 n 的模幂运算
	uint8_t  p_c[MAX_RSA_PRIME_BYTES];    // 模数 p 参数 C，用于 p 的模幂运算
	uint8_t  p_q[4];                      // 模数 p 参数 Q，用于 p 的模幂运算
	uint8_t  q_c[MAX_RSA_PRIME_BYTES];    // 模数 q 参数 C，用于 q 的模幂运算
	uint8_t  q_q[4];                      // 模数 q 参数 Q，用于 q 的模幂运算
}RSA_PrivateKeyTypeDef;


typedef struct
{
	uint32_t bytes;                       //RSA密钥长度字节数，即 RSA 密钥为 bytes * 8bits ，由于固定参数 MAX_RSA_MODULUS_BYTES 的限定，bytes 最大值为 256
	uint8_t  e[4];                        //RSA 公钥指数
	uint8_t  n[MAX_RSA_MODULUS_BYTES];    //RSA 模数 n
	uint8_t  n_c[MAX_RSA_MODULUS_BYTES];  //模数 n 参数 C，用于 n 的模幂运算
	uint8_t  n_q[4];                      //模数 n 参数 Q，用于 n 的模幂运算
}RSA_PublicKeyTypeDef;


typedef struct
{
  uint32_t bytes;                       //RSA密钥长度字节数，即 RSA 密钥为 bytes * 8bits ，由于固定参数 MAX_RSA_MODULUS_BYTES 的限定，bytes 最大值为 256
	uint8_t  e[4];                        //RSA 公钥指数
	uint8_t  n[MAX_RSA_MODULUS_BYTES];    //RSA 模数 n
}RSA_PublicKeyTypeDef2;


typedef struct
{
  uint32_t bytes;                       //RSA密钥长度字节数，即 RSA 密钥为 bytes * 8bits ，由于固定参数 MAX_RSA_MODULUS_BYTES 的限定，bytes 最大值为 256
	uint8_t  e[4];                        //RSA 公钥指数
	uint8_t  n[MAX_RSA_MODULUS_BYTES];    //RSA 模数 n
  uint8_t  d[MAX_RSA_MODULUS_BYTES];    //RSA 私钥指数
}RSA_PrivateKeyTypeDef2;


typedef enum
{
	RSA_1024	= 1024,
	RSA_2048	= 2048,
}RSA_KeyBitsTypedef;


typedef struct
{
	uint32_t *eta;
	uint32_t *pi;
	uint32_t *ro;
} rsa_prime;

//return value
#define RET_RSA_RANDNUM_NOT_ACCEPT          (('R'<<24)|('R'<<16)|('N'<<8)|('A'))  //随机数不被接受
#define RET_RSA_KEYGEN_SUCCESS              (('R'<<24)|('K'<<16)|('G'<<8)|('S'))  //RSA 密钥生成成功
#define RET_RSA_IMPLEMENT_SUCCESS           (('R'<<24)|('R'<<16)|('I'<<8)|('S'))  //执行成功
#define RET_RSA_IMPLEMENT_ERROR             (('R'<<24)|('R'<<16)|('A'<<8)|('E'))  //执行失败
#define RET_PRIME_CHECK_PASS                (('R'<<24)|('P'<<16)|('C'<<8)|('P'))  //素数检查成功
#define RET_PRIME_CHECK_FAIL                (('R'<<24)|('P'<<16)|('C'<<8)|('F'))  //素数检查失败
#define RET_RSA_SUCCESS                     (('R'<<24)|('E'<<16)|('R'<<8)|('S'))  //RSA 计算成功
#define RET_RSA_FAILURE                     (('R'<<24)|('E'<<16)|('R'<<8)|('E'))  //RSA 计算失败

//#define RET_RSA_GKEY_SUCCESS		 		(('R'<<24)|('G'<<16)|('K'<<8)|('S'))
//#define RET_RSA_GKEY_FAILURE		 		(('R'<<24)|('G'<<16)|('K'<<8)|('F'))

//#define RET_RSA_CKEY_SUCCESS		 		(('R'<<24)|('C'<<16)|('K'<<8)|('S'))
//#define RET_RSA_CKEY_FAILURE		 		(('R'<<24)|('C'<<16)|('K'<<8)|('F'))

/**
  * @method	RSA_Private
  * @brief	Raw RSA private-key operation. Useing CRT Algorithm
  * @param	output : output data buffer
  * @param	input  : input data buffer
  * @param	key    : RSA private key
  * @param	f_rng  : true random number generation function point
  * @param	p_rng  : true random number generation para
  * @retval	RET_RSA_SUCCESS or RET_RSA_FAILURE
  */
uint32_t  RSA_Private_crt(uint8_t *output, uint8_t *input, RSA_PrivateKeyTypeDef *para,rng_callback f_rng, void *p_rng);
//uint32_t RSA_PrivateStandard(uint8_t *output, uint8_t *input, RSA_PrivateKeyTypeDef *para,rng_callback f_rng, void *p_rng);

/**
  * @method	RSA_Public
  * @brief	Raw RSA public-key operation.
  * @param	output : output data buffer
  * @param	input  : input data buffer
  * @param	key    : RSA public key
  * @param	f_rng  : true random number generation function point
  * @param	p_rng  : true random number generation para
  * @retval RET_RSA_SUCCESS or RET_RSA_FAILURE
  */
//RSA_Private_Standard standard not crt;
uint32_t RSA_Private_Standard(uint8_t *output,uint8_t *input, RSA_PrivateKeyTypeDef *para,rng_callback f_rng, void *p_rng);

uint32_t RSA_Public(uint8_t *output, uint8_t *input, RSA_PublicKeyTypeDef *key, rng_callback f_rng, void *p_rng);

/**
  * @method	RSA_GenerateKey
  * @brief	Generate a RSA private key
  * @param	key      : rsa private key point
  * @param	exponent : public exponent
  * @param	nbits    : rsa module bits
  * @param	f_rng    : true random number generation function point
  * @param	p_rng    : true random number generation para
  * @retval RET_RSA_KEYGEN_SUCCESS or RET_RSA_RANDNUM_NOT_ACCEPT
  */
uint32_t RSA_GenerateKey(RSA_PrivateKeyTypeDef *key, uint32_t exponent, uint32_t nbits, rng_callback f_rng, void *p_rng);

/**
  * @method	RSA_CompleteKey
  * @brief	complete the private key by the rsa key paras p q d.
  * @param	key   : rsa private key point
  * @param	f_rng : true random number generation function point
  * @param	p_rng : true random number generation para
  * @retval RET_RSA_KEYGEN_SUCCESS or RET_RSA_RANDNUM_NOT_ACCEPT
  */
uint32_t RSA_CompleteKey(RSA_PrivateKeyTypeDef *key, rng_callback f_rng, void *p_rng);

/**
  * @method	RSA_CompleteKey_e
  * @brief	complete the private key by the rsa key paras p q e.
  * @param	key   : rsa private key point
  * @param	f_rng : true random number generation function point
  * @param	p_rng : true random number generation para
  * @retval RET_RSA_KEYGEN_SUCCESS or RET_RSA_RANDNUM_NOT_ACCEPT
  */
uint32_t RSA_CompleteKey_e(RSA_PrivateKeyTypeDef *key, rng_callback f_rng, void *p_rng);

/**
  * @method	RSA_GetPrativeKey_C_Q
  * @brief
  * @param	key   : rsa private key point
  * @param	f_rng : true random number generation function point
  * @param	p_rng : true random number generation para
  * @retval RET_RSA_IMPLEMENT_SUCCESS or RET_RSA_IMPLEMENT_ERROR
  */
uint32_t RSA_GetPrativeKey_C_Q(RSA_PrivateKeyTypeDef *key, rng_callback f_rng, void *p_rng);

/**
  * @method	RSA_GetPublicKey_C_Q
  * @brief
  * @param	key   : rsa private key point
  * @param	f_rng : true random number generation function point
  * @param	p_rng : true random number generation para
  * @retval RET_RSA_IMPLEMENT_SUCCESS or RET_RSA_IMPLEMENT_ERROR
  */
uint32_t RSA_GetPublicKey_C_Q(RSA_PublicKeyTypeDef *key, rng_callback f_rng, void *p_rng);
/**
  * @method	RSA_IsPrime
  * @brief
  * @param	X     : big number array
  * @param	size  : the big number len (32bit)
  * @param	f_rng : true random number generation function point
  * @param	p_rng : true random number generation para
  * @retval RET_PRIME_CHECK_PASS or RET_PRIME_CHECK_FAIL
  */
uint32_t RSA_IsPrime( uint32_t *X, uint32_t size, rng_callback f_rng, void *p_rng);

/**
  * @method	RSA_GeneratePrime
  * @brief
  * @param	X     : big number array
  * @param	size  : the big number bits (192/256/512/1024)
  * @param	f_rng : true random number generation function point
  * @param	p_rng : true random number generation para
  * @retval RET_CALC_IMPLEMENT_SUCCESS or RET_RSA_IMPLEMENT_ERROR
  */
uint32_t RSA_GeneratePrime( uint32_t *result, uint32_t nbits, rng_callback f_rng, void *p_rng);

/*
***************************************************************************
* Name: RSA_Private_Func
* Description: Perform RSA standard decryption or signing operation.
* Input: content: a pointer the context need to be signed or be decrypted.
		 para->bytes: the word-length of the value pointed to by content
		 para->e: RSA public key
		 para->n: a pointer to RSA public modulus
		 para->d: a pointer to RSA private key
* Output: result: a pointer to a variable to store the result of para->wlen words got from RSA_Private_Func().
* Return value: RET_RSA_IMPLEMENT_SUCCESS - The operation is successful.
                RET_RSA_IMPLEMENT_ERROR - The operation is failed.
***************************************************************************
*/

uint32_t RSA_Private_Func(uint8_t *output,uint8_t *input, RSA_PrivateKeyTypeDef2 *para,rng_callback f_rng, void *p_rng);


/**
  * @method	RSA_Public_Func
  * @brief	Raw RSA public-key operation.
  * @param	output		:output data buffer
  * @param	input		:input data buffer
  * @param	key			:RSA public key
  * 		para->bytes: the word-length of the value pointed to by content
		 	para->e: RSA public key
		 	para->n: a pointer to RSA public modulus
  * @param	f_rng		:true random number generation function point
  * @param	p_rng		:true random number generation para
  * @retval				:RET_RSA_SUCCESS or RET_RSA_FAILURE
  */
uint32_t RSA_Public_Func(uint8_t *output, uint8_t *input, RSA_PublicKeyTypeDef2 *para,rng_callback f_rng, void *p_rng);

#endif

