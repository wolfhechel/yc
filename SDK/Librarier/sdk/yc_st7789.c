/*
File Name    : yc_st7789.c
Author       : Yichip
Version      : V1.0
Date         : 2020/3/24
Description  : st7789 lcd encapsulation.
*/

#include "yc_st7789.h"

void ST7789_TFT_GpioInit(St7789TFT_InitTypedef *St7789TFT_InitStruct)
{
    SPI_InitTypeDef SPI_InitStruct;

    SPI_InitStruct.BaudRatePrescaler = SPI_BaudRatePrescaler_1;
    SPI_InitStruct.CPHA = SPI_CPHA_First_Edge;
    SPI_InitStruct.CPOL = SPI_CPOL_Low;
    SPI_InitStruct.Mode =  SPI_Mode_Master;
    SPI_InitStruct.RW_Delay = 1;
    #if (BOARD_VER == EPOS_BOARD_V1_0)
    GPIO_Config(St7789TFT_InitStruct->IO_TFT_SCL_Port, St7789TFT_InitStruct->IO_TFT_SCL_PIN, SPID1_SCK);
    GPIO_Config(St7789TFT_InitStruct->IO_TFT_CS_Port, St7789TFT_InitStruct->IO_TFT_CS_PIN, SPID1_NCS);
    GPIO_Config(St7789TFT_InitStruct->IO_TFT_SDA_Port, St7789TFT_InitStruct->IO_TFT_SDA_PIN, SPID1_MOSI);
    #endif
    SPI_Init(St7789TFT_InitStruct->SPIx, &SPI_InitStruct);
}

/**************************************************************
函数名称 : st7789_tft_write_reg
函数功能 : 向TFT驱动芯片写一个命令
输入参数 : reg：要写入的命令
返回值   : 无
备注     : ST7789_TFT_A0_PIN为0时表示写命令
**************************************************************/
static void st7789_tft_write_reg(uint8_t reg)
{
    #if (BOARD_VER == EPOS_BOARD_V1_0)
    GPIO_Config(ST7789_TFT_A0_PORT, ST7789_TFT_A0_PIN, OUTPUT_LOW);
    SPI_SendData(ST7789VTFTSPI, reg);
    #endif
}

/**************************************************************
函数名称 : st7789_tft_write_data
函数功能 : 向TFT驱动芯片写一个uint8_t数据
输入参数 : data：要写入的数据
返回值   : 无
备注     : ST7789_TFT_A0_PIN为1时表示写数据
**************************************************************/
static void st7789_tft_write_data(uint8_t data)
{
    #if (BOARD_VER == EPOS_BOARD_V1_0)
    GPIO_Config(ST7789_TFT_A0_PORT, ST7789_TFT_A0_PIN, OUTPUT_HIGH);
    SPI_SendData(ST7789VTFTSPI, data);
    #endif
}

/**************************************************************
函数名称 : st7789_tft_write_buff
函数功能 : 向TFT驱动芯片写一组数据
输入参数 : buff：要写入的数据首地址
输入参数 : led：要写入的数据长度
返回值   : 无
备注     : ST7789_TFT_A0_PIN为1时表示写数据
**************************************************************/
static void st7789_tft_write_buff(uint8_t *buff, int len)
{
    #if (BOARD_VER == EPOS_BOARD_V1_0)
    GPIO_Config(ST7789_TFT_A0_PORT, ST7789_TFT_A0_PIN, OUTPUT_HIGH);
    SPI_SendBuff(ST7789VTFTSPI, buff, len);
    #endif
}

/**************************************************************
函数名称 : st7789_tft_write_ram
函数功能 : 使能写入数据到RAM
输入参数 : 无
返回值   : 无
备注     : 无
**************************************************************/
static void st7789_tft_write_ram(void)
{
    st7789_tft_write_reg(0x2C);
}

/**************************************************************
函数名称 : st7789_tft_display_on
函数功能 : 打开显示
输入参数 : 无
返回值   : 无
备注     : 无
**************************************************************/
static void st7789_tft_display_on(void)
{
    st7789_tft_write_reg(0x29);
}

/**************************************************************
函数名称 : st7789_tft_display_off
函数功能 : 关闭显示
输入参数 : 无
返回值   : 无
备注     : 无
**************************************************************/
static void st7789_tft_display_off(void)
{
    st7789_tft_write_reg(0x28);
}

