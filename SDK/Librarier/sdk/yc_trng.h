#ifndef __YC_TRNG_H__
#define __YC_TRNG_H__

#include "yc3121.h"

/**
 * @brief  init TRNG
 *
 * @param  none
 *
 * @retval none
 */
void TRNG_Init(void);

/**
 * @brief  get 128bit TRNG data
 *
 * @param  rand[4]:128bit TRNG data out of buf
 *
 * @retval 0:succeed  1:error
 */
void TRNG_Get(volatile unsigned long *rand);

/**
 * @brief  Turn off Trng
 *
 * @param  none
 *
 * @retval none
 */
void Disable_Trng(void);

unsigned char GetTRNGData_8bit(void);

int GetTRNGData(void);

#endif
