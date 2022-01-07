#ifndef __YC_SSC_H
#define __YC_SSC_H

#include "yc3121.h"
#include "yc_lpm.h"
#include "type.h"

/* ssc interval*/
#define INTERVAL_999MS                    ((uint32_t)0)
#define INTERVAL_500MS                    ((uint32_t)1)
#define INTERVAL_250MS                    ((uint32_t)2)
#define INTERVAL_125MS                    ((uint32_t)3)
#define IS_INTERVAL(ITV)                  ((ITV == INTERVAL_999MS) | \
                                           (ITV == INTERVAL_500MS) | \
                                           (ITV == INTERVAL_250MS) | \
                                           (ITV == INTERVAL_125MS))

/* tamper port mode*/
#define TAMPER_Port_None_active           ((uint32_t)0x0000)
#define TAMPER_Port_S01_active            ((uint32_t)0x0001)
#define TAMPER_Port_S23_active            ((uint32_t)0x0002)
#define TAMPER_Port_S45_active            ((uint32_t)0x0004)
#define TAMPER_Port_S67_active            ((uint32_t)0x0008)
#define TAMPER_Port_ALL_active            ((uint32_t)0x000F)
#define IS_TAMPER_PORT_ACTIVE(PORT_ACTV)  (((PORT_ACTV) & (uint32_t)0xFFFFFFF0) == 0x0000)

/* tamper port pull up*/
#define TAMPER_Port_None_PU               ((uint32_t)0x0000)
#define TAMPER_Port_S0_PU                 ((uint32_t)0x0001)
#define TAMPER_Port_S1_PU                 ((uint32_t)0x0002)
#define TAMPER_Port_S2_PU                 ((uint32_t)0x0004)
#define TAMPER_Port_S3_PU                 ((uint32_t)0x0008)
#define TAMPER_Port_S4_PU                 ((uint32_t)0x0010)
#define TAMPER_Port_S5_PU                 ((uint32_t)0x0020)
#define TAMPER_Port_S6_PU                 ((uint32_t)0x0040)
#define TAMPER_Port_S7_PU                 ((uint32_t)0x0080)
#define TAMPER_Port_All_PU                ((uint32_t)0x00FF)
#define IS_TAMPER_PORT_PU(PORT_PU)        (((PORT_PU) & (uint32_t)0xFFFFFF00) == 0x00)

/* tamper enable*/
#define TAMPER_Port_S01                   ((uint32_t)0x0001)
#define TAMPER_Port_S23                   ((uint32_t)0x0002)
#define TAMPER_Port_S45                   ((uint32_t)0x0004)
#define TAMPER_Port_S67                   ((uint32_t)0x0008)
#define TAMPER_Port_ALL                   ((uint32_t)0x000F)
#define IS_TAMPER_PORT(PORT)              (((PORT) & (uint32_t)0xFFFFFF00) == 0x00)

/* tamper GlitchTimes */
#define TAMPER_GlitchTimes_31_25US        ((uint32_t)0x0000)
#define TAMPER_GlitchTimes_1ms            ((uint32_t)0x0001)
#define TAMPER_GlitchTimes_4ms						((uint32_t)0x0002)
#define TAMPER_GlitchTimes_8ms						((uint32_t)0x0003)
#define IS_TAMPER_GLITCH_TIME(TIME)			  ((TIME == TAMPER_GlitchTimes_31_25US) | \
                                           (TIME == TAMPER_GlitchTimes_1ms)     | \
                                           (TIME == TAMPER_GlitchTimes_4ms)     | \
                                           (TIME == TAMPER_GlitchTimes_8ms))

/* temper pull up Hold Time*/
#define TAMPER_PUPU_HoldTime_always       ((uint32_t)0x0000)/*!< always high*/
#define TAMPER_PUPU_HoldTime_2ms          ((uint32_t)0x0001)/*!< 1/2*Check Frequency*/
#define TAMPER_PUPU_HoldTime_8ms          ((uint32_t)0x0002)/*!< 1/4*Check Frequency*/
#define TAMPER_PUPU_HoldTime_16ms         ((uint32_t)0x0003)/*!< 1/16*Check Frequency*/
#define IS_TAMPER_PUPU_HOLD_TIME(TIME)    ((TIME == TAMPER_PUPU_HoldTime_always) | \
                                           (TIME == TAMPER_PUPU_HoldTime_2ms)    | \
                                           (TIME == TAMPER_PUPU_HoldTime_8ms)    | \
                                           (TIME == TAMPER_PUPU_HoldTime_16ms))

