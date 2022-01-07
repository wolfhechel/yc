#ifndef _CALC_ECC_H_
#define _CALC_ECC_H_

#include "yc_rsa.h"

#define PLATFORM8 0xffffee
#define PLATFORM  0xffffee11


#if PLATFORM == PLATFORM8
#else

typedef struct
{
	uint32_t *x;  //x轴坐标
	uint32_t *y;  //y轴坐标
}ecc_point_a;     //仿射坐标

typedef struct
{
	uint32_t *x;  //x轴坐标
	uint32_t *y;  //y轴坐标
	uint32_t *z;  //z轴坐标
}ecc_point_j;     //雅可比坐标

typedef struct
{
	uint32_t *x;  //x轴坐标
	uint32_t *y;  //y轴坐标
	uint32_t *z;  //z轴坐标
	uint32_t *t;  //t轴坐标
}ecc_point_mj;    //修正雅可比坐标

typedef struct
{
	uint32_t *p;        //模数--由外部传入
	uint32_t *a;        //椭圆曲线常数a--由外部传入
	uint32_t *b;        //椭圆曲线常数b--由外部传入
	ecc_point_a g;      //基点--由外部传入
	uint32_t *n;        //基点的阶--由外部传入
	uint32_t len_bits;  //曲线位数,比特
	uint32_t len_words; //曲线长度,字
	uint32_t field;     //域
	uint32_t a_type;    //判断a是否等于-3bit0：0-相等，1-不等--由外部传入
	uint32_t *p_c;      //以p为取模的常量c
	uint32_t p_q;       //以p为取模的调整因子q
	uint32_t *n_c;      //以p为取模的常量c
	uint32_t n_q;       //以p为取模的调整因子q
	uint32_t *n1_c;
	uint32_t n1_q;
}ecc_para;              //ecc输入参数的数据结构

typedef struct
{
	uint32_t *d;        //ecc私钥
	ecc_point_a e;      //ecc公钥
}ecc_key;               //ecc密钥的数据结构
typedef struct
{
	uint32_t mask;      //明文盲化(底数掩码)
	uint32_t verify;    //指数盲化(指数掩码)
}ecc_security;
//macro define
#define ECC_P192 0
#define ECC_P224 1
#define ECC_P256 2
#define ECC_B163 3
#define ECC_B193 4
#define ECC_B233 5
#define ECC_B257 6

#define ECC_PRIME  (('E'<<8)|('P'))
#define ECC_BINARY (('E'<<8)|('B'))
#define ECC_A_IS_NEGATIVE_3 (('A'<<24)|('I'<<16)|('N'<<8)|('3'))
#define ECC_A_NOT_NEGATIVE_3 (('A'<<24)|('N'<<16)|('N'<<8)|('3'))
#define INT_MASK_OPEN (('I'<<16)|('M'<<8)|('O'))
#define INT_MASK_CLOSE (('I'<<16)|('M'<<8)|('C'))
#define EC_PARA_VERIFY_OPEN (('E'<<24)|('P'<<16)|('V'<<8)|('O'))
#define EC_PARA_VERIFY_CLOSE (('E'<<24)|('P'<<16)|('V'<<8)|('C'))
#define SCALAR_IS_EVEN  (('S'<<16)|('I'<<8)|('E'))
#define SCALAR_NOT_EVEN (('S'<<16)|('N'<<8)|('E'))
#define EMBEDED_PUBLIC_KEY_VERIFY (('E'<<24)|('P'<<16)|('K'<<8)|('V'))
#define COMMON_PUBLIC_KEY_VERIFY  (('C'<<24)|('P'<<16)|('K'<<8)|('V'))

#define RET_POINT_INFINITE_FAR         (('E'<<24)|('P'<<16)|('F'<<8)|('F'))
#define RET_ECC_POINT_SUCCESS          (('E'<<24)|('P'<<16)|('S'<<8)|('U'))
#define RET_ECC_POINT_FAILED           (('E'<<24)|('P'<<16)|('F'<<8)|('A'))
#define RET_ECC_POINT_ADD_ERROR        (('E'<<24)|('P'<<16)|('A'<<8)|('E'))
#define RET_ECC_POINT_MULT_ERROR       (('E'<<24)|('P'<<16)|('M'<<8)|('E'))
#define RET_ECC_POINT_INFINITE_FAR     (('E'<<24)|('P'<<16)|('I'<<8)|('F'))
#define RET_ECC_PUBLIC_KEY_FAILED      (('E'<<24)|('P'<<16)|('K'<<8)|('F'))
#define RET_ECC_PUBLIC_KEY_PASS        (('E'<<24)|('P'<<16)|('K'<<8)|('P'))
#define RET_ECC_KEY_GENERATION_SUCCESS (('E'<<24)|('K'<<16)|('G'<<8)|('S'))
#define RET_ECC_KEY_GENERATION_FAILED  (('E'<<24)|('K'<<16)|('G'<<8)|('F'))

//function
extern const  unsigned int  CONST_LONG_ONE[64]  ;
extern const  unsigned int  CONST_LONG_FFFFFFFF[64]  ;

extern uint32_t sm2_rand_val[9];
extern uint8_t sm2_rand_index;




void ecc_config(ecc_para *para,uint32_t config);
uint32_t ecc_genkey(ecc_key *key,ecc_para *para,ecc_security *security,uint32_t *rand_key,uint32_t *rand_mask);
uint32_t ecc_verifykey(ecc_key *key,ecc_para *para,uint32_t config);
uint32_t ecc_pmul(ecc_point_a *result,ecc_point_a *a,uint32_t *b,ecc_para *para,ecc_security *security,uint32_t *rand, uint32_t rand_len);
uint32_t ecc_pmul_calc(ecc_point_a *result,ecc_point_a *a, uint32_t *k,uint32_t k_len,ecc_para *para);
uint32_t ecc_verifypoint(ecc_point_a *a,ecc_para *para);
uint32_t ecc_pdbl_a(ecc_point_a *result,ecc_point_a *a,ecc_para *para);
uint32_t ecc_padd_ja(ecc_point_j *result,ecc_point_j *a,ecc_point_a *b,ecc_para *para);
void sm2_bin_padd_1(ecc_point_a *result, ecc_point_j *a, ecc_point_j *b, ecc_para *para);
void sm2_bin_padd_2(ecc_point_a *result, ecc_point_j *a, ecc_point_j *b, ecc_para *para);

void sm2_rand_init(void);

#endif

#endif