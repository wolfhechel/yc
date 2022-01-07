/*
File Name  :yc_chrg.h
Author       : Yichip
Version      : V1.0
Date         : 2019/12/12
Description  : Charger  encapsulation.	
*/

#ifndef __CHARG_H_
#define __CHARG_H_

#include "yc3121.h"
#include "yc_lpm.h"


/***************Charger CV mode output voltage coarse-tune control in 180mV/steps*/
/**CHARG_OUT_DEFINE**/
#define CHARG_OUT_4D19V 0x00
#define CHARG_OUT_4D36V 0x04
#define CHARG_OUT_4D54V 0x06
#define CHARG_OUT_4D74V 0x07
#define CHARG_OUT_DEFAULT CHARG_OUT_436V


/*CHARGE_IND STATE DEFINE*/
#define CHARGE_IN   1
#define CHANGE_OUT  0

/*CHARGE WORKING STATE DEFINE*/
#define CHARG_ACTIVE 0//充电中
#define CHARG_CUTOFF 1 //未插入USB
#define CHARG_INACTIVE 2  //已充满，充电截止


/****************************
chgr_rup   chgr_rdn    I_charge
111        000         94.91mA
011        000         78.26mA
001        000         72.47mA
000        000         69.87mA
111        100         120.5mA
111        110         137.4mA
111        111         147.9mA
************************************/
#define CHARG_CURRENT_94mA 	0
#define CHARG_CURRENT_78mA	1
#define CHARG_CURRENT_72mA 	2
#define CHARG_CURRENT_69mA 	3
#define CHARG_CURRENT_120mA 4
#define CHARG_CURRENT_137mA 5
#define CHARG_CURRENT_147mA 6



#define CHARG_VOL_ADDR		((volatile uint8_t *)( 0xc812c))



/**
  * @brief	Config charge voltage output
  * @param	the para will be macro CHARG_OUT_DEFINE
  * @retval 	none
  */
void CHARGE_VolSel(uint8_t ChargOutVol);


/**
  * @brief	Config charge current
  * @param	the para will be macro CHARG_OUT_DEFINE
  * @retval 	none
  */
void CHARGE_CurSel(uint8_t ChargCurtSel);



/**
  * @brief	Detect charg_in statu
  * @param	none
  * @retval 	CHARGE_IN:charg_in high,CHARGE_out,charg_low
  */
uint8_t CHARGE_InsertDet(void);



/**
  * @brief	Get Chager working status.
  * @param	none
  * @retval 	none
  */
uint8_t CHARGE_State(void);


#endif