/* sensor dur*/
#define SENSOR_DUR_ALWAYS_ON              ((uint32_t)0)
#define SENSOR_DUR_2MS                    ((uint32_t)1)
#define SENSOR_DUR_8MS                    ((uint32_t)2)
#define SENSOR_DUR_16MS                   ((uint32_t)3)
#define IS_SENSOR_DUR(DUR)                ((DUR == SENSOR_DUR_ALWAYS_ON) | \
                                           (DUR == SENSOR_DUR_2MS)       | \
                                           (DUR == SENSOR_DUR_8MS)       | \
                                           (DUR == SENSOR_DUR_16MS))

/* sensor delay*/
#define SENEOR_DELAY_31_25US              ((uint32_t)0x0000)
#define SENEOR_DELAY_250US                ((uint32_t)0x0001)
#define SENEOR_DELAY_1_MS                 ((uint32_t)0x0002)
#define SENEOR_DELAY_4MS                  ((uint32_t)0x0003)
#define IS_SENEOR_DELAY(DELAY)            ((DELAY == SENEOR_DELAY_31_25US) | \
                                           (DELAY == SENEOR_DELAY_250US)   | \
                                           (DELAY == SENEOR_DELAY_1_MS)    | \
                                           (DELAY == SENEOR_DELAY_4MS))

/* LPM sensor enable*/
#define LPM_BAT_VDT12L_ENABLE             ((uint32_t)0x0001)
#define LPM_BAT_VDT33H_ENABLE             ((uint32_t)0x0002)
#define LPM_BAT_VDT33L_ENABLE             ((uint32_t)0x0004)
#define LPM_TEMPERATURE_40_ENABLE         ((uint32_t)0x0008)		/*!< Alarm below -40*/
#define LPM_TEMPERATURE_120_ENABLE        ((uint32_t)0x0010)		/*!< Alarm above 120*/
#define LPM_SENSOR_ALL_ENABLE             ((uint32_t)0x001F)
#define IS_SENSOR_ENABLE(VALUE)           (((VALUE) & (uint32_t)0xFFFFFFE0) == 0x00)

#define IS_BPK_LEN(VALUE, OFFSET)         (((VALUE > 0) && (VALUE <= 32))&& (VALUE+OFFSET) <= 32)

//***********LPM**************//
#define SSC_IT_VDT12L                    ((uint16_t)0x0001)
#define SSC_IT_VDT33H                    ((uint16_t)0x0002)
#define SSC_IT_VDT33L                    ((uint16_t)0x0004)
#define SSC_IT_TEMPERATURE_120           ((uint16_t)0x0008)
#define SSC_IT_TEMPERATURE_40            ((uint16_t)0x0010)
#define SSC_IT_TAMPER_S0                 ((uint16_t)0x0020)
#define SSC_IT_TAMPER_S1                 ((uint16_t)0x0040)
#define SSC_IT_TAMPER_S2                 ((uint16_t)0x0080)
#define SSC_IT_TAMPER_S3                 ((uint16_t)0x0100)
#define SSC_IT_TAMPER_S4                 ((uint16_t)0x0200)
#define SSC_IT_TAMPER_S5                 ((uint16_t)0x0400)
#define SSC_IT_TAMPER_S6                 ((uint16_t)0x0800)
#define SSC_IT_TAMPER_S7                 ((uint16_t)0x1000)
#define SSC_IT_MESH_SHIELDING            ((uint16_t)0x2000)//mesh shileding alarm

/* ssc secure sensor enable*/
#define SSC_SECSURE__VDT12L_ENABLE        ((uint32_t)0x0001)
#define SSC_SECSURE__VDT33H_ENABLE        ((uint32_t)0x0002)
#define SSC_SECSURE__VDT33L_ENABLE        ((uint32_t)0x0004)
#define SSC_BAT__VDT12L_ENABLE            ((uint32_t)0x0008)
#define SSC_BAT__VDT33H_ENABLE            ((uint32_t)0x0010)
#define SSC_BAT__VDT33L_ENABLE            ((uint32_t)0x0020)
#define SSC_BAT__TEMPERATURE_120_ENABLE   ((uint32_t)0x0040)
#define SSC_BAT__TEMPERATURE_40_ENABLE    ((uint32_t)0x0080)
#define SSC_SECURE_SENSOR_ALL_ENABLE      ((uint32_t)0x00FF)
#define IS_SSC_SEC_ENABLE(value)          (((value) & (uint32_t)0xFFFFFF00) == 0x00)

/* ssc secure sensor interrupt status*/
//**********SECURE**********//
#define SSC_IT_LPM_VDT12L                 ((uint8_t)0x01)
#define SSC_IT_SECSURE_VDT33H             ((uint8_t)0x02)
#define SSC_IT_SECSURE_VDT33L             ((uint8_t)0x04)  //frequency detect
#define SSC_IT_BAT_VDT12L                 ((uint8_t)0x08)
#define SSC_IT_BAT_VDT33H                 ((uint8_t)0x10)
#define SSC_IT_BAT_VDT33L                 ((uint8_t)0x20)
#define SSC_IT_BAT_TEMPERATURE_120        ((uint8_t)0x40)
#define SSC_IT_BAT_TEMPERATURE_40         ((uint8_t)0x80)

