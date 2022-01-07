#include "yc_chrg.h"

#define CHRG_IND_BIT	28
#define CHRG_WEAKUP_BIT	30

void CHARGE_VolSel(uint8_t ChargOutVol)
{
	uint32_t temp = 0;
	enable_clock(CLKCLS_BT);

	temp = (*(CHARG_VOL_ADDR+0));
	temp &= ~(7<<4);
	temp|=	(ChargOutVol<<4);

	temp |= ((*(CHARG_VOL_ADDR+1) <<8)); delay(10);
	temp |= (*(CHARG_VOL_ADDR+2)<<16); delay(10);
	temp |= (*(CHARG_VOL_ADDR+3)<<24);	 delay(10);
	lpm_bt_write(6,temp);


}

void CHARGE_CurSel(uint8_t ChargCurtSel)
{

	uint32_t temp = 0;
	uint32_t chgr_rup = 0;
	uint32_t chgr_rdn = 0;
	enable_clock(CLKCLS_BT);

	switch(ChargCurtSel)
	{
		case CHARG_CURRENT_94mA:
			chgr_rup = 0x07;
			chgr_rdn = 0x00;
		break;
		case CHARG_CURRENT_78mA:
			chgr_rup = 0x03;
			chgr_rdn = 0x00;
		break;
		case CHARG_CURRENT_72mA:
			chgr_rup = 0x01;
			chgr_rdn = 0x00;
		break;
		case CHARG_CURRENT_69mA:
			chgr_rup = 0x00;
			chgr_rdn = 0x00;
		break;

		case CHARG_CURRENT_120mA:
			chgr_rup = 0x07;
			chgr_rdn = 0x04;
		break;
		case CHARG_CURRENT_137mA:
			chgr_rup = 0x07;
			chgr_rdn = 0x06;
		break;

		case CHARG_CURRENT_147mA:
			chgr_rup = 0x07;
			chgr_rdn = 0x07;
		break;
	}

	delay(2000);

	temp = (*(CHARG_VOL_ADDR+0));

	temp |= ((*(CHARG_VOL_ADDR+1) <<8)); delay(10);
	temp |= (*(CHARG_VOL_ADDR+2)<<16); delay(10);
	temp |= (*(CHARG_VOL_ADDR+3)<<24);	 delay(10);

	temp&=~(0x07<<7);
	temp|= chgr_rdn<<7;

	temp&=~(0x07<<13);
	temp|=(chgr_rup<<13);

	lpm_bt_write(6,temp);
}


uint8_t CHARGE_InsertDet(void)
{
	if(  (SYSCTRL_STATUS>> 30)&1)
	{
		return CHARGE_IN;
	}
	else
	{
		return CHANGE_OUT;
	}
}


uint8_t CHARGE_State(void)
{
	uint32_t statu = SYSCTRL_STATUS;

	if (!((statu >> CHRG_WEAKUP_BIT) & 0x1))
		return CHARG_CUTOFF ; //未插入USB
	else if (((statu >> CHRG_IND_BIT) & 0x1) &&((statu >> CHRG_WEAKUP_BIT) & 0x1))
		return CHARG_ACTIVE; //USB插入，充电
	else if (!((statu >> CHRG_IND_BIT) & 0x1))
		return CHARG_INACTIVE; //未充电
}
