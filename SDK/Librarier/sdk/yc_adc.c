/*
File Name    : yc_adc.c
Author       : Yichip
Version      : V1.2
Notes        :  1. add ADC_Calibration_Para function
                2. calculate_voltage absolute value return it
Date         : 2021/08/02
Description  : ADC encapsulation.
*/


#include "yc_adc.h"

#define ADC_RESULT (0x7FE&ADC_RDATA)


/*@Note        ：Otp parameter calibration channel select the channel in use.
                By default, GPIO40 and ADC_CHANNEL_3 are used as the calibration channels.*/
#define ADC_CALIBR_OPEN
#define ADC_1V_DIFF_VALUE       (0x20)
#define ADC_CALIB_0V_GPIO       (40)
#define ADC_CALIB_0V_CHANNEL    (ADC_CHANNEL_3)

extern Boolean ipc_inited;
static int calculate_voltage(int result, int vol_high_ref, int vol_low_ref, int vol_high, int vol_low);
uint8_t r_otp_ref_para[16];
static int io_vol_low_ref = 0;
static int io_vol_high_ref = 0;

static void ADC_Calibration_Para(void)
{
    int adc_0v_value = 0;
    int adc_1v_value = 0;
    uint8_t cnt = 5;

    /*get 0v ad value*/

    uint8_t gpio_func = GPIO_CONFIG(ADC_CALIB_0V_GPIO);
    GPIO_CONFIG(ADC_CALIB_0V_GPIO) = OUTPUT_LOW ;
    delay_ms(5);
    while(cnt -- )
    {
        adc_0v_value = ADC_GetResult(ADC_CALIB_0V_CHANNEL);
        if(adc_0v_value > 0x3e0 && adc_0v_value < 0x420)
        {
            break;
        }
        delay_ms(100);
    }
    GPIO_CONFIG(ADC_CALIB_0V_GPIO) = gpio_func;

    /*Calibration 1v para*/

    int k = ((((r_otp_ref_para[3] << 8) | r_otp_ref_para[2]) -  adc_0v_value)*1000)/ IO_LOW_VOLTAGE_REF;
    adc_1v_value =  (IO_LOW_VOLTAGE_REF * k + (((r_otp_ref_para[3] << 8) | r_otp_ref_para[2])*1000))/1000;

    int diff = adc_1v_value > ((r_otp_ref_para[9] << 8) | r_otp_ref_para[8])? adc_1v_value -  ((r_otp_ref_para[9] << 8) | r_otp_ref_para[8]): ((r_otp_ref_para[9] << 8) | r_otp_ref_para[8]) - adc_1v_value;
    if(diff > ADC_1V_DIFF_VALUE)
    {
        r_otp_ref_para[9] = r_otp_ref_para[3];
        r_otp_ref_para[8] = r_otp_ref_para[2];
        r_otp_ref_para[2] = adc_0v_value & 0xff;
        r_otp_ref_para[3] = adc_0v_value >> 8 & 0xff;
        io_vol_low_ref = 0;
        io_vol_high_ref = IO_LOW_VOLTAGE_REF;
    }
    else
    {
        io_vol_low_ref = IO_LOW_VOLTAGE_REF;
        io_vol_high_ref = IO_HIHG_VOLTAGE_REF;
    }
    return ;
}

