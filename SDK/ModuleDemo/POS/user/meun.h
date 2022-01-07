#ifndef __YC_MEUN_H__
#define __YC_MEUN_H__

#include "font.h"
#include "stdlib.h"
#include "yc3121.h"
#include "yc_lpm.h"
#include "yc_7816.h"
#include "yc_adc.h"
#include "yc_kscan.h"
#include "yc_st7789.h"
#include "yc_7816.h"
#include "yc_msr.h"
#include "yc_touch.h"
#include "yc_bt.h"
#include "yc_qspi.h"
#include "yc_systick.h"
#include "yc_emv_contactless_l1.h"
#include "yc_nfc_common.h"

typedef enum
{ 
	NFC_CS = 0x01,
	TFT_CS = 0x02,
}Enable_CS_TypeDef;

void Display_InterfaceWindow(void);
void MeunState_Check_Update(void);
void TFT_SPI_Init(void);
void BEEP_PWM_Init(void);
void TFT_Power_scan(void);
void TFT_Single_scan(void);
#if (VERSIONS == EXIST_BT)
void app_bt_init(void);
void show_bt_status(Boolean status);
#endif
#endif
