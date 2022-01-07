#include "yc_power.h"

#define POWER_GPIO_WEAKUP_BIT 31
#define POWER_CHRG_WEAKUP_BIT 30

Boolean Power_Keyscan(void)
{
	uint32_t powerkeystat = SYSCTRL_STATUS;
	
	if (powerkeystat & ((uint32_t)0x1 << POWER_GPIO_WEAKUP_BIT))
	{
		return TRUE;
	}

	return FALSE;
}

Boolean Power_off(void)
{
	uint32_t data;
	uint32_t powerkeystat = SYSCTRL_STATUS;
	
	if(!(powerkeystat & ((uint32_t)0x1 << POWER_CHRG_WEAKUP_BIT)))
	{
		while(Power_Keyscan());
		data = lpm_bt_read(LPMBT_CONTROL0);
		lpm_bt_write(LPMBT_CONTROL0, data & 0xfffbffff);
	}

	return FALSE;
}