void ADC_Init(ADC_InitTypeDef *ADC_InitStruct)
{
    _ASSERT(IS_ADC_CHANNEL(ADC_InitStruct->ADC_Channel));
    _ASSERT(IS_ADC_MODE(ADC_InitStruct->ADC_Mode));
    _ASSERT(IS_ADC_SCALE(ADC_InitStruct->ADC_Scale));
    /****************open dpll*******************/
    IpcInit();

    /****************config adc******************* */
    ADC_CTRL1 = 0x8f | ((ADC_InitStruct->ADC_Mode) << 4);
#if (VERSIONS == NO_BT)
    ADC_ENBLE &= ~(1 << 3);
    delay_us(2);
    ADC_ENBLE = 0x74;
    delay_us(2);
    ADC_ENBLE = 0x7c;
#endif
    ADC_CTRL0 = 0x7f;
    ADC_CTRL2 = 0xf8 | ADC_InitStruct->ADC_Scale;
    ADC_CTRL3 = 0x30 | ADC_InitStruct->ADC_Channel;

    //read otp reference
    read_otp(ADC_REF_OTP_ADDR, r_otp_ref_para, 16);

    delay_ms(100);

#ifdef  ADC_CALIBR_OPEN
    /*adc calibration*/
    ADC_Calibration_Para();
#else
    io_vol_low_ref = IO_LOW_VOLTAGE_REF;
    io_vol_high_ref = IO_HIHG_VOLTAGE_REF;
#endif

}

uint16_t  ADC_GetResult(ADC_ChxTypeDef ADC_Channel)
{
    if (ipc_inited == FALSE)	return 0;
    ADC_CTRL3 = 0x30 | ADC_Channel;
    delay_us(WAIT_TIME);
    return	ADC_RESULT;
}

int ADC_Filt(ADC_ChxTypeDef ADC_Channel)
{
    int8_t i, j;
    uint16_t ADC_Buffer[FILT_NUM], tmp;
    int cnt = 200;
    if (ipc_inited == FALSE)	return -1;
    ADC_CTRL3 = 0x30 | ADC_Channel;

    delay_us(WAIT_TIME);

    if (REMOVE_NUM * 2 >= FILT_NUM)
    {
        return -1;
    }

    for (i = 0; i < FILT_NUM; i++)
    {
        ADC_Buffer[i] = ADC_RESULT;
        if (ADC_Buffer[i] != 0)
        {
            continue;
        }
        else
        {
            i--;
            if (0 == cnt--)
                return -1;
        }
#ifdef SDK_DEBUG
        MyPrintf("ADC_result,%d ", ADC_RESULT);
#endif
    }
    for (j = 1; j < FILT_NUM; j++)
    {
        tmp = ADC_Buffer[j];
        i = j - 1;
        while (i >= 0 && ADC_Buffer[i] > tmp)
        {
            ADC_Buffer[i + 1] = ADC_Buffer[i];
            i--;
        }
        ADC_Buffer[i + 1] = tmp;
    }

    for (tmp = 0, i = REMOVE_NUM; i < FILT_NUM - REMOVE_NUM; i++)
    {
        tmp += ADC_Buffer[i];
    }

    return tmp / (FILT_NUM - 2 * REMOVE_NUM);
}

