/*
File Name    : yc_rtc.c
Author       : Yichip
Version      : V1.0
Date         : 2020/02/19
Description  : RTC encapsulation.
*/
#include "yc_rtc.h"

#define RTC_TIMER_ENABLE_BIT        17
#define RTC_TIMEOUT_INTR_STATUS_BIT 31

void RTC_SetRefRegister(uint32_t RefRegValue)
{
    lpm_write(LPM_RTC_CNT, RefRegValue);
}

uint32_t RTC_GetRefRegister(void)
{
    return (lpm_read(LPM_RTC_CNT));
}

void RTC_SetWakeUpCounter(uint32_t cnt)
{
    lpm_write(LPM_WKUP_TIMER, cnt);
}

void RTC_Config(FunctionalState NewState)
{
    uint32_t tmp = 0;
    tmp = lpm_read(LPM_GPIO_WKHI);

    if (NewState == ENABLE)
    {
        tmp |= (1 << RTC_TIMER_ENABLE_BIT);
        lpm_write(LPM_GPIO_WKHI, tmp);
    }
    else
    {
        tmp &= (~(1 << RTC_TIMER_ENABLE_BIT));
        lpm_write(LPM_GPIO_WKHI, tmp);
    }
}

ITStatus RTC_GetITStatus(void)
{
    uint32_t tmp = lpm_read(LPM_STATUS);
    if ((tmp >> 31) & 1)
        return  SET;
    else
        return RESET;
}

void RTC_Set_SecMax(uint16_t secmax)
{
    (*LPM_SECMAX) = secmax;
    while (SYSCTRL_LPM_STATUS & 0x40);
}

void RTC_ClearITPendingBit(void)
{
    RTC_Config(DISABLE);
    RTC_Config(ENABLE);
    uint32_t temp = lpm_read(LPM_CLR_INTR);
    temp &= (~(0xff));
    temp |= 0x6c;
    lpm_write(LPM_CLR_INTR, temp);
}

uint16_t RTC_Get_SexMax(void)
{
    *(volatile byte *)0xc8042 = 0xc4;
    *(volatile byte *)0xc800a = 0x80;
    delay(10000000);
    int mem_810a = *(volatile byte *)0xc810a;
    mem_810a += (*(volatile byte *)0xc810b) << 8;
    mem_810a += (*(volatile byte *)0xc810c) << 16;
    return 49152000000 / mem_810a;
}

void RTC_Calibration(void)
{
    RTC_Set_SecMax(RTC_Get_SexMax());
}