static void TFT_WR_DATA(uint16_t da)
{
    unsigned char data[2];

    data[0] = (da >> 8);
    data[1] = da & 0xFF;

    st7789_tft_write_buff(data, 2);
}
/**************************************************************
函数功能:  设置光标的位置
**************************************************************/
void St7789_tft_SetCursor(uint16_t x, uint16_t y)
{
    st7789_tft_write_reg(0x2A); //设置X坐标
    TFT_WR_DATA(x);  //写入X
    st7789_tft_write_reg(0x2B);    //设置Y坐标
    TFT_WR_DATA(y);   //写入Y

}
/**************************************************************
函数名称 : st7789_tft_address_set
函数功能 : 设置坐标
输入参数 : x1,y1：起始地址，x2,y2：终点地址
返回值   : 无
备注     : 无
**************************************************************/
static void st7789_tft_address_set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    st7789_tft_write_reg(0x2A);
    TFT_WR_DATA(x1);
    TFT_WR_DATA(x2);
    st7789_tft_write_reg(0x2B);
    TFT_WR_DATA(y1);
    TFT_WR_DATA(y2);
}

void st7789_tft_clear(uint16_t Color)
{
    uint16_t i, j;

    st7789_tft_display_off();
    st7789_tft_address_set(0, 0, TFT_COL - 1, TFT_ROW - 1);
    st7789_tft_write_ram();

    for (i = 0; i < TFT_COL; i++)
    {
        for (j = 0; j < TFT_ROW; j++)
        {
            TFT_WR_DATA(Color);
        }
    }
    st7789_tft_display_on();
}

/**************************************************************
函数名称 : st7789_tft_draw_point
函数功能 : TFT画一个点
输入参数 : x,y	--> 画点坐标，color --> 点的颜色
返回值   : 无
备注     : 无
**************************************************************/
static void st7789_tft_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    st7789_tft_address_set(x, y, x, y);
//	St7789_tft_SetCursor(x,y);
    st7789_tft_write_ram();
    TFT_WR_DATA(color);
}

void ST7789_TFT_Draw_Piece(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t Color)
{
    uint16_t i, j;

    st7789_tft_address_set(x1, y1, x2, y2);
    st7789_tft_write_ram();

    for (i = x1; i < x2; i++)
    {
        for (j = y1; j < y2; j++)
        {
            TFT_WR_DATA(Color);
        }
    }
}
/**************************************************************
函数功能：画直线
参    数：
x1,y1:起点坐标
x2,y2:终点坐标
**************************************************************/
void St7899_tft_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1; //计算坐标增量
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;
    if (delta_x > 0)
        incx = 1; //设置单步方向
    else if (delta_x == 0)
        incx = 0;//垂直线
    else
    {
        incx = - 1;
        delta_x = - delta_x;
    }
    if (delta_y > 0)
        incy = 1;
    else if (delta_y == 0)
        incy = 0;//水平线
    else
    {
        incy = - 1;
        delta_y = - delta_y;
    }
    if (delta_x > delta_y)
        distance = delta_x; //选取基本增量坐标轴
    else
        distance = delta_y;
    for (t = 0; t <= distance + 1; t++)//画线输出
    {
        st7789_tft_draw_point(uRow, uCol, color);//画点
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}

/**************************************************************
函数功能：画矩形
参    数：(x1,y1),(x2,y2):矩形的对角坐标
**************************************************************/
void St7789_tft_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    St7899_tft_DrawLine(x1, y1, x2, y1, color);
    St7899_tft_DrawLine(x1, y1, x1, y2, color);
    St7899_tft_DrawLine(x1, y2, x2, y2, color);
    St7899_tft_DrawLine(x2, y1, x2, y2, color);
}

/**************************************************************
函数功能：矩形填充
参    数：(x1,y1),(x2,y2):矩形的对角坐标
**************************************************************/
void St7789_tft_RectangleFill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint32_t i, j;
    for (i = y1; i <= y2; i++) //以高度为准
    {
        St7789_tft_SetCursor(x1, i); //设置光标的位置
        st7789_tft_write_ram();  //表示写GRAM  就是颜色数据
        for (j = x1; j <= x2; j++)
        {
            TFT_WR_DATA(color); //写入颜色
        }
    }
}

