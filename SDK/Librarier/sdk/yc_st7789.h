/*
File Name    : yc_st7789v2.c
Author       : Yichip
Version      : V1.0
Date         : 2020/1/14
Description  : st7789v2 lcd encapsulation.
*/

#ifndef __YC_ST7789_H__
#define __YC_ST7789_H__

#include <yc_gpio.h>
#include <yc_timer.h>
#include <yc_spi.h>
#include "font.h"
#include "board_config.h"

#define TFT_COL 320
#define TFT_ROW 240

#define BLACK   0x0000 //黑色
#define WHITE   0xFFFF //白色
#define RED     0xF800 //红色
#define GREEN   0x07E0 //绿色
#define BLUE    0x001F //蓝色
#define GRAY    0xDEDB //灰色15%
#define LGRAY   0xC618 //灰色25%
#define DGRAY   0x7BEF //深灰色
#define NAVY    0x000F //深蓝色
#define DGREEN  0x03E0 //深绿色
#define DCYAN   0x03EF //深青色
#define MAROON  0x7800 //深红色
#define PURPLE  0x780F //紫色
#define OLIVE   0x7BE0 //橄榄绿
#define CYAN    0x07FF //青色
#define MAGENTA 0xF81F //品红
#define YELLOW  0xFFE0 //黄色

typedef struct
{
    SPIx_TypeDef SPIx;
    uint16_t IO_TFT_SDA_PIN;
    GPIO_TypeDef IO_TFT_SDA_Port;
    uint16_t IO_TFT_SCL_PIN;
    GPIO_TypeDef IO_TFT_SCL_Port;
    uint16_t IO_TFT_A0_PIN;
    GPIO_TypeDef IO_TFT_A0_Port;
    uint16_t IO_TFT_RES_PIN;
    GPIO_TypeDef IO_TFT_RES_Port;
    uint16_t	IO_TFT_CS_PIN;
    GPIO_TypeDef IO_TFT_CS_Port;
    uint16_t IO_TFT_BL_PIN;
    GPIO_TypeDef IO_TFT_BL_Port;
} St7789TFT_InitTypedef;

/* TFT液晶屏控制IO初始化 */
void ST7789_TFT_GpioInit(St7789TFT_InitTypedef *St7789TFT_InitStruct);

/* 液晶屏驱动初始化 */
void ST7789_TFT_Init(void);

/* 画块函数 */
void ST7789_TFT_Draw_Piece(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t Color);

/* ASCII字符显示 */
void ST7789_TFT_ShowChar(uint16_t x, uint16_t y, uint16_t num, uint16_t size, uint16_t mode, uint16_t BL_color, uint16_t P_color);

/* ASCII字符串显示 */
void ST7789_TFT_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *p, uint16_t size, uint16_t mode, uint16_t BL_color, uint16_t P_color);

/* 汉字显示 */
void ST7789_TFT_ShowChinese(uint16_t x, uint16_t y, const uint8_t *ch, uint8_t size, uint8_t n, uint8_t mode, uint16_t BL_color, uint16_t P_color);

/* 图片显示 */
void ST7789_TFT_Picture(uint16_t Stratx, uint16_t Starty, const uint8_t *pic);

/* 图片显示快速 */
void ST7789_TFT_Picture_Q(uint16_t Stratx, uint16_t Starty, const uint8_t *pic);

void ST7789_TFT_Clear_White(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void ST7789_TFT_Clear_Gary(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void St7899_tft_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void St7789_tft_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void St7789_tft_RectangleFill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void St7789_tft_DrawCircle(uint16_t y0, uint16_t x0, uint8_t r, uint16_t c);
void st7789_tft_clear(uint16_t Color);

#endif
