#ifndef __YC_LPM_H__
#define __YC_LPM_H__

#include "yc3121.h"
#include "yc_ipc.h"
#include "yc_otp.h"
#include "yc_sysctrl.h"
#include "misc.h"

#define LPMBT_CONTROL0      0         //LPMCONTROL
#define LPMBT_CONTROL1      1         //LPMCONTROL2
#define LPMBT_WKUPLOW       2
#define LPMBT_WKUPHIGH      3
#define LPMBT_SLEEPCOUNT    4
#define LPMBT_CONTROL2      5         //BUCKControl
#define LPMBT_CONTROL3      6         //CHARGER

extern Boolean ipc_inited;

uint32_t lpm_read(volatile int *addr);

void lpm_write(volatile int *addr, uint32_t value);

void lpm_bt_write(uint8_t type, uint32_t val);

uint32_t lpm_bt_read(uint8_t type);

void lpm_sleep(void);

void setlpmval(volatile int *addr, uint8_t startbit, uint8_t bitwidth, uint32_t val);

uint32_t readlpmval(volatile int *addr, uint8_t startbit, uint8_t bitwidth);

/**
  * @brief  Drop down all unused GPIO
  * @param  None
  * @retval None
  */
void GPIO_Unused_Pd(void);

/**
  * @brief  BT go into deep sleep mode
  * @param  None
  * @retval tmp: time
  */
void BT_Hibernate(void);

/**
  * @brief  Reduce chip frequency.
  * @param  None
  * @retval None
  */
void Chip_Speedstep(void);


/*
 * @brief:  lpm_sleep config
 * @param:  time: RTC wakeup time (unit:second)
 * @param:  GPIO_Pin0_Pin31      GPIOGROUPA&GPIOGROUPB
 * @param:  GPIO_Pin32_Pin47     GPIOGROUPC
 * @param:  islow_wakeup:	0:High level wakeup
 *                          1:LOW  level wakeup
 * @param:  is_powerdownbt:	0: diseable bt sleep
 *                          1: enable bt sleep
 * @param:  tmp: BT_Hibernate return or 0
 * @return: none
 */
void CM0_Sleep(uint32_t time, uint32_t GPIO_Pin0_Pin31, uint16_t GPIO_Pin32_Pin47, uint8_t islow_wakeup, uint8_t is_powerdownbt);

/*
 * @brief: GPIO32(GPIOC GPIO_Pin_0) enable
 */
void enable_gpio_32(void);

#endif