/**************************************************************
函数功能：在指定位置画一个指定大小的圆
参    数：
		(x,y):中心点
		r    :半径
**************************************************************/
void St7789_tft_DrawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t c)
{
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);//判断下个点位置的标志
    while (a <= b)
    {
        st7789_tft_draw_point(x0 + a, y0 - b, c);             //5
        st7789_tft_draw_point(x0 + b, y0 - a, c);             //0
        st7789_tft_draw_point(x0 + b, y0 + a, c);             //4
        st7789_tft_draw_point(x0 + a, y0 + b, c);             //6
        st7789_tft_draw_point(x0 - a, y0 + b, c);             //1
        st7789_tft_draw_point(x0 - b, y0 + a, c);
        st7789_tft_draw_point(x0 - a, y0 - b, c);             //2
        st7789_tft_draw_point(x0 - b, y0 - a, c);             //7
        a++;
        if (di < 0)
            di += 4 * a + 6;	//Bresenham画圆算法
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

void ST7789_TFT_Init(void)
{
    #if (BOARD_VER == EPOS_BOARD_V1_0)
    GPIO_Config(ST7789_TFT_RST_PORT, ST7789_TFT_RST_PIN, OUTPUT_LOW);
    delay_ms(20);
    GPIO_Config(ST7789_TFT_RST_PORT, ST7789_TFT_RST_PIN, OUTPUT_HIGH);
    #endif
    delay_ms(20);

    //--------------------------------------Display Setting------------------------------------------//
    /* Memory Data Access Control：横屏显示 */
    st7789_tft_write_reg(0x36);
    st7789_tft_write_data(0xA0);

    /* Column Address Set: 0~319 */
    st7789_tft_write_reg(0x2A);
    st7789_tft_write_data(0x00);//列起始地址
    st7789_tft_write_data(0x00);
    st7789_tft_write_data(0x00);//列结束地址
    st7789_tft_write_data(0x3F);

    /* Row Address Set: 0~239 */
    st7789_tft_write_reg(0x2B);
    st7789_tft_write_data(0x00);//行起始地址
    st7789_tft_write_data(0x00);
    st7789_tft_write_data(0x01);//行结束地址
    st7789_tft_write_data(0xEF);

    /* Frame Rate Control in Normal Mode: 58M */
    st7789_tft_write_reg(0xC6);
    st7789_tft_write_data(0x10);

    /*  Interface Pixel Format: RGB */
    st7789_tft_write_reg(0x3A);
    st7789_tft_write_data(0x05);

    st7789_tft_write_reg(0xCF);
    st7789_tft_write_data(0x00);
    st7789_tft_write_data(0xC1);
    st7789_tft_write_data(0X30);

    st7789_tft_write_reg(0xED);
    st7789_tft_write_data(0x64);
    st7789_tft_write_data(0x03);
    st7789_tft_write_data(0X12);
    st7789_tft_write_data(0X81);

    st7789_tft_write_reg(0xE8);
    st7789_tft_write_data(0x85);
    st7789_tft_write_data(0x10);
    st7789_tft_write_data(0x78);

    st7789_tft_write_reg(0xCB);
    st7789_tft_write_data(0x39);
    st7789_tft_write_data(0x2C);
    st7789_tft_write_data(0x00);
    st7789_tft_write_data(0x34);
    st7789_tft_write_data(0x02);

    st7789_tft_write_reg(0xF7);
    st7789_tft_write_data(0x20);

    st7789_tft_write_reg(0xEA);
    st7789_tft_write_data(0x00);
    st7789_tft_write_data(0x00);

    /* LCM Control */
    st7789_tft_write_reg(0xC0);
    st7789_tft_write_data(0x21);

    /* ID code Setting */
    st7789_tft_write_reg(0xC1);
    st7789_tft_write_data(0x12);

    /* VCOM Offset Set */
    st7789_tft_write_reg(0xC5);
    st7789_tft_write_data(0x32);
    st7789_tft_write_data(0x3C);

    /* CABC Control */
    st7789_tft_write_reg(0xC7);
    st7789_tft_write_data(0XC1);

    /* RGB Interface Control */
    st7789_tft_write_reg(0xB1);
    st7789_tft_write_data(0x00);
    st7789_tft_write_data(0x18);

    /* Gamma Set */
    st7789_tft_write_reg(0x26);
    st7789_tft_write_data(0x01);

    /* Positive Voltage Gamma Control */
    st7789_tft_write_reg(0xE0);
    st7789_tft_write_data(0x0F);
    st7789_tft_write_data(0x20);
    st7789_tft_write_data(0x1E);
    st7789_tft_write_data(0x09);
    st7789_tft_write_data(0x12);
    st7789_tft_write_data(0x0B);
    st7789_tft_write_data(0x50);
    st7789_tft_write_data(0XBA);
    st7789_tft_write_data(0x44);
    st7789_tft_write_data(0x09);
    st7789_tft_write_data(0x14);
    st7789_tft_write_data(0x05);
    st7789_tft_write_data(0x23);
    st7789_tft_write_data(0x21);
    st7789_tft_write_data(0x00);

    /* Negative Voltage Gamma Control */
    st7789_tft_write_reg(0XE1);
    st7789_tft_write_data(0x00);
    st7789_tft_write_data(0x19);
    st7789_tft_write_data(0x19);
    st7789_tft_write_data(0x00);
    st7789_tft_write_data(0x12);
    st7789_tft_write_data(0x07);
    st7789_tft_write_data(0x2D);
    st7789_tft_write_data(0x28);
    st7789_tft_write_data(0x3F);
    st7789_tft_write_data(0x02);
    st7789_tft_write_data(0x0A);
    st7789_tft_write_data(0x08);
    st7789_tft_write_data(0x25);
    st7789_tft_write_data(0x2D);
    st7789_tft_write_data(0x0F);

    /* Sleep Out */
    st7789_tft_write_reg(0x11);
    delay_ms(20); //Delay 20ms

    /* Display On */
    st7789_tft_clear(WHITE);
    st7789_tft_write_reg(0x29);
    #if (BOARD_VER == EPOS_BOARD_V1_0)
    GPIO_Config(ST7789_TFT_BL_PORT, ST7789_TFT_BL_PIN, ST7789_TFT_BL_HIGH_LIGHT?OUTPUT_HIGH:OUTPUT_LOW);
    #endif
}

/**************************************************************
函数名称 : ST7789_TFT_ShowChar
函数功能 : lcd显示一个字符
输入参数 : x,y:起始坐标
          num:要显示的字符:" "--->"~"
          size:字体大小
          mode:叠加方式(1)还是非叠加方式(0)
返回值   : 无
备注     : 无
**************************************************************/
void ST7789_TFT_ShowChar(uint16_t x, uint16_t y, uint16_t num, uint16_t size, uint16_t mode, uint16_t BL_color, uint16_t P_color)
{
    uint16_t temp, t1, t;
    uint16_t y0 = y;
    uint16_t csize = ((size / 8) + ((size % 8) ? 1 : 0)) * (size / 2); /* 得到字体一个字符对应点阵集所占的字节数	 */

    num = num - ' ';/* 得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库） */

    for (t = 0; t < csize; t++)/*遍历打印所有像素点到LCD */
    {
        if (16 == size)
        {
            temp = asc2_1608[num][t];/* 调用1608字体 */
        }
        else if (24 == size)
        {
            temp = asc2_2412[num][t];/* 调用2412字体 */
        }
        else if (32 == size)
        {
            temp = asc2_3216[num][t];/* 调用3216数码管字体 */
        }
        else
        {
            return;/* 没有找到对应的字库 */
        }
        for (t1 = 0; t1 < 8; t1++)/* 打印一个像素点到液晶 */
        {
            if (temp & 0x80)
            {
                st7789_tft_draw_point(x, y, P_color);//字色
            }
            else if (0 == mode)
            {
                st7789_tft_draw_point(x, y, BL_color);//背景色
            }
            temp <<= 1;
            y++;

            if (y >= TFT_ROW)
            {
                return;/* 超区域了 */
            }
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                if (x >= TFT_COL)
                {
                    return;/* 超区域了 */
                }
                break;
            }
        }
    }
}

