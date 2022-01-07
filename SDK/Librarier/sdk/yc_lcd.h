/*
File Name    : yc_lcd.c
Author       : Yichip
Version      : V1.0
Date         : 2019/12/07
Description  : This file provides all the LCD functions.
*/
/*
LCD messages:

*/
#ifndef __LCD_H__
#define __LCD_H__

#include "yc_gpio.h"
#include "yc_spi.h"
#include "yc_timer.h"
#include "board_config.h"

#define	LCD_RS_CMD		0x00
#define	LCD_RS_DATA		0x01

typedef struct
{
    SPIx_TypeDef SPIx;
    uint16_t IO_LCD_SDA_PIN;
    GPIO_TypeDef IO_LCD_SDA_Port;
    uint16_t IO_LCD_SCL_PIN;
    GPIO_TypeDef IO_LCD_SCL_Port;
    uint16_t IO_LCD_A0_PIN;
    GPIO_TypeDef IO_LCD_A0_Port;
    uint16_t IO_LCD_RES_PIN;
    GPIO_TypeDef IO_LCD_RES_Port;
    uint16_t	IO_LCD_CS_PIN;
    GPIO_TypeDef IO_LCD_CS_Port;
    uint16_t IO_LCD_BL_PIN;
    GPIO_TypeDef IO_LCD_BL_Port;
} LCD_InitTypedef;

/**
  * @brief 	lcd io initialize
  */
void lcd_io_init(LCD_InitTypedef *LCD_InitStruct);

/**
	* @brief  MCU sends cmd to LCD via SPI
  * @param  data: output cmd.
  */
void spi_write_cmd(unsigned char cmd);

/**
  * @brief  MCU sends one byte to LCD via SPI
  * @param  data: data or cmd val.
  *         cmd: send type.
  */
void write_byte_lcd(unsigned char data, unsigned char cmd);

/**
  * @brief  MCU sends n byte to LCD via SPI
  * @param  data: data or cmd val.
  *         len: number of content sent
  *         cmd: send type.
  */
void write_bytes_lcd(unsigned char *pdata, unsigned char len, unsigned char cmd);

/**
  * @brief  clear screen
  */
void clr_screen(void);

/**
  * @brief  fill all screen
  */
void full_screen(void);

/**
 * @brief  set x y position
 * @param  column: horizontal position you want set,this value must be between 0 and 127.
 *         page: longitudinal position you want set,this value must be between 0 and 7.
 */
void set_xy(unsigned char column, unsigned char page);

/**
 * @brief  initialize lcd
 */
void init_lcd(void);

#endif
