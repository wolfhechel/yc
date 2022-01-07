#include "yc_timer.h"
#include "rom_api.h"

void delay_us(int us)
{
    ((void(*)(int))(FUNC_DELAY_US_ADDR))(us);
}

void delay_ms(int ms)
{
    ((void(*)(int))(FUNC_DELAY_MS_ADDR))(ms);
}

void TIM_Init(TIM_InitTypeDef *TIM_init_struct)
{
    _ASSERT(ISTIMERNUM(TIM_init_struct->TIMx));

    TIM_Cmd(TIM_init_struct->TIMx, DISABLE);
    TIM_PCNT(TIM_init_struct->TIMx) = TIM_init_struct->period;
    if (TIM_init_struct->TIMx < TIM8)
        TIM_CTRL |= (((((uint32_t)1) << TIM_CTRL_MODE) | (((uint32_t)1) << TIM_CTRL_AUTO_RELOAD)) << TIM_init_struct->TIMx * 4);
    else
        TIM_CTRL1 |= (((((uint32_t)1) << TIM_CTRL_MODE) | (((uint32_t)1) << TIM_CTRL_AUTO_RELOAD)) << 0);
}

void TIM_DeInit()
{
    disable_clock(CLKCLS_TIM);
}

void TIM_Cmd(TIM_NumTypeDef TIMx, FunctionalState NewState)
{
    _ASSERT(ISTIMERNUM(TIMx));

    if (NewState == ENABLE)
    {
        if (TIMx < TIM8)
            TIM_CTRL |= ((((uint32_t)1) << TIM_CTRL_ENABLE) << TIMx * 4);
        else
            TIM_CTRL1 |= ((((uint32_t)1) << TIM_CTRL_ENABLE) << 0);
    }
    else
    {
        if (TIMx < TIM8)
            TIM_CTRL &= ~((((uint32_t)1) << TIM_CTRL_ENABLE) << TIMx * 4);
        else
            TIM_CTRL1 &= ~((((uint32_t)1) << TIM_CTRL_ENABLE) << 0);
    }
}

void TIM_ModeConfig(TIM_NumTypeDef TIMx, TIM_ModeTypeDef TIM_Mode)
{
    _ASSERT(ISTIMERNUM(TIMx));
    _ASSERT(IS_TIM_MODE(TIM_Mode));

    if (TIM_Mode == TIM_Mode_TIMER)
    {
        if (TIMx < TIM8)
            TIM_CTRL |= ((((uint32_t)1) << TIM_CTRL_MODE) << TIMx * 4);
        else
            TIM_CTRL1 |= ((((uint32_t)1) << TIM_CTRL_MODE) << 0);
    }
    else
    {
        if (TIMx < TIM8)
            TIM_CTRL &= ~((((uint32_t)1) << TIM_CTRL_MODE) << TIMx * 4);
        else
            TIM_CTRL1 &= ~((((uint32_t)1) << TIM_CTRL_MODE) << 0);
    }
}

void TIM_SetPeriod(TIM_NumTypeDef TIMx, uint32_t Period)
{
    _ASSERT(ISTIMERNUM(TIMx));

    TIM_PCNT(TIMx) = Period;
}

void TIM_PWMInit(PWM_InitTypeDef *PWM_init_struct)
{
    _ASSERT(ISTIMERNUM(PWM_init_struct->TIMx));

    TIM_Cmd(PWM_init_struct->TIMx, DISABLE);
    TIM_PCNT(PWM_init_struct->TIMx) = PWM_init_struct->HighLevelPeriod;
    TIM_NCNT(PWM_init_struct->TIMx) = PWM_init_struct->LowLevelPeriod;
    if (PWM_init_struct->SatrtLevel == OutputHigh)
    {
        if (PWM_init_struct->TIMx < TIM8)
            TIM_CTRL |= ((((uint32_t)1) << TIM_CTRL_START_LEVEL) << PWM_init_struct->TIMx * 4);
        else
            TIM_CTRL1 |= ((((uint32_t)1) << TIM_CTRL_START_LEVEL) << 0);
    }
    else
    {
        if (PWM_init_struct->TIMx < TIM8)
            TIM_CTRL &= ~((((uint32_t)1) << TIM_CTRL_START_LEVEL) << PWM_init_struct->TIMx * 4);
        else
            TIM_CTRL1 &= ~((((uint32_t)1) << TIM_CTRL_START_LEVEL) << 0);
    }
    if (PWM_init_struct->TIMx < TIM8)
        TIM_CTRL &= ~((((uint32_t)1) << TIM_CTRL_MODE) << PWM_init_struct->TIMx * 4);
    else
        TIM_CTRL1 &= ~((((uint32_t)1) << TIM_CTRL_MODE) << 0);
}

void TIM_SetPWMPeriod(TIM_NumTypeDef TIMx, uint32_t LowLevelPeriod, uint32_t HighLevelPeriod)
{
    _ASSERT(ISTIMERNUM(TIMx));

    TIM_PCNT(TIMx) = HighLevelPeriod;
    TIM_NCNT(TIMx) = LowLevelPeriod;
}

void TIM_PWMDifferential(TIM_NumTypeDef TIMx, TIM_NumTypeDef TIMy, uint32_t LowLevelPeriod, uint32_t HighLevelPeriod)
{
    _ASSERT(ISTIMERNUM(TIMx));
    _ASSERT(ISTIMERNUM(TIMy));

    uint32_t TDifferentialConfig;

    TIM_CTRL &= ~((1 << (TIMy * 4)) | (1 << (TIMx * 4)));
    TIM_PCNT(TIMx) = HighLevelPeriod;
    TIM_NCNT(TIMx) = LowLevelPeriod;
    TIM_PCNT(TIMy) = LowLevelPeriod;
    TIM_NCNT(TIMy) = HighLevelPeriod;
    TIM_CTRL &= ~(1 << (TIMx * 4 + 1));
    TDifferentialConfig = TIM_CTRL;
    TDifferentialConfig |= ((3 << (TIMy * 4)) | (1 << (TIMx * 4)));
    TIM_CTRL |= 1 << (TIMy * 4 + 1);
    delay((LowLevelPeriod - 8) / 4);
    TIM_CTRL = TDifferentialConfig;
}
