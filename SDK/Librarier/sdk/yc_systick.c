/*
File Name    : yc_systick.c
Author       : Yichip
Version      : V1.0
Date         : 2019/12/9
Description  : systick encapsulation.
*/

#include "yc_systick.h"

#define ONE_US (CPU_MHZ/1000000)
#define ONE_MS (CPU_MHZ/1000)

tick SystickCount;

static void sysTick_delay_ms(uint32_t ms)//MS    MAX:0.35s
{
    _ASSERT(IS_DELAY_MS_VALUE(ms));

    uint32_t ReloadValue = ms * ONE_MS;
    ReloadValue -= 1;

    *SYSTICK_RVR = ReloadValue; //Set the reload value
    *SYSTICK_CVR = 0; //clear the current value
    *SYSTICK_CSR &= ~(1 << SYSTICK_CSR_TICKINT);
    *SYSTICK_CSR |= ((SYSTICK_SYSCLOCK << SYSTICK_CSR_CLKSOURCE) | \
                     (1 << SYSTICK_CSR_ENABLE));
    while (!((*SYSTICK_CSR) >> 16) & 0x01);

    *SYSTICK_CSR &= ~SYSTICK_CSR_ENABLE;
}

void SysTick_Delay_Ms(uint32_t nms)
{
    uint32_t repeat = nms / 300;
    uint32_t remain = nms % 300;
    while (repeat)
    {
        sysTick_delay_ms(300);
        repeat--;
    }
    if (remain)
        sysTick_delay_ms(remain);
}

void SysTick_Delay_Us(uint32_t us)//US   MAX: 0.35s
{
    _ASSERT(IS_DELAY_US_VALUE(us));

    uint32_t ReloadValue = us * ONE_US;
    ReloadValue -= 1;

    *SYSTICK_RVR = ReloadValue; //Set the reload value
    *SYSTICK_CVR = 0; //clear the current value
    *SYSTICK_CSR &= ~(1 << SYSTICK_CSR_TICKINT);
    *SYSTICK_CSR |= ((SYSTICK_SYSCLOCK << SYSTICK_CSR_CLKSOURCE) | \
                     (1 << SYSTICK_CSR_ENABLE));
    while (!((*SYSTICK_CSR) >> 16) & 0x01);

    *SYSTICK_CSR &= ~SYSTICK_CSR_ENABLE;
}


void SysTick_Config(uint32_t ReloadValue)
{
    _ASSERT(IS_RELOAD_VALUE(ReloadValue));

    ReloadValue -= 1;

    *SYSTICK_CSR &= ~(((uint32_t)1) << SYSTICK_CSR_ENABLE);

    *SYSTICK_RVR = ReloadValue;//Set the reload value
    *SYSTICK_CVR = 0; //clear the current value
    SystickCount = 0;// Reset the overflow counter
    *SYSTICK_CSR |= ((SYSTICK_SYSCLOCK << SYSTICK_CSR_CLKSOURCE) | \
                     (1 << SYSTICK_CSR_ENABLE) | \
                     (1 << SYSTICK_CSR_TICKINT));
}

void SysTick_disable()
{
    *SYSTICK_CSR &= ~(((uint32_t)1) << SYSTICK_CSR_ENABLE);

    *SYSTICK_RVR = 0;//Set the reload value
    *SYSTICK_CVR = 0; //clear the current value
    SystickCount = 0;// Reset the overflow counter
    *SYSTICK_CSR = 0;
}

tick SysTick_GetTick(void)
{
    return SystickCount;
}

Boolean SysTick_IsTimeOut(tick start_tick, int interval)
{
    start_tick = SysTick_GetTick() - start_tick;
    if (start_tick < 0)
        start_tick += TICK_MAX_VALUE;
    if (((start_tick * (*SYSTICK_RVR)) / (CPU_MHZ / 1000)) >= interval)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

uint32_t SysTick_GetRelativeTime(tick start_tick)
{
    start_tick = SysTick_GetTick() - start_tick;
    if (start_tick < 0)
        start_tick += TICK_MAX_VALUE;
    return ((start_tick * (*SYSTICK_RVR)) / (CPU_MHZ / 1000));
}
