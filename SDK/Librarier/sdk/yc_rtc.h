/*
File Name    : yc_rtc.h
Author       : Yichip
Version      : V1.0
Date         : 2019/12/13
Description  : RTC encapsulation.
*/

#ifndef __YC_RTC_H__
#define __YC_RTC_H__

#include "yc3121.h"
#include "yc_lpm.h"

/**
 * @brief set RTC reference Register
 *
 * @param RefRegValue:reference Register value
 *
 * @retval none
 */
void RTC_SetRefRegister(uint32_t RefRegValue);

/**
 * @brief Get RTC reference Register
 *
 * @param RefRegValue:reference Register value
 *
 * @retval RTC reference Register
 */
uint32_t RTC_GetRefRegister(void);

/**
 * @brief set RTC WakeUp time(s)
 *
 * @param wake_RefRegValue:when RefRegValue==wake_RefRegValue WakeUp and into interrupt
 *
 * @retval none
 */
void RTC_SetWakeUpCounter(uint32_t wake_RefRegValue);

/**
 * @brief clear RTC interrupt
 *
 * @param none
 *
 * @retval none
 */
void RTC_ClearITPendingBit(void);

/**
 * @brief get RTC interrupt status
 *
 * @param none
 *
 * @retval RTC interrupt status
 */
ITStatus RTC_GetITStatus(void);

/**
 * @brief Set max count for one second
 *
 * @param secmax:count value.
 *
 * @retval none
 *@
 */
void RTC_Set_SecMax(uint16_t secmax);

/**
 * @brief Config RTC
 *
 * @param NewState:Ennable or disable RTC.
 *
 * @retval none
 */
void RTC_Config(FunctionalState NewState);

/**
 * @brief Get RTC SecMax through 24M crystal.
 *
 * @param none
 *
 * @retval SecMax
 */
uint16_t RTC_Get_SexMax(void);

/**
 * @brief RTC Counter base calibration.
 *
 * @param none
 *
 * @retval none
 */
void RTC_Calibration(void);

#endif   /* __YC_RTC_H__ */
