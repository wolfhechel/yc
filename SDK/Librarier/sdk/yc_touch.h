/*
File Name    : yc_touch.h
Author       : Yichip
Version      : V1.0
Date         : 2020/1/14
Description  :
*/

#ifndef __YC_TOUCH_H__
#define __YC_TOUCH_H__

#include "yc_st7789.h"
#include "board_config.h"
#include "yc_adc.h"

//#define TP_DEBUG
#if defined(TP_DEBUG)
#define TP_PRINT        MyPrintf
#else
#define TP_PRINT(x,...)
#endif

#define TP_LEVEL_DIFF  (30)
#define Tp_AbsolouteDifference(val_a,val_b) (val_a > val_b ? val_a - val_b : val_b - val_a)
#define Tp_Get_Diff_Result(diff_value,com_value)    (diff_value > com_value ? FALSE : TRUE)

typedef enum
{
    X_AXIS = 0,
    Y_AXIS
}TP_Axis_TypeDef;

typedef struct
{
    uint16_t x;
    uint16_t y;
}Tp_Calibrate_CB;

typedef struct
{
    uint16_t kx;
    uint16_t ky;
}Tp_CalibrateSlope_CB;

typedef struct
{
    uint16_t nx; /*now x value*/
    uint16_t ny; /*now y value*/
    uint16_t lx; /*last x value*/
    uint16_t ly; /*last y value*/
    int pressflag; /*Press the tag*/
}Tp_Sign_Para_TypeDef;

#define REGULT_ADC_MAX      (0x7fe)

#define TOP_SIZE_X (220)
#define TOP_SIZE_Y (190)

#define LEFT_UP_X (50)
#define LEFT_UP_Y (50)

#define RIGHT_UP_X  (LEFT_UP_X + TOP_SIZE_X)
#define RIGHT_UP_Y  LEFT_UP_Y

#define LEFT_DOWN_X  LEFT_UP_X
#define LEFT_DOWN_Y  TOP_SIZE_Y

#define RIGHT_DOWN_X  (LEFT_UP_X + TOP_SIZE_X)
#define RIGHT_DOWN_Y  TOP_SIZE_Y

#define MIDDLE_X  (LEFT_UP_X + TOP_SIZE_X/2)
#define MIDDLE_Y  (LEFT_UP_Y + TOP_SIZE_Y/2)


#define TP_CALIPONT_X  (160)
#define TP_CALIPONT_Y  (145)
#define TP_FAC_KX      (270)
#define TP_FAC_KY      (363)
#define TP_PEGPONT_X   (0x613)
#define TP_PEGPONT_Y   (0x5a8)
#define TP_CONSTANT_X  (1987)
#define TP_CONSTANT_Y  (1974)

extern Tp_Calibrate_CB DisplaySample[5];
extern Tp_Calibrate_CB ScreenSample[5];
extern Tp_Calibrate_CB PegPont;
extern Tp_Calibrate_CB CaliPont;
extern Tp_CalibrateSlope_CB fac;
extern Tp_CalibrateSlope_CB Constant;

#define PRESS_DETECT_CHANNEL    ADC_CHANNEL_6   /*The GPIO corresponding channel with pull-up mode needs to be selected to detect touch*/
#define Y_DETECT_CHANNEL        ADC_CHANNEL_6   /*Y axis AD detection channel*/
#define X_DETECT_CHANNEL        ADC_CHANNEL_7   /*X axis AD detection channel*/

Boolean Tp_Calibrate(void);
Boolean Tp_DetectPressing(void);
void Tp_GetAdXY(uint16_t *adx,uint16_t *ady);
void Tp_GetPoint(uint16_t *Disx,uint16_t*Disy);

#endif
