/*
File Name    : yc_adc.h
Author       : Yichip
Version      : V1.2
Notes        :  1. add ADC_Calibration_Para function
                2. calculate_voltage absolute value return it
Date         : 2021/08/02
Description  : ADC encapsulation.
*/

#ifndef __YC_ADC_H__
#define __YC_ADC_H__

#include "yc3121.h"
#include "yc_otp.h"
#include "yc_ipc.h"

#define IO_LOW_VOLTAGE_REF      512
#define IO_HIHG_VOLTAGE_REF     1024
#define HVIN_LOW_VOLTAGE_REF    3300
#define HVIN_HIHG_VOLTAGE_REF   5000
#define VINLPM_LOW_VOLTAGE_REF  2000
#define VINLPM_HIHG_VOLTAGE_REF 3000
#define IO_2_4_LOW_VOLTAGE_REF  1024
#define IO_2_4_HIGH_VOLTAGE_REF 2048
#define ADC_REF_OTP_ADDR 0x012c

#define FILT_NUM   20
#define REMOVE_NUM 6

#define WAIT_TIME 100

/* ADC Channel select */
typedef enum
{
    ADC_CHANNEL_0 = 0,
    ADC_CHANNEL_1,
    ADC_CHANNEL_2,
    ADC_CHANNEL_3,
    ADC_CHANNEL_4,
    ADC_CHANNEL_5,
    ADC_CHANNEL_6,
    ADC_CHANNEL_7,
} ADC_ChxTypeDef;

#define IS_ADC_CHANNEL(CHANNEL_NUM) (((CHANNEL_NUM) == ADC_CHANNEL_0) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_1) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_2) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_3) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_4) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_5) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_6)||\
                                     ((CHANNEL_NUM) == ADC_CHANNEL_7))

typedef enum
{
    ADC_GPIO = 0,
    ADC_DIFF,
    ADC_HVIN,
    ADC_VINLPM,
} ADC_ModeTypeDef;

#define IS_ADC_MODE(MODE)  ((MODE == ADC_GPIO)||\
                            (MODE == ADC_DIFF)||\
                            (MODE == ADC_HVIN)||\
                            (MODE == ADC_VINLPM))

typedef enum
{
    ADC_Scale_defult = 2,
    ADC_Scale_1200Mv =2,
    ADC_Scale_2400Mv = 1,
} ADC_ScaleTypeDef;
#define IS_ADC_SCALE(SCALE) ((SCALE == ADC_Scale_1200Mv)||\
                             (SCALE == ADC_Scale_2400Mv))
/* ADC Struct Define*/
typedef struct _ADC_InitTypeDef
{
    ADC_ChxTypeDef  ADC_Channel;            /* ADC Channel select */
    ADC_ModeTypeDef ADC_Mode;
    ADC_ScaleTypeDef ADC_Scale;
} ADC_InitTypeDef;

/**
 * @brief Initialize adc module
 *
 * @param ADC_InitStruct : piont to ADC_InitTypeDef struct
 *
 * @retval none
 */
void ADC_Init(ADC_InitTypeDef *ADC_InitStruct);

/**
 * @brief  Get ADC result
 *
 * @param  ADC_Channel : In gpio mode ,the para can be every value of ADC_ChxTypeDef,
 *                       in gpio diff mode, the param can be ADC_CHANNEL_0 to ADC_CHANNEL_3,and in other mode,
 *                       the param can be 0.
 * @retval 0:fail other:adc value
 */
uint16_t ADC_GetResult(ADC_ChxTypeDef ADC_Channel);

/**
 * @brief  Get voltage
 * @param  ADC_Channel: ADC_Channel Select channel,
 * @retval -1:fail;  other:adc voltage (mV);
 */
int ADC_GetVoltage(ADC_ChxTypeDef ADC_Channel);

/**
 * @brief  Median and arithmetic mean filtering
 * @param  ADC_Channel: ADC_Channel Select channel,select ADC_CHANNEL_0 in hvin or lpmhvin mode
 * @retval The filtered result,-1:get result failed
 */
int ADC_Filt(ADC_ChxTypeDef ADC_Channel);

#endif   /* __YC_ADC_H__ */
