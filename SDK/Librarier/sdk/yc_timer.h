/**
  ******************************************************************************
  * @file    yc_timer.h
  * @author  Yichip
  * @version V1.0
  * @date    27-March-2018
  * @brief   timer encapsulation.
  ******************************************************************************
  * @attention
  *
  * Timer and pwm use the same hardware resources,Avoid conflict when using
  ******************************************************************************
  */

#ifndef __YC_TIMER_H__
#define __YC_TIMER_H__

#include "yc3121.h"
#include "yc_gpio.h"

/**
  * @brief  timer number Structure definition
  */
typedef enum
{
    TIM0 = 0,
    TIM1,
    TIM2,
    TIM3,
    TIM4,
    TIM5,
    TIM6,
    TIM7,
    TIM8
} TIM_NumTypeDef;
#define ISTIMERNUM(TIMx) (TIMx<PWM_TOTAL)

/**
  * @brief  timer Init Structure definition
  */
typedef struct
{
    TIM_NumTypeDef TIMx;
    uint32_t period;
} TIM_InitTypeDef;

/**
  * @brief  PWM Init Structure definition
  */
typedef struct
{
    TIM_NumTypeDef TIMx;
    uint32_t LowLevelPeriod;
    uint32_t HighLevelPeriod;
    GPIO_OutputTypeDef SatrtLevel;
} PWM_InitTypeDef;

typedef enum
{
    TIM_Mode_PWM	= 0,
    TIM_Mode_TIMER	= 1
} TIM_ModeTypeDef;
#define IS_TIM_MODE(mode)		(mode == TIM_Mode_TIMER || mode == TIM_Mode_PWM)

/**
 * @brief  Blocking delay
 *
 * @param  us : the delay time,Unit of Microsecond
 *
 * @retval none
 */
void delay_us(int us);

/**
 * @brief Blocking delay
 *
 * @param ms : the delay time,Unit of milliseconds
 *
 * @retval none
 */
void delay_ms(int ms);

/**
 * @brief  Initialize TIMx ,not start timer,use TIM_Cmd start timer
 *
 * @param  TIM_init_struct : the TIM_InitTypeDef Structure
 *
 * @retval none
 */
void TIM_Init(TIM_InitTypeDef *TIM_init_struct);

/**
 * @brief  DeInit TIM
 *
 * @retval none
 */
void TIM_DeInit(void);

/**
 * @brief  enable or disable timer
 *
 * @param  TIMx : the timer number,TIM0-TIM5
 *
 * @param  NewState :DISABLE or ENABLE
 *
 * @retval none
 */
void TIM_Cmd(TIM_NumTypeDef TIMx, FunctionalState NewState);

/**
 * @brief  Configure timer module mode
 *
 * @param  TIMx : the timer number,TIM0-TIM5
 *
 * @param  TIM_ModeTypeDef :TIM_Mode_TIMER	or TIM_Mode_PWM
 *
 * @retval none
 */
void TIM_ModeConfig(TIM_NumTypeDef TIMx, TIM_ModeTypeDef TIM_Mode);

/**
 * @brief  Configure timer Period
 *
 * @param  TIMx : the timer number,TIM0-TIM5
 *
 * @param  Period :the timer Period(Reload value)
 *
 * @retval none
 */
void TIM_SetPeriod(TIM_NumTypeDef TIMx, uint32_t Period);

/**
 * @brief  Initialize TIMx into PWM mode,only Initialize,use TIM_Cmd start putout PWM
 *
 * @param  PWM_init_struct : the PWM_InitTypeDef Structure
 *
 * @retval none
 */
void TIM_PWMInit(PWM_InitTypeDef *PWM_init_struct);

/**
 * @brief  Configure PWM Period
 *
 * @param  TIMx : the timer number,TIM0-TIM5
 *
 * @param  LowLevelPeriod :the PWM low level Period(Reload value)
 *
 * @param  HighLevelPeriod :the PWM high level Period(Reload value)
 *
 * @retval none
 */
void TIM_SetPWMPeriod(TIM_NumTypeDef TIMx, uint32_t LowLevelPeriod, uint32_t HighLevelPeriod);

/**
 * @brief  Configure PWM Differential output
 *
 * @param  TIMx : the timer number of output PWM1(TIM0-TIM5)
 *
 * @param  TIMy : the timer number of output PWM2(TIM0-TIM5)
 *
 * @param  LowLevelPeriod :the PWM1 low level Period(Reload value)
 *
 * @param  HighLevelPeriod :the PWM1 high level Period(Reload value)
 *
 * @retval none
 */
void TIM_PWMDifferential(TIM_NumTypeDef TIMx, TIM_NumTypeDef TIMy, uint32_t LowLevelPeriod, uint32_t HighLevelPeriod);

#endif /*__YC_TIMER_H__*/
