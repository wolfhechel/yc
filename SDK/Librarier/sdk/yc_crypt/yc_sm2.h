#ifndef __YC_SM2_H__
#define __YC_SM2_H__

#include "yc3121.h"
#include "yc_crypt.h"
#include "system.h"
#include "yc_sm3.h"
#include "yc_ecc.h"
#include "yc_trng.h"

#define SM2_FLOW_CONTROL

#define SM2_KEY_BITS 		257
#define SM2_KEY_BYTES		((SM2_KEY_BITS + 7) / 8)
#define SM2_KEY_WORDS		((SM2_KEY_BYTES + 3) / 4)

typedef struct
{
	uint32_t r[SM2_KEY_WORDS];
	uint32_t s[SM2_KEY_WORDS];
}SM2_SignTypeDef;

typedef struct
{
	uint32_t x[SM2_KEY_WORDS];
	uint32_t y[SM2_KEY_WORDS];
}SM2_PointTypeDef;

typedef struct
{
	uint32_t p[SM2_KEY_WORDS];  //模数
	uint32_t a[SM2_KEY_WORDS];  //椭圆曲线常数 a
	uint32_t b[SM2_KEY_WORDS];  //椭圆曲线常数 b
	SM2_PointTypeDef g;         //基点
	uint32_t n[SM2_KEY_WORDS];  //基点的阶
}SM2_EllipseParaTypeDef;

typedef SM2_PointTypeDef SM2_PublicKeyTypeDef;

typedef struct
{
	uint32_t d[SM2_KEY_WORDS];
	SM2_PublicKeyTypeDef e;
}SM2_PrivateKeyTypeDef;


#define RET_SM2_ENC_SUCCESS       (('R'<<24)|('S'<<16)|('E'<<8)|('S'))  //加密成功
#define RET_SM2_ENC_FAILURE       (('R'<<24)|('S'<<16)|('E'<<8)|('F'))  //加密失败
#define RET_SM2_DEC_SUCCESS       (('R'<<24)|('S'<<16)|('D'<<8)|('S'))  //解密成功
#define RET_SM2_DEC_FAILURE       (('R'<<24)|('S'<<16)|('D'<<8)|('F'))  //解密失败
#define RET_SM2_KDF_SUCCESS       (('R'<<24)|('S'<<16)|('K'<<8)|('S'))  //密钥派生成功
#define RET_SM2_KDF_FAILURE       (('R'<<24)|('S'<<16)|('K'<<8)|('F'))  //密钥派生失败
#define RET_SM2_SIGN_SUCCESS      (('R'<<24)|('S'<<16)|('S'<<8)|('S'))  //数字签名成功
#define RET_SM2_SIGN_FAILURE      (('R'<<24)|('S'<<16)|('S'<<8)|('F'))  //数字签名错误
#define RET_SM2_SIGN_ERROR        (('R'<<24)|('S'<<16)|('S'<<8)|('E'))  //数字签名错误
#define RET_SM2_VERIFY_SUCCESS    (('R'<<24)|('S'<<16)|('V'<<8)|('S'))  //数字签名验证成功
#define RET_SM2_VERIFY_FAILURE    (('R'<<24)|('S'<<16)|('V'<<8)|('F'))  //数字签名验证失败
#define RET_SM2_VERIFY_ERROR      (('R'<<24)|('S'<<16)|('V'<<8)|('E'))  //数字签名验证错误


/**
  * @method	SM2_Genkey
  * @brief	SM2 密钥生成函数，用于生成 SM2 密钥
  * @param	key   : SM2 私钥（输出参数）
  * @param	para  : 椭圆曲线参数（输入参数）
  * @param	f_rng : 随机数函数（输入参数）
  * @param	p_rng : 随机数函数参数（输入参数）
  * @retval RET_ECC_KEY_GEN_SUCCESS or RET_SM2_KEY_GEN_ERROR
  */