typedef struct  
{
	uint32_t TAMPER_Port_mode;      /*!< bitx=0:static mode, bitx=1:Dynamic mode;	bit0:port S0 and S1		bit1:port S2 and S3		bit2:port S4 and S5		bit3:port S6 and S7 >!*/
	uint32_t TAMPER_Port_PullUp;    /*!< bit 0~7 corresponding SENSOR_Port_S0~SENSOR_Port_S7,Static mode must be pull up, external grounding >!*/
	uint32_t TAMPER_Port_Enable;    /*!< bit0:port S0 and S1		bit1:port S2 and S3		bit2:port S4 and S5		bit3:port S6 and S7 >!*/
	uint32_t TAMPER_GlitchTimes;    /*!< Burr removal time >!*/
	uint32_t TAMPER_PUPU_HoldTime;  /*!< 上拉保持时间 >!*/
}TAMPER_InitTypeDef;

/**
  * @brief  Internal Set
  *
  * @param  ssc Interval
  *
  * @retval none
  */
void SSC_Interval(uint32_t Interval);

/**
  * @brief  Sheilding Alarm Enable
  *
  * @param  NewState : ENABLE or DISABLE
  *
  * @retval none
  */
void SSC_LPMSheildingAlarmEnable(FunctionalState NewState);

/**
  * @brief  Enable or disable clear key function in button battery  voltage field
  *
  * @param  NewState : ENABLE or DISABLE
  *
  * @retval none
  */
void SSC_ClearKeyCMD(FunctionalState NewState);

/**
  * @brief  Temper(external sensor) Init
  *
  * @param  TEMPER_InitStruct: point to TEMPER_InitTypeDef Struct
  *
  * @retval none
  */
void SSC_TemperInit(TAMPER_InitTypeDef *TAMPER_InitStruct);

/**
  * @brief  enable or disable Temper(external sensor)
  *
  * @param  SENSOR_Port : TEMPER_Port_S01~TEMPER_Port_S67
  *
  * @param  NewState    : ENABLE or DISABLE
  *
  * @retval none
  */
void SSC_LPMTemperCmd(uint32_t SENSOR_Port, FunctionalState NewState);

/**
  * @brief  Set Sensor detect time
  *
  * @param  sensor_dur : seltct macro SENSOR_DUR_ALWAYS_ON ~SENSOR_DUR_16MS.
  *
  * @retval none
  */
void SSC_SensorDur(uint32_t sensor_dur);

/**
  * @brief  Set alarm duration threshold
  *
  * @param  sensor_dur : seltct macro SENEOR_DELAY_31_25US ~ SENEOR_DELAY_4MS
  *
  * @retval none
  */
void SSC_SensorDelay(uint32_t sensor_dur);

/**
  * @brief  LPM sensor enable
  *
  * @param  LPM_sensor : LPM sensor enable define
  *
  * @param  NewState : ENABLE or DISABLE
  *
  * @retval none
  */
void SSC_LPMSensorCmd(uint32_t LPM_secsure_sensor, FunctionalState NewState);

/**
  * @brief  test function: Writes the secret key to the battery domain register
  * @param  uint32_t *buf, uint32_t len(1~32)
  * @retval none
  */
void SSC_LPMKeyRead(uint32_t *buf, uint8_t len, uint8_t offset);

/**
  * @brief  test function: Reads the secret key to the battery domain register
  * @param  uint32_t *buf, uint32_t len(1~32)
  * @retval none
  */
void SSC_LPMKeyWrite(uint32_t *buf, uint32_t len, uint8_t offset);

/**
 * @brief Lock LPM sensor\tamper&key register config, can not unlock
 *
 * @param  none
 *
 * @retval :none
 */
void SSC_LPMLock(void);

/**
 * @brief  get ssc lpm sensor&tamper interrupt status
 *
 * @param  none
 *
 * @retval matching ssc lpm sensor interrupt status define
 */
int16_t SSC_GetLPMStatusReg(void);

/**
  *  brief  clear ssc lpm alarm status bit
  *
  * @param  none
  *
  * @retval none
  */
void SSC_LPMClearStatusBit(void);

/**
 * @brief  ssc secure sensor enable
 *
 * @param  SSC_secsure_sensor : ssc secure sensor enable define
 *
 * @param  NewState : ENABLE or DISABLE
 *
 * @retval none
 */
void SSC_SecureCmd(uint32_t SSC_secsure, FunctionalState NewState);

/**
 * @brief  get ssc secure sensor interrupt status
 *
 * @param  none
 *
 * @retval matching ssc secure sensor interrupt status define
 */
uint8_t SSC_GetSecureStatus(void);

#endif 
