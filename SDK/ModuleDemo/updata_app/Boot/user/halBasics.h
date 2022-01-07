#ifndef _HAL_BASIC_H_
#define _HAL_BASIC_H_

#include "..\core\yc3121.h"
#include "..\sdk\yc_timer.h"
#include "..\sdk\yc_otp.h"
#include "..\sdk\yc_sysctrl.h"
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define s32 int
#define s8 char

#define  NO_DATA 0	
#define  HAVE_DATA 1	
#define	HAL_OK			0
#define	HAL_ERROR      	-1
//#define	OK			0
//#define  ERROR      	0
//#define 	PARAMERR  	-2
//#define  TIMEOUT   	-3

//#define 	SUCCESS		0
//#define  FAILED      	-1

//#define  TRUE		0x01
//#define  FALSE		0x00

#define 	ON        	0x01
#define 	OFF        	0x00


void halMsDelay(u32 uiMs);
void halUsDelay(u32 uiUs);
void halClkCoreFreqSet(u32 uiFreq);
void halInitInterrupt(void);
void halEnableIrq(u32 uiID);
void halDisableIrq(u32 id);
void halGetMcuID(u8 *pucMcdID);

#endif
