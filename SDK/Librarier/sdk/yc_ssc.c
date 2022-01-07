#include "yc_ssc.h"

#define SHIELDING_CHECKFRQ_BIT 					24
#define SHIELDING_ALARM_EN_BIT          27
#define ERASE_KEY_EN_BIT                20

#define SHIELDING_PORT_MODE_BIT 				12
#define SHIELDING_PORT_PULLUP_BIT 			16
#define SHIELDING_PORT_ENABLE_BIT 			8
#define SHIELDING_GLITCHTIME_DEALY_BIT 	30
#define SHIELDING_PU_HOLDTIME_BIT 			28

#define SENSOR_DUR_BIT                  21
#define SENSOR_DEALY_BIT                5
#define ANALOG_BAT12L_BIT               4
#define ANALOG_BATV33HL_BIT             5
#define ANALOG_TEMPRATURE_BIT           6

#define LPM_SHIELDING_LOCK_BIT          7

#define SENSOR_ALARM_CONTINUE_LIMIT_BIT 8

#define LPM_ALARM_BIT 16

void SSC_Interval(uint32_t Interval)
{
    _ASSERT(IS_INTERVAL(Interval));

    uint32_t ssc_temp;

    ssc_temp = lpm_read(LPM_SENSOR);

    ssc_temp &= ~(uint32_t)((uint32_t)0x3 << SHIELDING_CHECKFRQ_BIT);
    ssc_temp |= (uint32_t)((uint32_t)Interval << SHIELDING_CHECKFRQ_BIT);

    lpm_write(LPM_SENSOR, ssc_temp);
}

void SSC_LPMSheildingAlarmEnable(FunctionalState NewState)
{
    _ASSERT(IS_FUNCTIONAL_STATE(NewState));

    uint32_t ssc_temp;

    ssc_temp = lpm_read(LPM_SENSOR);

    if (NewState == ENABLE)
    {
        ssc_temp |= ((uint32_t)1) << SHIELDING_ALARM_EN_BIT;
    }
    else if (NewState == DISABLE)
    {
        ssc_temp &= ~(((uint32_t)1) << SHIELDING_ALARM_EN_BIT);
    }

    lpm_write(LPM_SENSOR, ssc_temp);
}

void SSC_ClearKeyCMD(FunctionalState NewState)
{
    _ASSERT(IS_FUNCTIONAL_STATE(NewState));

    uint32_t ssc_temp;

    ssc_temp = lpm_read(LPM_GPIO_WKHI);

    if (NewState == ENABLE)
    {
        ssc_temp |= ((uint32_t)1) << ERASE_KEY_EN_BIT;	//enable key clearing
    }
    else if (NewState == DISABLE)
    {
        ssc_temp &= ~(((uint32_t)1) << ERASE_KEY_EN_BIT);
    }

    lpm_write(LPM_GPIO_WKHI, ssc_temp);
}