/**************************************************************
函数名称 : ST7789_TFT_ShowString
函数功能 : lcd显示字符串
输入参数 : x,y:起始坐标
          width,height：区域大小
          *p:字符串起始地址
          size:字体大小
          mode:叠加方式(1)还是非叠加方式(0)
返回值  : 无
备注    : 无
**************************************************************/
void ST7789_TFT_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *p, uint16_t size, uint16_t mode, uint16_t BL_color, uint16_t P_color)
{
    uint16_t x0 = x;
    uint16_t len = 0;
    uint16_t WxDxH = 0;
    width += x;
    height += y;

    WxDxH = size / 2;
//	if (width > 320)
//	{
//		len = (320-x)/size/2;
//		ST7789_TFT_ShowString(x, height, len*(height-y), 16, p+(12-len+1), 32, 1, WHITE, OLIVE);
//	}

    if (width > 320)
    {
        len = (320 - x) / WxDxH;
        ST7789_TFT_ShowString(x, height + 8, (width - x) - (len * WxDxH), height - y, p + len, size, mode, BL_color, P_color);
    }

    while ((*p <= '~') && (*p >= ' ')) /* 判断是不是非法字符! */
    {
        if (x >= width)
        {
            x = x0;
            y += size;
        }
        if (y >= height)
        {
            break;
        }
        ST7789_TFT_ShowChar(x, y, *p, size, mode, BL_color, P_color);
        x += WxDxH;
        p++;
    }
}

