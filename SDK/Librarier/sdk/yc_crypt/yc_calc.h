#ifndef __YC_CALC_H__
#define __YC_CALC_H__


#include "yc3121.h"
#include "yc_crypt.h"

#define SET_GENKEY     0x0
#define SET_ENC_DATA1  0x1
#define SET_ENC_DATA2  0x2

#define GET_E_LEN      0x0
#define GET_FifoBuffer 0x1

//struct
typedef struct
{
	uint32_t *a;    //operand a
	uint32_t *b;    //operand b
	uint32_t *p;    //modulus p
	uint32_t *c;    //factor  c
	uint32_t q;     //factor  q
} calc_operand;

//the values of the parameter config in function calc_config()
#define CALC_POLL  0
#define CALC_IDLE  1

//the values of the parameter index in function  calc_run()
#define FUNC_MUL           0   //multiplication
#define FUNC_DIV           1   //division
#define FUNC_MODEXP        2   //modular exponentiation
#define FUNC_MODEXP2_PRIME 3   //modular exponentiation in prime field, 2^(2*32*(Cb+1)) % B
#define FUNC_MODMUL_PRIME  4   //modular multiplication in prime field
#define FUNC_MODINV_PRIME  5   //modular inversion in prime field
#define FUNC_MODEXP2_BIN   6   //modular exponentiation in binary field, 2^(2*32*(Cb+1)) % B
#define FUNC_MODMUL_BIN    7   //modular multiplication in binary field
#define FUNC_MODINV_BIN    8   //modular inversion in binary field
#define FUNC_PM_BIN        9   //point multiplication in binary field
#define FUNC_PM_PRIME      10  //point multiplication in prime field
#define FUNC_PA_PRIME      11  //point addition in prime field
#define FUNC_PD_PRIME      12  //point double in prime field

//return value definition
#define RET_CALC_FUNCTION_ID_ERROR       (('R'<<24)|('F'<<16)|('I'<<8)|('E'))
#define RET_CALC_OPERAND_LENGTH_ERROR    (('R'<<24)|('O'<<16)|('L'<<8)|('E'))
#define RET_CALC_IMPLEMENT_SUCCESS       (('R'<<24)|('E'<<16)|('I'<<8)|('S'))  //0x52454953
#define RET_CALC_GCD_NOT_ONE             (('R'<<24)|('G'<<16)|('N'<<8)|('O'))

#define CMPOK        0xA55AA55A
#define CMPERROR     0x5AA55AA5

//function
void calc_interrupt(void);
void calc_config(uint32_t config);
void calc_setsecurity(uint8_t config);
uint32_t calc_run(uint8_t index);
uint32_t calc_add(volatile uint32_t *result,volatile uint32_t *a,volatile uint32_t *b,uint32_t len);
uint32_t calc_sub(uint32_t *result,uint32_t *a,uint32_t *b,uint32_t len);
void calc_modadd(uint32_t *result,uint32_t *a,uint32_t *b,uint32_t *p,uint32_t len);
void calc_modsub(uint32_t *result,uint32_t *a,uint32_t *b,uint32_t *p,uint32_t len);
void calc_xor(uint32_t *result,uint32_t *a,uint32_t *b,uint32_t len);
uint32_t calc_mul(uint32_t *result,uint32_t *a,uint32_t *b,uint32_t len_a,uint32_t len_b);
uint32_t calc_div(uint32_t *result_q,uint32_t *result_r,uint32_t *a,uint32_t *b,uint32_t len_a,uint32_t len_b);
uint32_t calc_mod(uint32_t *result, uint32_t *a, uint32_t *b, uint32_t len_a, uint32_t len_b);
uint32_t calc_modexp(uint32_t *result,calc_operand *operand,uint32_t len_a,uint32_t len_b);
uint32_t calc_modexp2(uint32_t *result, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t len_a, uint32_t len_b);
uint32_t calc_const_c(uint32_t *result, uint32_t *p, uint32_t len);
uint32_t calc_const_c_f2m(uint32_t *result, uint32_t *p, uint32_t len);
uint32_t calc_modmul(uint32_t *result, calc_operand *operand, uint32_t len);
uint32_t calc_modmul2(uint32_t *result, uint32_t *a,volatile uint32_t *b, uint32_t *c, uint32_t len);
uint32_t calc_modmul_f2m(uint32_t *result, calc_operand *operand, uint32_t len);
uint32_t calc_modmul2_f2m(uint32_t *result, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t len);
uint32_t calc_modinv(uint32_t *result, uint32_t *a, uint32_t *p, uint32_t len);
uint32_t calc_modinv_f2m(uint32_t *result, uint32_t *a, uint32_t *p , uint32_t len);
uint32_t calc_gcd(uint32_t *a, uint32_t *p, uint32_t len);
uint32_t calc_const_q(uint32_t *result, uint32_t *a);
uint32_t calc_const_q_f2m(uint32_t *result, uint32_t *a);
void calc_sr(uint32_t *result, uint32_t *a, uint32_t len, uint32_t sf_len);
uint32_t calc_bitlen(uint32_t *a, uint32_t len);
void calc_modinv2048(uint32_t *result, uint32_t *a, uint32_t *b, uint32_t len_a, uint32_t len_b);
uint32_t calc_gcd2048(uint32_t a, uint32_t *b, uint32_t len);
uint32_t calc_gcd2048_l(uint32_t *result,uint32_t *a, uint32_t *b, uint32_t len_a);
uint32_t calc_rolladd(uint32_t *result,uint32_t *a,uint32_t *b,uint32_t wlen, uint32_t startflag);
uint32_t calc_modmul_base(uint32_t *result, calc_operand *operand, uint32_t len, uint32_t config);


void uint32touint8(uint8_t *dst,uint32_t *src);
void uint8touint32(uint32_t *dst,uint8_t *src);
void mem_cpy(volatile unsigned int *dst,volatile unsigned int *src,unsigned int s)  ;
int mem_cmp(volatile unsigned int *str1,volatile unsigned int *str2,int len)  ;
int mem_cmp2(volatile unsigned int *str1,unsigned int str2,int len)  ;
void mem_set(volatile unsigned int *s, unsigned int c, unsigned int n);
//void revstr(char *str, size_t len);
int mem_rollcmp(volatile uint32_t *indata1,uint32_t *indata2,uint32_t wlen, uint32_t startflag);
int mem_rollcmp_char(uint8_t *indata1,uint8_t *indata2,uint32_t wlen, uint32_t startflag);
unsigned int calculatexor(unsigned int *data,unsigned int initdata,int len);
void memcpy_r(uint8_t *a,uint8_t *b,uint32_t len);
uint32_t mem_rollcpy(volatile uint32_t *result,uint32_t *content,uint32_t wlen, uint32_t startflag);
void mem_rollcpy_r_char( uint8_t *result,uint8_t *content,uint32_t wlen, uint32_t startflag);
uint32_t mem_rollcpy_char( uint8_t *result,uint8_t *content,uint32_t wlen, uint32_t startflag);
uint32_t calc_modmul_f2m_2(uint32_t *result, uint32_t *a, uint32_t *b, uint32_t *p, uint32_t *c, uint32_t len, uint32_t config);
void calc_modadd_1(uint32_t *result,uint32_t *a,uint32_t *b,uint32_t *p,uint32_t len);

void cpu_open_branch_self();
void cpu_open_uniform_branch_timing();
void cpu_close_branch_self();
void cpu_close_uniform_branch_timing();
uint32_t CRYPT_GetVersion(void);

#endif
