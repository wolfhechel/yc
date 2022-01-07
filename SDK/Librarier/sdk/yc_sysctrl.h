#ifndef __YC_SYSCTRL_H__
#define __YC_SYSCTRL_H__

#include "yc3121.h"

/*======AHB总线控制====*/
#define SYSCTRL_AHBPeriph_INTR        ((uint32_t)(1<<CLKCLS_INT))
#define SYSCTRL_AHBPeriph_SHA         ((uint32_t)(1<<CLKCLS_SHA))
#define SYSCTRL_AHBPeriph_CRC         ((uint32_t)(1<<CLKCLS_CRC))
#define SYSCTRL_AHBPeriph_PWM         ((uint32_t)(1<<CLKCLS_TIM))
#define SYSCTRL_AHBPeriph_WDT         ((uint32_t)(1<<CLKCLS_WDT))
#define SYSCTRL_AHBPeriph_USB         ((uint32_t)(1<<CLKCLS_USB))
#define SYSCTRL_AHBPeriph_SPI         ((uint32_t)(1<<CLKCLS_SPI))
#define SYSCTRL_AHBPeriph_DES         ((uint32_t)(1<<CLKCLS_DES))
#define SYSCTRL_AHBPeriph_RSA         ((uint32_t)(1<<CLKCLS_RSA))
#define SYSCTRL_AHBPeriph_ASE         ((uint32_t)(1<<CLKCLS_AES))
#define SYSCTRL_AHBPeriph_GPIO        ((uint32_t)(1<<CLKCLS_GPIO))
#define SYSCTRL_AHBPeriph_7816        ((uint32_t)(1<<CLKCLS_7816))
#define SYSCTRL_AHBPeriph_BT          ((uint32_t)(1<<CLKCLS_BT	))
#define SYSCTRL_AHBPeriph_SM4         ((uint32_t)(1<<CLKCLS_SM4))
#define SYSCTRL_AHBPeriph_UART        ((uint32_t)(1<<CLKCLS_UART))
#define SYSCTRL_AHBPeriph_7811        ((uint32_t)(1<<CLKCLS_7811))
#define SYSCTRL_AHBPeriph_ADC7811     ((uint32_t)(1<<CLKCLS_ADC7811))
#define SYSCTRL_AHBPeriph_CP          ((uint32_t)(1<<CLKCLS_CP))
#define IS_SYSCTRL_AHB_PERIPH(PERIPH) ((PERIPH &0x7fffe) != 0x00)

#define SYSCTRL_HCLK_COFG_REGBIT      ((uint32_t)0x0f)
#define SYSCTRL_HCLK_Div_None         ((uint32_t)0x00)
#define SYSCTRL_HCLK_Div2             ((uint32_t)0x02)
#define SYSCTRL_HCLK_Div4             ((uint32_t)0x06)
#define SYSCTRL_HCLK_Div8             ((uint32_t)0x0e)
#define IS_GET_SYSCTRL_HCLK_DIV(DIV)  (((DIV) == SYSCTRL_HCLK_Div_None) || \
                                       ((DIV) == SYSCTRL_HCLK_Div2) || \
                                       ((DIV) == SYSCTRL_HCLK_Div4)) ||\
                                       ((DIV) == SYSCTRL_HCLK_Div8)

typedef enum
{
//    SleepMode_IDLE = 0x01,
    SleepMode_STOP = 0x02
} SleepMode_TypeDef;
#define IS_ALL_SLEEP_MODE(MODE)                     ((MODE) == SleepMode_STOP)

typedef struct
{
    uint32_t HCLK_Frequency;    /*!< returns HCLK frequency expressed in Hz */
} SYSCTRL_ClocksTypeDef;

/**
  * @brief  Enables or disables the APB peripheral clock.
  * @param  SYSCTRL_APBPeriph: specifies the APB peripheral to gates its clock.
  *
  *         For @b this parameter can be any combination
  *         of the enum SYSCTRL_AHB_PERIPH.
  * @param  NewState: new state of the specified peripheral clock.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SYSCTRL_AHBPeriphClockCmd(uint32_t SYSCTRL_AHBPeriph, FunctionalState NewState);

/**
  * @brief  Config CPU Enter sleep mode
  * @param  SleepMode_TypeDef: Select SleepMode.
  * @retval None
  */
void SYSCTRL_EnterSleep(SleepMode_TypeDef SleepMode);

/**
  * @brief  Config HCLK
  * @param  HCLK_Div:Div value
  * @retval None
  */
void SYSCTRL_HCLKConfig(uint32_t HCLK_Div);

/**
  * @brief  Get HCLK and PCLK frequency
  * @param  SYSCTRL_Clocks:The pointer point to the memmory preserve clocks value
  * @retval None
  */
void SYSCTRL_GetClocksFreq(SYSCTRL_ClocksTypeDef *SYSCTRL_Clocks);

/**
  * @brief  enable dpll clock(Accurate clock,Available for UART and USB)
  * @param  none
  * @retval None
  */
void SYSCTRL_EnableDpllClk(void);

#endif      /*__SYSCTRL_H*/