/**************************************************************
函数名称 : ST7789_TFT_ShowChinese
函数功能 : TFT显示字符串
输入参数 : x,y:起始坐标
          *ch:汉字字符串起始地址
          size:字体大小
          n:汉字个数
          mode:叠加方式(1)还是非叠加方式(0)
返回值   : 无
备注     : 无
**************************************************************/
void ST7789_TFT_ShowChinese(uint16_t x, uint16_t y, const uint8_t *ch, uint8_t size, uint8_t n, uint8_t mode, uint16_t BL_color, uint16_t P_color)
{
    uint32_t temp, t, t1;
    uint16_t y0 = y;
    uint32_t csize = ((size / 8) + ((size % 8) ? 1 : 0)) * (size) * n;	/* 得到字体字符对应点阵集所占的字节数 */

    for (t = 0; t < csize; t++)
    {
        temp = ch[t];	/* 得到点阵数据 */

        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
            {
                st7789_tft_draw_point(x, y, P_color);
            }
            else if (mode == 0)
            {
                st7789_tft_draw_point(x, y, BL_color);
            }
            temp <<= 1;
            y++;
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

void ST7789_TFT_Picture(uint16_t Stratx, uint16_t Starty, const uint8_t *pic)
{
    uint32_t i = 8, len;
    uint16_t temp, x, y, D_x = 0, D_y = 0;

    x = (uint16_t)((pic[2] << 8) | pic[3]) - 1;
    y = (uint16_t)((pic[4] << 8) | pic[5]) - 1;

    len = 2 * (x + 1) * (y + 1);

    while (i < (len + 8))
    {
        temp = (uint16_t)((pic[i] << 8) + pic[i + 1]);		//可以考虑用逻辑 或 操作替代
        if (D_x > x)
        {
            D_x = 0;
            D_y += 1;
            // D_y+=2;
            // i += x*2;
        }
        st7789_tft_draw_point(Stratx + D_x, Starty + D_y, temp);
        D_x += 1;
        i = i + 2;
    }
}

void ST7789_TFT_Picture_Q(uint16_t Stratx, uint16_t Starty, const uint8_t *pic)
{
    //计算长宽数据及长度
    uint32_t len, i = 8;
    uint16_t x, y;
    uint8_t temp[16];
    uint8_t j = 0;

    x = (uint16_t)((pic[2] << 8) | pic[3]) - 1;
    y = (uint16_t)((pic[4] << 8) | pic[5]) - 1;
    len = 2 * (x + 1) * (y + 1);

    st7789_tft_address_set(Stratx, Starty, Stratx + x, Starty + y);
    st7789_tft_write_reg(0x2C);

    while (i < (len + 8))
    {
        j = 0;
        while (j < 16)
        {
            temp[j++] = pic[i++];
        }
        st7789_tft_write_buff(temp, 16);
    }
}

void ST7789_TFT_Clear_White(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint16_t i, j;
    uint8_t temp[16];

    st7789_tft_address_set(x1, y1, x2, y2);
    st7789_tft_write_reg(0x2C);

    for (i = 0; i < 16; i++)
        temp[i] = 0xff;

    for (i = y1; i < y2; i++)
    {
        for (j = x1; j < x2; j++)
        {
            st7789_tft_write_buff(temp, 16);
        }
    }
}

void ST7789_TFT_Clear_Gary(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint16_t i, j;
    uint8_t temp[2];

    st7789_tft_address_set(x1, y1, x2, y2);
    st7789_tft_write_reg(0x2C);

    temp[0] = 0xEF;
    temp[1] = 0x5D;

    for (i = y1; i < y2; i++)
    {
        for (j = x1; j < x2; j++)
        {
            st7789_tft_write_buff(temp, 2);
        }
    }
}