int ADC_GetVoltage(ADC_ChxTypeDef ADC_Channel)
{
    int v_rang = 0;
    int result = 0;
    int i = 0, j = 0;
    int voltage_low  = 2800;
    int voltage_high = 5000;
    int reg_voltage_low  = 640 * 1000;
    int ref_voltage_high  = 744 * 1000;
    int adc_result_buf[FILT_NUM] = {0};

    uint8_t getmode = (ADC_CTRL1 & 0x70) >> 4;
    uint8_t getscale = (ADC_CTRL2&3);
    if (ipc_inited == FALSE)	return -1;

    switch (getmode)
    {
    case ADC_GPIO:
        if(getscale == ADC_Scale_1200Mv)
        {
            reg_voltage_low  = ((r_otp_ref_para[3] << 8) | r_otp_ref_para[2]) * 1000;
            ref_voltage_high = ((r_otp_ref_para[9] << 8) | r_otp_ref_para[8]) * 1000;
            voltage_low = io_vol_low_ref;
            voltage_high = io_vol_high_ref;
            v_rang = 1200;
        }
        if(getscale == ADC_Scale_2400Mv)
        {
            reg_voltage_low  = ((r_otp_ref_para[13] << 8) | r_otp_ref_para[12]) * 1000;
            ref_voltage_high = ((r_otp_ref_para[7] << 8) | r_otp_ref_para[6]) * 1000;
            voltage_low = IO_2_4_LOW_VOLTAGE_REF;
            voltage_high = IO_2_4_HIGH_VOLTAGE_REF;
            v_rang = 2400;
        }
        break;
    case ADC_DIFF:
        if(getscale == ADC_Scale_1200Mv)
        {
            reg_voltage_low  = ((r_otp_ref_para[3] << 8) | r_otp_ref_para[2]) * 1000;
            ref_voltage_high = ((r_otp_ref_para[9] << 8) | r_otp_ref_para[8]) * 1000;
            voltage_low = io_vol_low_ref;
            voltage_high = io_vol_high_ref;
            v_rang = 1200;
        }
        if(getscale == ADC_Scale_2400Mv)
        {
            reg_voltage_low  = ((r_otp_ref_para[13] << 8) | r_otp_ref_para[12]) * 1000;
            ref_voltage_high = ((r_otp_ref_para[7] << 8) | r_otp_ref_para[6]) * 1000;
            voltage_low = IO_2_4_LOW_VOLTAGE_REF;
            voltage_high = IO_2_4_HIGH_VOLTAGE_REF;
            v_rang = 2400;
        }
        break;
    case ADC_HVIN:
        reg_voltage_low  = ((r_otp_ref_para[5] << 8) | r_otp_ref_para[4]) * 1000; //OTP中3.3v校准值
        ref_voltage_high = ((r_otp_ref_para[11] << 8) | r_otp_ref_para[10]) * 1000; //OTP中5.0V校准值
        voltage_low = HVIN_LOW_VOLTAGE_REF;
        voltage_high = HVIN_HIHG_VOLTAGE_REF;
        v_rang = 5500;
        break;

    case ADC_VINLPM:
        reg_voltage_low  = ((r_otp_ref_para[5] << 8) | r_otp_ref_para[4]) * 1000;
        ref_voltage_high = ((r_otp_ref_para[11] << 8) | r_otp_ref_para[10]) * 1000;
        voltage_low = VINLPM_LOW_VOLTAGE_REF;
        voltage_high = VINLPM_HIHG_VOLTAGE_REF;
        v_rang = 1200;
        break;
    default:
        return -1;
    }

    if (reg_voltage_low == 0 || ref_voltage_high == 0)
        return -1;

    while (i < FILT_NUM)//get FFILT_NUM  result
    {
        result =  ADC_GetResult(ADC_Channel);
        if (result > 0)
        {
            adc_result_buf[i++] = calculate_voltage(result, ref_voltage_high, reg_voltage_low, voltage_high, voltage_low);
        }
        if (++j > FILT_NUM + 10)
            return -1;    //debug whiel(1)
    }

    for (j = 1; j < FILT_NUM; j++)
    {
        result = adc_result_buf[j];
        i = j - 1;
        while (i >= 0 && adc_result_buf[i] > result)
        {
            adc_result_buf[i + 1] = adc_result_buf[i];
            i--;
        }
        adc_result_buf[i + 1] = result;
    }

    for (result = 0, i = REMOVE_NUM; i < FILT_NUM - REMOVE_NUM; i++)
    {
        result += adc_result_buf[i];
    }
    result /=  (FILT_NUM - 2 * REMOVE_NUM);
    return result > v_rang ? v_rang : result;
}

/**
  * @brief  calculate voltage ,acording reference value and reference voltage
  * @param  result       : adc result
  *         vol_high_ref : high voltage reference value
  *         vol_low_ref  : low voltage reference value
  *         vol_high     : high reference voltage (mV)
  *         vol_low      : low reference voltage
  * @retval voltage (mV)
  */
static int calculate_voltage(int result, int vol_high_ref, int vol_low_ref, int vol_high, int vol_low)
{
    int rega = result * 1000 - vol_low_ref;
    int regb = vol_high_ref - vol_low_ref;
    int regc = vol_high - vol_low;
    int temp = (rega * regc) / regb + vol_low;
    return temp > 0 ? temp : (0 - temp);
}
