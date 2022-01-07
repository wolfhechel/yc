#include "halBasics.h"
#include "yc_systick.h"

void halMsDelay(u32 uiMs)
{
	u32 getCurt = SysTick_GetTick();
	while(getCurt- SysTick_GetTick()<uiMs);
	//delay_us(uiMs*1000);
}

void halUsDelay (u32 us)
{
	delay_us(us);
}

void halClkCoreFreqSet(u32  uiFreq)
{
	u32 HCLK_Div = (CRYSTAL_CLK)/ uiFreq-2;
	SYSCTRL_HCLKConfig(HCLK_Div);
}
void halInitInterrupt(void)
{
	for(int id=0;id<32;id++)
	{
		disable_intr(id);
	}
}

void halEnableIrq(u32 uiID)
{
	enable_intr(uiID);
}
void hal_DisableIrq(u32 id)
{
	disable_intr(id);
}
void halGetMcuID(u8 *pucMcdID)
{
	read_chipid(pucMcdID);
}