uint32_t SM2_Genkey(SM2_PrivateKeyTypeDef *key, const SM2_EllipseParaTypeDef *para,
                    rng_callback f_rng, void *p_rng, uint8_t config);

//uint32_t SM2_Kdf(uint8_t *K, uint32_t klen, const uint8_t *Z, uint32_t zlen);
/**
  * @method	SM2_Hash_z
  * @brief	get sm2 sign para Za
  * @param	Za     : the Za para of sm2 sign
  * @param	IDa    : user id
  * @param	IDalen : the bits of user id (8 * (number of bytes))
  * @param	para   : sm2 paras
  * @param	key    : sm2 public key
  * @retval nil
  */
uint32_t SM2_Hash_Za(uint8_t *Za, uint8_t *IDa, uint16_t IDalen, SM2_EllipseParaTypeDef *para, SM2_PublicKeyTypeDef *key, uint16_t block_size);

/**
  * @method	SM2_Hash_e
  * @brief	get sm2 sign para e
  * @param	e    : the e para of sm2 sign (e is already cover bytes to integer)
  * @param	Za   : the Za para of sm2 sign
  * @param	m    : message
  * @param	mlen : bytes of message
  * @retval nil
  */
uint32_t SM2_Hash_e(uint32_t *e, uint8_t *Za, uint8_t *m, uint32_t mlen);

/**
  * @method	SM2_Enc
  * @brief	SM2 公钥加解密函数，使用 SM2 公钥对数据进行加密或解密操作
  * @param	output : 输出数据缓冲区的指针（输出参数）
  * @param	Olen   : 输出数据长度（输出参数）
  * @param	input  : 输入数据缓冲区的指针（输入参数）
  * @param	ilen   : 输入数据长度（输入参数）
  * @param	key    : SM2 公钥（输入参数）
  * @param	para   : 椭圆曲线参数（输入参数）
  * @param	f_rng  : 随机数函数（输入参数）
  * @param	p_rng  : 随机数函数参数（输入参数）
  * @retval RET_SM2_ENC_SUCCESS or RET_SM2_ENC_FAILURE
  */
uint32_t SM2_Enc(uint8_t *output, uint32_t *olen,
                 uint8_t *input, uint32_t ilen,
                 SM2_PublicKeyTypeDef *key,  SM2_EllipseParaTypeDef *para,
                 rng_callback f_rng, void *p_rng,uint8_t config);

/**
  * @method	SM2_Dec
  * @brief	SM2 私钥加解密函数，使用 SM2 私钥对数据进行加密或解密操作
  * @param	output : 输出数据缓冲区的指针（输出参数）
  * @param	olen   : 输出数据长度（输出参数）
  * @param	input  : 输入数据缓冲区的指针（输入参数）
  * @param	ilen   : 输入数据长度（输入参数）
  * @param	key    : SM2 私钥（输入参数）
  * @param	para   : 椭圆曲线参数（输入参数）
  * @param	f_rng  : 随机数函数（输入参数）
  * @param	p_rng  : 随机数函数参数（输入参数）
  * @retval RET_SM2_DEC_SUCCESS or RET_SM2_DEC_FAILURE
  */
uint32_t SM2_Dec(uint8_t *output, uint32_t *olen,
                 uint8_t *input, uint32_t ilen,
                 SM2_PrivateKeyTypeDef *key,  SM2_EllipseParaTypeDef *para,
                 rng_callback f_rng, void *p_rng,uint8_t config);

//uint32_t SM2_digital_sign(SM2_SignTypeDef *sign, uint8_t Za[32],
//							uint8_t *msg, uint32_t mlen,
//							SM2_PrivateKeyTypeDef *key, SM2_EllipseParaTypeDef *para,
//							mh_rng_callback f_rng, void *p_rng);
//uint32_t SM2_verify_sign(SM2_SignTypeDef *sign, uint8_t Za[32],
//							uint8_t *msg, uint32_t mlen,
//							SM2_PublicKeyTypeDef *key, SM2_EllipseParaTypeDef *para,
//							mh_rng_callback f_rng, void *p_rng);