void SSC_TemperInit(TAMPER_InitTypeDef *TAMPER_InitStruct)
{
    _ASSERT(IS_TAMPER_PORT_ACTIVE(TAMPER_InitStruct->TAMPER_Port_mode));
    _ASSERT(IS_TAMPER_PORT_PU(TAMPER_InitStruct->TAMPER_Port_PullUp));
    _ASSERT(IS_TAMPER_PORT(TAMPER_InitStruct->TAMPER_Port_Enable));
    _ASSERT(IS_TAMPER_GLITCH_TIME(TAMPER_InitStruct->TAMPER_GlitchTimes));
    _ASSERT(IS_TAMPER_PUPU_HOLD_TIME(TAMPER_InitStruct->TAMPER_PUPU_HoldTime));

    uint32_t ssc_temp;

    ssc_temp = lpm_read(LPM_SENSOR);

    ssc_temp &= ~((uint32_t)((uint32_t)0xFF << SHIELDING_PORT_PULLUP_BIT) | \
                  (uint32_t)((uint32_t)0xF << SHIELDING_PORT_ENABLE_BIT) | \
                  (uint32_t)((uint32_t)0xF << SHIELDING_PORT_MODE_BIT)   | \
                  (uint32_t)((uint32_t)0x3 << SHIELDING_GLITCHTIME_DEALY_BIT) | \
                  (uint32_t)((uint32_t)0x3 << SHIELDING_PU_HOLDTIME_BIT));
    ssc_temp |= TAMPER_InitStruct->TAMPER_Port_mode     << SHIELDING_PORT_MODE_BIT;         //shielding type
    ssc_temp |= TAMPER_InitStruct->TAMPER_Port_PullUp   << SHIELDING_PORT_PULLUP_BIT;       //pull up
    ssc_temp |= TAMPER_InitStruct->TAMPER_Port_Enable   << SHIELDING_PORT_ENABLE_BIT;       //shielding enable
    ssc_temp |= TAMPER_InitStruct->TAMPER_GlitchTimes   << SHIELDING_GLITCHTIME_DEALY_BIT;  //alarm_delay
    ssc_temp |= TAMPER_InitStruct->TAMPER_PUPU_HoldTime << SHIELDING_PU_HOLDTIME_BIT;       //pull up delay

    lpm_write(LPM_SENSOR, ssc_temp);
}

void SSC_LPMTemperCmd(uint32_t SENSOR_Port, FunctionalState NewState)
{
    _ASSERT(IS_TAMPER_PORT(SENSOR_Port));
    _ASSERT(IS_FUNCTIONAL_STATE(NewState));

    uint32_t ssc_temp;

    ssc_temp = lpm_read(LPM_SENSOR);

    if (NewState == ENABLE)
    {
        ssc_temp |= (uint32_t)SENSOR_Port << SHIELDING_PORT_ENABLE_BIT;
    }
    else if (NewState == DISABLE)
    {
        ssc_temp &= ~((uint32_t)SENSOR_Port << SHIELDING_PORT_ENABLE_BIT);
    }

    lpm_write(LPM_SENSOR, ssc_temp);
}

void SSC_SensorDur(uint32_t sensor_dur)
{
    _ASSERT(IS_SENSOR_DUR(sensor_dur));

    uint32_t ssc_temp;

    ssc_temp = lpm_read(LPM_GPIO_WKHI);

    ssc_temp |= ((uint32_t)sensor_dur) << SENSOR_DUR_BIT;

    lpm_write(LPM_GPIO_WKHI, ssc_temp);
}

void SSC_SensorDelay(uint32_t sensor_delay)
{
    _ASSERT(IS_SENEOR_DELAY(sensor_delay));

    uint32_t ssc_temp;

    ssc_temp = lpm_read(LPM_SENSOR);

    ssc_temp |= ((uint32_t)sensor_delay) << SENSOR_DEALY_BIT;//sensor delay

    lpm_write(LPM_SENSOR, ssc_temp);
}

