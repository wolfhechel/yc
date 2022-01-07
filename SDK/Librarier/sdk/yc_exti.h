/*
File Name    : yc_exti.h
Author       : Yichip
Version      : V1.0
Date         : 2019/12/04
Description  : exit encapsulation.
*/

#ifndef __YC_EXTI_H__
#define __YC_EXTI_H__

#include "yc3121.h"
#include "yc_gpio.h"

/**
  * @brief  EXTI Trigger enumeration
  */
typedef enum
{
    EXTI_Trigger_Off            = 0,
    EXTI_Trigger_Rising         = 1,
    EXTI_Trigger_Falling        = 2,
    EXTI_Trigger_Rising_Falling	= 3
} EXTI_TriggerTypeDef;

#define EXTI_Line0          ((uint32_t)0x0000)  /*!< External interrupt line 0 */
#define EXTI_Line1          ((uint32_t)0x0001)  /*!< External interrupt line 1 */
#define EXTI_Line2          ((uint32_t)0x0002)  /*!< External interrupt line 2 */
#define IS_EXTI_LINE(LINE)  (((LINE) == EXTI_Line0) || \
                             ((LINE) == EXTI_Line1) || \
									           ((LINE) == EXTI_Line2))


#define EXTI_PinSource0          ((uint16_t)0x0001)  /*!< Pin 0 selected */
#define EXTI_PinSource1          ((uint16_t)0x0002)  /*!< Pin 1 selected */
#define EXTI_PinSource2          ((uint16_t)0x0004)  /*!< Pin 2 selected */
#define EXTI_PinSource3          ((uint16_t)0x0008)  /*!< Pin 3 selected */
#define EXTI_PinSource4          ((uint16_t)0x0010)  /*!< Pin 4 selected */
#define EXTI_PinSource5          ((uint16_t)0x0020)  /*!< Pin 5 selected */
#define EXTI_PinSource6          ((uint16_t)0x0040)  /*!< Pin 6 selected */
#define EXTI_PinSource7          ((uint16_t)0x0080)  /*!< Pin 7 selected */
#define EXTI_PinSource8          ((uint16_t)0x0100)  /*!< Pin 8 selected */
#define EXTI_PinSource9          ((uint16_t)0x0200)  /*!< Pin 9 selected */
#define EXTI_PinSource10         ((uint16_t)0x0400)  /*!< Pin 10 selected */
#define EXTI_PinSource11         ((uint16_t)0x0800)  /*!< Pin 11 selected */
#define EXTI_PinSource12         ((uint16_t)0x1000)  /*!< Pin 12 selected */
#define EXTI_PinSource13         ((uint16_t)0x2000)  /*!< Pin 13 selected */
#define EXTI_PinSource14         ((uint16_t)0x4000)  /*!< Pin 14 selected */
#define EXTI_PinSource15         ((uint16_t)0x8000)  /*!< Pin 15 selected */
#define EXTI_PinSourceAll        ((uint16_t)0xffff)  /*!< Pin All selected */

#define IS_EXTI_PIN_SOURCE(PIN)  (((((PIN) & ~(uint32_t)0xFFFF)) == 0x00) && ((PIN) != (uint32_t)0x00))


#define	EXIT_Num         GPIO_GROUP_NUM
#define	EXIT_Pin_Num     GPIO_PIN_NUM

/**
  * @brief  Clear interrupt flag
  * @param	EXTI_Line:EXTI_Line0...EXTI_Line5
  * @param	EXTI_PinSource:EXTI_PinSource0...EXTI_PinSource7 or EXTI_PinSourceAll
  * @retval none
  */
void EXTI_ClearITPendingBit(uint32_t EXTI_Line, uint16_t EXTI_PinSource);

/**
  * @brief  Deinitializes the EXTI registers to default reset values.
  * @param	none
  * @retval none
  */
void EXTI_DeInit(void);

/**
  * @brief  get interrupt status
  * @param	EXTI_Line:EXTI_Line0...EXTI_Line5
  * @retval none
  */
uint16_t EXTI_GetITLineStatus(uint32_t EXTI_Line);

/**
  * @brief  EXTI LineConfig
  * @param	EXTI_Line:EXTI_Line0...EXTI_Line5
  * @param	EXTI_PinSource:EXTI_PinSource0...EXTI_PinSource7 or EXTI_PinSourceAll
  * @param	EXTI_Trigger:EXTI Trigger mode
  * @retval none
  */
void EXTI_LineConfig(uint32_t EXTI_Line, uint16_t EXTI_PinSource, EXTI_TriggerTypeDef EXTI_Trigger);

#endif	/* __YC_EXTI_H__ */

