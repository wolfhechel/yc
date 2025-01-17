/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usb_bsp.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : USB-OTG Core layer.
 *****************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "usb_bsp.h"
#include "usb_conf.h"

/** @addtogroup USBH_USER
* @{
*/

/** @defgroup USB_BSP
  * @brief This file is responsible to offer board support package
  * @{
  */

/** @defgroup USB_BSP_Private_Defines
  * @{
  */
//#define USE_ACCURATE_TIME
#define TIM_MSEC_DELAY                     0x01
#define TIM_USEC_DELAY                     0x02
#define HOST_OVRCURR_PORT                  GPIOE
#define HOST_OVRCURR_LINE                  GPIO_Pin_1
#define HOST_OVRCURR_PORT_SOURCE           GPIO_PortSourceGPIOE
#define HOST_OVRCURR_PIN_SOURCE            GPIO_PinSource1
#define HOST_OVRCURR_PORT_RCC              RCC_APB2Periph_GPIOE
#define HOST_OVRCURR_EXTI_LINE             EXTI_Line1
#define HOST_OVRCURR_IRQn                  EXTI1_IRQn

#ifdef USE_STM3210C_EVAL
    #define HOST_POWERSW_PORT_RCC             RCC_APB2Periph_GPIOC
    #define HOST_POWERSW_PORT                 GPIOC
    #define HOST_POWERSW_VBUS                 GPIO_Pin_9
#else
    #ifdef USE_USB_OTG_FS
        #define HOST_POWERSW_PORT_RCC            RCC_AHB1Periph_GPIOH
        #define HOST_POWERSW_PORT                GPIOH
        #define HOST_POWERSW_VBUS                GPIO_Pin_5
    #endif
#endif

#define HOST_SOF_OUTPUT_RCC                RCC_APB2Periph_GPIOA
#define HOST_SOF_PORT                      GPIOA
#define HOST_SOF_SIGNAL                    GPIO_Pin_8

/**
  * @}
  */


/** @defgroup USB_BSP_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */



/** @defgroup USB_BSP_Private_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_BSP_Private_Variables
  * @{
  */
ErrorStatus HSEStartUpStatus;
#ifdef USE_ACCURATE_TIME
    volatile uint32_t BSP_delay = 0;
#endif
/**
  * @}
  */

/** @defgroup USBH_BSP_Private_FunctionPrototypes
  * @{
  */

#ifdef USE_ACCURATE_TIME
    static void BSP_SetTime(uint8_t Unit);
    static void BSP_Delay(uint32_t nTime, uint8_t Unit);
    static void USB_OTG_BSP_TimeInit(void);
#endif
/**
  * @}
  */

/** @defgroup USB_BSP_Private_Functions
  * @{
  */

/**
  * @brief  USB_OTG_BSP_Init
  *         Initilizes BSP configurations
  * @param  None
  * @retval None
  */

void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{

}
/**
  * @brief  USB_OTG_BSP_EnableInterrupt
  *         Configures USB Global interrupt
  * @param  None
  * @retval None
  */
void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{

}

/**
  * @brief  BSP_Drive_VBUS
  *         Drives the Vbus signal through IO
  * @param  state : VBUS states
  * @retval None
  */

void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev, uint8_t state)
{

}

/**
  * @brief  USB_OTG_BSP_ConfigVBUS
  *         Configures the IO for the Vbus and OverCurrent
  * @param  None
  * @retval None
  */

void  USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev)
{
#ifdef USE_USB_OTG_FS
    GPIO_InitTypeDef GPIO_InitStructure;

#ifdef USE_STM3210C_EVAL
    RCC_APB2PeriphClockCmd(HOST_POWERSW_PORT_RCC, ENABLE);


    /* Configure Power Switch Vbus Pin */
    GPIO_InitStructure.GPIO_Pin = HOST_POWERSW_VBUS;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(HOST_POWERSW_PORT, &GPIO_InitStructure);
#else
#ifdef USE_USB_OTG_FS
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);

    GPIO_InitStructure.GPIO_Pin = HOST_POWERSW_VBUS;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    GPIO_Init(HOST_POWERSW_PORT, &GPIO_InitStructure);
#endif
#endif

    /* By Default, DISABLE is needed on output of the Power Switch */
    GPIO_SetBits(HOST_POWERSW_PORT, HOST_POWERSW_VBUS);

    USB_OTG_BSP_mDelay(200);   /* Delay is need for stabilising the Vbus Low
  in Reset Condition, when Vbus=1 and Reset-button is pressed by user */
#endif
}


/**
  * @brief  USB_OTG_BSP_uDelay
  *         This function provides delay time in micro sec
  * @param  usec : Value of delay required in micro sec
  * @retval None
  */
void USB_OTG_BSP_uDelay(const uint32_t usec)
{

#ifdef USE_ACCURATE_TIME
    BSP_Delay(usec, TIM_USEC_DELAY);
#else
    volatile uint32_t count = 0;
    const uint32_t utime = (120 * usec / 7);
    do
    {
        if (++count > utime)
        {
            return ;
        }
    }
    while (1);
#endif

}


/**
  * @brief  USB_OTG_BSP_mDelay
  *          This function provides delay time in milli sec
  * @param  msec : Value of delay required in milli sec
  * @retval None
  */
void USB_OTG_BSP_mDelay(const uint32_t msec)
{
#ifdef USE_ACCURATE_TIME
    BSP_Delay(msec, TIM_MSEC_DELAY);
#else
    USB_OTG_BSP_uDelay(msec * 1000);
#endif

}


/**
  * @brief  USB_OTG_BSP_TimerIRQ
  *         Time base IRQ
  * @param  None
  * @retval None
  */

void USB_OTG_BSP_TimerIRQ(void)
{
#ifdef USE_ACCURATE_TIME
//  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
//  {
//    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//    if (BSP_delay > 0x00)
//    {
//      BSP_delay--;
//    }
//    else
//    {
//      TIM_Cmd(TIM2,DISABLE);
//    }
//  }
#endif
}

#ifdef USE_ACCURATE_TIME
/**
  * @brief  BSP_Delay
  *         Delay routine based on TIM2
  * @param  nTime : Delay Time
  * @param  unit : Delay Time unit : mili sec / micro sec
  * @retval None
  */
static void BSP_Delay(uint32_t nTime, uint8_t unit)
{

//  BSP_delay = nTime;
//  BSP_SetTime(unit);
//  while(BSP_delay != 0);
//  TIM_Cmd(TIM2,DISABLE);
}

/**
  * @brief  BSP_SetTime
  *         Configures TIM2 for delay routine based on TIM2
  * @param  unit : msec /usec
  * @retval None
  */
static void BSP_SetTime(uint8_t unit)
{
//  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//
//  TIM_Cmd(TIM2,DISABLE);
//  TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
//
//
//  if(unit == TIM_USEC_DELAY)
//  {
//    TIM_TimeBaseStructure.TIM_Period = 11;
//  }
//  else if(unit == TIM_MSEC_DELAY)
//  {
//    TIM_TimeBaseStructure.TIM_Period = 11999;
//  }
//  TIM_TimeBaseStructure.TIM_Prescaler = 5;
//  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//
//  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
//  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//
//  TIM_ARRPreloadConfig(TIM2, ENABLE);
//
//  /* TIM IT enable */
//  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
//
//  /* TIM2 enable counter */
//  TIM_Cmd(TIM2, ENABLE);
}

#endif

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