void SSC_LPMSensorCmd(uint32_t sensor, FunctionalState NewState)
{
    _ASSERT(IS_SENSOR_ENABLE(sensor));

    uint32_t ssc_temp;

    ssc_temp = lpm_read(LPM_SENSOR);

    if (NewState == ENABLE)
    {
        ssc_temp |= (uint32_t)sensor;
    }
    else if (NewState == DISABLE)
    {
        ssc_temp &= ~(uint32_t)sensor;
    }

    lpm_write(LPM_SENSOR, ssc_temp);

    ssc_temp = lpm_read(LPM_CTRL);

    if (NewState == ENABLE)
    {

        if (LPM_BAT_VDT12L_ENABLE & sensor)
        {
            ssc_temp |= ((uint32_t)1 << ANALOG_BAT12L_BIT);
        }
        if ((LPM_BAT_VDT33H_ENABLE | LPM_BAT_VDT33L_ENABLE) & sensor)
        {
            ssc_temp |= ((uint32_t)1 << ANALOG_BATV33HL_BIT);
        }
        if ((LPM_TEMPERATURE_40_ENABLE | LPM_TEMPERATURE_120_ENABLE) & sensor)
        {
            ssc_temp |= ((uint32_t)1 << ANALOG_TEMPRATURE_BIT);
        }

        ssc_temp &= ~((0x07<<24) | (0x07<<28));//高低温高低压校准参数
        ssc_temp |= ((0x04<<28) | (0x03<<24));
    }
    else if (NewState == DISABLE)
    {
        if (LPM_BAT_VDT12L_ENABLE & sensor)
        {
            ssc_temp &= ~((uint32_t)1 << ANALOG_BAT12L_BIT);
        }
        if ((LPM_BAT_VDT33H_ENABLE | LPM_BAT_VDT33L_ENABLE) & sensor)
        {
            ssc_temp &= ~((uint32_t)1 << ANALOG_BATV33HL_BIT);
        }
        if ((LPM_TEMPERATURE_40_ENABLE | LPM_TEMPERATURE_120_ENABLE) & sensor)
        {
            ssc_temp &= ~((uint32_t)1 << ANALOG_TEMPRATURE_BIT);
        }
    }

    lpm_write(LPM_CTRL, ssc_temp);
}

void SSC_LPMKeyRead(uint32_t *buf, uint8_t len, uint8_t offset)
{
    _ASSERT(IS_BPK_LEN(len, offset));

    uint8_t i, k;

    for (i = 0; i < len; i++)
    {
        k = i + offset;
        buf[i] = lpm_read(LPM_KEY(k));
    }
}

void SSC_LPMKeyWrite(uint32_t *buf, uint32_t len, uint8_t offset)
{
    _ASSERT(IS_BPK_LEN(len, offset));

    uint8_t i, k;

    SSC_ClearKeyCMD(DISABLE);

    for (i = 0; i < len; i++)
    {
        k = i + offset;
        lpm_write(LPM_KEY(k), buf[i]);
    }

    SSC_ClearKeyCMD(ENABLE);
}

void SSC_LPMLock(void)
{
    uint32_t ssc_temp;

    ssc_temp = lpm_read(LPM_SENSOR);

    ssc_temp |= ((uint32_t)1) << LPM_SHIELDING_LOCK_BIT;

    lpm_write(LPM_SENSOR, ssc_temp);
}

int16_t SSC_GetLPMStatusReg(void)
{
    uint16_t ssc_inq;

    ssc_inq = (lpm_read(LPM_STATUS) >> 16);
    if((ssc_inq&(1<<14))==0)
    {
        ssc_inq &= 0xffe0;
    }

    return ssc_inq;
}

void SSC_LPMClearStatusBit(void)
{
    uint32_t ssc_temp;
    ssc_temp = lpm_read(LPM_SENSOR);
    SSC_LPMSheildingAlarmEnable(DISABLE);
    lpm_write(LPM_CLR_INTR, 0x6c);
    if (ssc_temp & (((uint32_t)1) << SHIELDING_ALARM_EN_BIT))
    {
        SSC_LPMSheildingAlarmEnable(ENABLE);
    }
}

void SSC_SecureCmd(uint32_t SSC_secsure, FunctionalState NewState)
{
    _ASSERT(IS_SSC_SEC_ENABLE(SSC_secsure));

    uint32_t ssc_temp;

    ssc_temp = SECURE_CTRL;

    if (NewState == ENABLE)
    {
        ssc_temp |= SSC_secsure;
    }
    else if (NewState == DISABLE)
    {
        ssc_temp &= ~SSC_secsure;
    }

    ssc_temp |= ((uint32_t)7) << SENSOR_ALARM_CONTINUE_LIMIT_BIT;

    SECURE_CTRL = ssc_temp;
}

uint8_t SSC_GetSecureStatus(void)
{
    uint8_t ssc_inq;

    ssc_inq = (SECURE_STATUS >> 1);

    return ssc_inq;
}
