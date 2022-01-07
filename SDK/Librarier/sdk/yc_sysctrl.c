#include "yc_sysctrl.h"

void SYSCTRL_AHBPeriphClockCmd(uint32_t SYSCTRL_AHBPeriph, FunctionalState NewState)
{
    _ASSERT(IS_SYSCTRL_AHB_PERIPH(SYSCTRL_AHBPeriph));
    if (NewState == DISABLE)
    {
        SYSCTRL_CLK_CLS |= SYSCTRL_AHBPeriph;
    }
    else
    {
        SYSCTRL_CLK_CLS &= ~SYSCTRL_AHBPeriph;
    }
}

void SYSCTRL_EnterSleep(SleepMode_TypeDef SleepMode)
{
    _ASSERT(IS_ALL_SLEEP_MODE(SleepMode));
    *LPM_SLEEP = 0x5a;
}

void __NOINLINE HCLKConfig_Div_None(uint32_t HCLK_Div)
{
    QSPI_CTRL  |= (0x80);
    int temp = (SYSCTRL_HCLK_CON & (~SYSCTRL_HCLK_COFG_REGBIT)) | HCLK_Div;
    SYSCTRL_HCLK_CON =  temp;
}

void SYSCTRL_HCLKConfig(uint32_t HCLK_Div)
{
    _ASSERT(IS_GET_SYSCTRL_HCLK_DIV(HCLK_Div));
    __asm("CPSID i");
    if (HCLK_Div == SYSCTRL_HCLK_Div_None)
    {
        uint32_t HCLKConfig_Div_None_addr = (uint32_t)HCLKConfig_Div_None;
        ((void(*)(void *, void *))FUNC_PREFETCH)(((uint32_t *)HCLKConfig_Div_None_addr), ((uint32_t *)(HCLKConfig_Div_None_addr + 64)));
        HCLKConfig_Div_None(HCLK_Div);
    }
    else
    {
        int temp = (SYSCTRL_HCLK_CON & (~SYSCTRL_HCLK_COFG_REGBIT)) | HCLK_Div;
        SYSCTRL_HCLK_CON =  temp;
    }
    __asm("CPSIE i");
}

void SYSCTRL_GetClocksFreq(SYSCTRL_ClocksTypeDef *SYSCTRL_Clocks)
{
    SYSCTRL_Clocks->HCLK_Frequency = CPU_MHZ;
}

void SYSCTRL_EnableDpllClk(void)
{
    enable_clock(CLKCLS_BT);
    SYSCTRL_ROM_SWITCH = 0x94;
    delay(500);

    *(volatile byte *)0xc4ab1 = 0x7f;
    *(volatile byte *)0xc4ab2 = 0xff;
    *(volatile byte *)0xc4ab3 = 0xff;

    *(volatile byte *)0xc40c3 = 0xb1;
    *(volatile byte *)0xc40c4 = 0x4a;

    BT_CLKPLL_EN = 0xff;
}