/**
  * @method	SM2_DigitalSignWith_e
  * @brief	SM2 数字签名函数
  * @param	sign  : SM2 数字签名结果（输出参数）
  * @param	e     : hash 值（输入参数）
  * @param	msg   : 签名消息输入数据缓冲区的指针（输入参数）
  * @param	mlen  : 签名消息数据长度（输入参数）
  * @param	key   : SM2 私钥（输入参数）
  * @param	para  : 椭圆曲线参数（输入参数）
  * @param	f_rng : 随机数函数（输入参数）
  * @param	p_rng : 随机数函数参数（输入参数）
  * @retval RET_SM2_SIGN_SUCCESS or RET_SM2_SIGN_FAILURE
  */
uint32_t SM2_DigitalSignWith_e(SM2_SignTypeDef *sign, uint32_t *e,
                               uint8_t *msg, uint32_t mlen,
                               SM2_PrivateKeyTypeDef *key, SM2_EllipseParaTypeDef *para,
                               rng_callback f_rng, void *p_rng, uint8_t config);

/**
  * @method	SM2_VerifySignWith_e
  * @brief	SM2 签名验证函数
  * @param	sign  : SM2 数字签名结果（输出参数）
  * @param	e     : hash 值（输入参数）
  * @param	msg   : 签名消息输入数据缓冲区的指针（输入参数）
  * @param	mlen  : 签名消息数据长度（输入参数）
  * @param	key   : SM2 私钥（输入参数）
  * @param	para  : 椭圆曲线参数（输入参数）
  * @param	f_rng : 随机数函数（输入参数）
  * @param	p_rng : 随机数函数参数（输入参数）
  * @retval RET_SM2_VERIFY_SUCCESS or RET_SM2_VERIFY_FAILURE
  */
uint32_t SM2_VerifySignWith_e(SM2_SignTypeDef *sign, uint32_t *e,
							uint8_t *msg, uint32_t mlen,
							SM2_PublicKeyTypeDef *key, SM2_EllipseParaTypeDef *para,
							rng_callback f_rng, void *p_rng, uint8_t config);
/*
uint32_t SM2_key_ex_equation_0(uint8_t _x[32], uint32_t *_xlen,
							const uint8_t x[32], uint32_t w);
void SM2_key_ex_equation_1(uint8_t *t, const uint8_t *d,
						const uint8_t *x, const uint8_t *r,
						SM2_EllipseParaTypeDef *para);
void SM2_key_ex_equation_2(SM2_PointTypeDef *P, uint8_t *t,
						uint8_t *x, SM2_PointTypeDef *R,
						SM2_EllipseParaTypeDef *para, SM2_PublicKeyTypeDef *key,
						rng_callback f_rng, void *p_rng);

uint32_t SM2_key_ex_section_0(SM2_PointTypeDef *R, uint8_t *t, uint8_t *rand,
								SM2_EllipseParaTypeDef *para, SM2_PrivateKeyTypeDef *key,
								rng_callback f_rng, void *p_rng);
uint32_t SM2_key_ex_section_1(uint8_t *K, uint32_t Klen, SM2_PointTypeDef *P,
							SM2_PointTypeDef *R, uint8_t *t,
							uint8_t *Za, uint8_t *Zb,
							SM2_EllipseParaTypeDef *para, SM2_PublicKeyTypeDef *key,
							rng_callback f_rng, void *p_rng);
uint32_t SM2_key_ex_hash(uint8_t *Hash, uint8_t HashHead,
						SM2_PointTypeDef *P,uint8_t *Za, uint8_t *Zb,
						SM2_PointTypeDef *Ra, SM2_PointTypeDef *Rb);
*/
//#define SM2_DEBUG
#endif


