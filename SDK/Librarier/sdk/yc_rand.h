#ifndef __YC_RAND_H
#define __YC_RAND_H

#include <math.h>
#include "string.h"
#include "yc3121.h"
#include "yc_trng.h"

#define RET_RAND_CHECK_FAILURE				(('M'<<24)|('R'<<16)|('C'<<8)|('F'))
#define RET_RAND_CHECK_SUCCESS				(('M'<<24)|('R'<<16)|('C'<<8)|('S'))
#define RET_RAND_CHECK_DATA_LENGTH_ERROR		(('M'<<24)|('R'<<16)|('C'<<8)|('L'))

/**
  * @brief	get random bytes
  * @param	rand	:random data buffer
  * @param	bytes	:size of rand data
  * @retval		0:FAIL
  *			other:len(SUCCESS)
  */
uint32_t RAND_Rand(void *rand, uint32_t bytes);

uint32_t RAND_RandP(void *rand, uint32_t bytes, void *p_rng);

/**
  * @brief	check whether the data meet the requirement
  * @param	rand	:random data buf
  * @param	bytes	:range: 16 < bytes < 784
  * @retval		:RET_RAND_CHECK_FAILURE
  *			:RET_RAND_CHECK_SUCCESS
  *			:RET_RAND_CHECK_DATA_LENGTH_ERROR
  */
uint32_t RAND_RandCheck(void *rand, uint32_t bytes);


uint32_t RAND_Init(void);




#endif

