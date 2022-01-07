#ifndef __NFC_COMMON_H_
#define __NFC_COMMON_H_

#define STOP	0x00

#include "yc_uart.h"
#include "yc_gpio.h"
#include "yc_spi.h"
#include "yc_timer.h"
#include "board_config.h"

#define     __O     volatile                  /*!< defines 'write only' permissions     */
#define     __IO    volatile                  /*!< defines 'read / write' permissions   */

#define RST_Enable()	GPIO_Config(NFC_RST_PORT, NFC_RST_PIN, OUTPUT_HIGH);
#define RST_Disable()	GPIO_Config(NFC_RST_PORT, NFC_RST_PIN, OUTPUT_LOW);

#define NFC_CS_LOW()	
#define NFC_CS_HIGH()	

void Nfc_SysTick_Delay_Us(__IO uint32_t us);
void Nfc_SysTick_Delay_Ms(__IO uint32_t ms);


void WRITEREG(uint8_t addr, uint8_t data);
uint8_t READREG(uint8_t addr);

void YC_EMV_HwReset(void);

#endif
