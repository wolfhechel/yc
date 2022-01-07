/*
File Name    : yc_systick.h
Author       : Yichip
Version      : V1.0
Date         : 2019/12/9
Description  : systick encapsulation.
*/

#ifndef __YC_SYSTICK_H__
#define __YC_SYSTICK_H__

#include "yc3121.h"

#define SYSTICK_SYSCLOCK        1
#define SYSTICK_HALF_SYSCLOCK   0

#define IS_RELOAD_VALUE(x)   (x <= 0xffffff)
#define IS_DELAY_US_VALUE(x) (x <= 349000)
#define IS_DELAY_MS_VALUE(x) (x <= 349)

typedef int32_t tick;
#define TICK_MAX_VALUE  (int32_t)0x7FFFFFFF

/**
 * @brief  SysTick_Delay_Ms
 *
 * @param  nms
 *
 * @retval none
 */
void SysTick_Delay_Ms(uint32_t nms);

/**
 * @brief  SysTick_Delay_Us
 *
 * @param  us
 *
 * @retval none
 */
void SysTick_Delay_Us(uint32_t us);

/**
 * @brief  Initialize systick and start systick
 *
 * @param  ReloadValue: the systick reload value
 *
 * @retval 0:succeed  1:error
 */
void SysTick_Config(uint32_t ReloadValue);

void SysTick_disable(void);

/**
 * @brief  get the current value of SystickCount(SystickCount plus one for every overflow interrupt)
 *
 * @param  none
 *
 * @retval current value of SystickCount
 */
tick SysTick_GetTick(void);

/**
 * @brief  Determine whether the timeout that millisecond.
 *
 * @param  start_tick:start tick
 *
 * @param  interval:time interval(ms)
 *
 * @retval TRUE is timeout ,FALSE is not timeout
 */
Boolean SysTick_IsTimeOut(tick start_tick, int interval);

/**
 * @brief  get relative time.
 *
 * @param  start_tick:start tick(start time)
 *
 * @retval the relative time(millisecond)
 */
uint32_t SysTick_GetRelativeTime(tick start_tick);

#endif   /* __YC_SYSTICK_H__ */
