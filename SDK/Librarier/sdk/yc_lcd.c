/*
File Name    : yc_lcd.c
Author       : Yichip
Version      : V1.0
Date         : 2019/12/07
Description  : lcd encapsulation.
*/
#include "yc_lcd.h"

SPIx_TypeDef LCDSPI;
uint16_t LCD_SDA_PIN;
uint16_t LCD_SCL_PIN;
uint16_t LCD_A0_PIN ;
uint16_t LCD_RES_PIN;
uint16_t LCD_CS_PIN ;

GPIO_TypeDef LCD_SDA_Port;
GPIO_TypeDef LCD_SCL_Port;
GPIO_TypeDef LCD_A0_Port;
GPIO_TypeDef LCD_RES_Port;
GPIO_TypeDef LCD_CS_Port;

uint8_t      LCD_BL_PIN  = GPIO_Pin_7;
GPIO_TypeDef LCD_BL_Port = GPIOA;

void lcd_io_init(LCD_InitTypedef *LCD_InitStruct)
{
    _ASSERT(NULL != LCD_InitStruct);

    SPI_InitTypeDef SPI_InitStruct;

    LCDSPI = LCD_InitStruct->SPIx;

    LCD_SDA_PIN = LCD_InitStruct->IO_LCD_SDA_PIN;
    LCD_SCL_PIN = LCD_InitStruct->IO_LCD_SCL_PIN;
    LCD_A0_PIN  = LCD_InitStruct->IO_LCD_A0_PIN;
    LCD_RES_PIN = LCD_InitStruct->IO_LCD_RES_PIN;
    LCD_CS_PIN  = LCD_InitStruct->IO_LCD_CS_PIN;
    LCD_BL_PIN 	= LCD_InitStruct->IO_LCD_BL_PIN;

    LCD_SDA_Port = LCD_InitStruct->IO_LCD_SDA_Port;
    LCD_SCL_Port = LCD_InitStruct->IO_LCD_SCL_Port;
    LCD_A0_Port  = LCD_InitStruct->IO_LCD_A0_Port;
    LCD_RES_Port = LCD_InitStruct->IO_LCD_RES_Port;
    LCD_CS_Port  = LCD_InitStruct->IO_LCD_CS_Port;
    LCD_BL_Port  = LCD_InitStruct->IO_LCD_BL_Port;

    SPI_InitStruct.BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_InitStruct.CPHA = SPI_CPHA_First_Edge;
    SPI_InitStruct.CPOL = SPI_CPOL_Low;
    SPI_InitStruct.Mode =  SPI_Mode_Master;
    SPI_InitStruct.RW_Delay = 33;

    GPIO_Config(LCD_SCL_Port, LCD_SCL_PIN, SPID0_SCK);
    GPIO_Config(LCD_CS_Port, LCD_CS_PIN, SPID0_NCS);
    GPIO_Config(LCD_SDA_Port, LCD_SDA_PIN, SPID0_MOSI);
    GPIO_Config(LCD_BL_Port, LCD_BL_PIN, OUTPUT_LOW);
    SPI_Init(LCDSPI, &SPI_InitStruct);

    GPIO_Config(LCD_A0_Port, LCD_A0_PIN, OUTPUT_HIGH);
    GPIO_Config(LCD_RES_Port, LCD_RES_PIN, OUTPUT_HIGH);
    delay_ms(20);
    GPIO_Config(LCD_RES_Port, LCD_RES_PIN, OUTPUT_LOW);
    delay_ms(100);
    GPIO_Config(LCD_RES_Port, LCD_RES_PIN, OUTPUT_HIGH);
}

static void clr_a0(void)
{
    GPIO_Config(LCD_A0_Port, LCD_A0_PIN, OUTPUT_LOW);
    GPIO_Config(LCD_A0_Port, LCD_A0_PIN, OUTPUT_LOW);
    GPIO_Config(LCD_A0_Port, LCD_A0_PIN, OUTPUT_LOW);
}

static void set_a0(void)
{
    GPIO_Config(LCD_A0_Port, LCD_A0_PIN, OUTPUT_HIGH);
    GPIO_Config(LCD_A0_Port, LCD_A0_PIN, OUTPUT_HIGH);
    GPIO_Config(LCD_A0_Port, LCD_A0_PIN, OUTPUT_HIGH);
}

static void spi_write_data(unsigned char data)
{
    set_a0();
    SPI_SendData(LCDSPI, data);
}

void spi_write_cmd(unsigned char cmd)
{
    clr_a0();
    SPI_SendData(LCDSPI, cmd);
}

void write_byte_lcd(unsigned char data, unsigned char cmd)
{
    if (LCD_RS_CMD  == cmd)
    {
        spi_write_cmd(data);
    }
    else if (LCD_RS_DATA  == cmd)
    {
        spi_write_data(data);
    }
}

void write_bytes_lcd(unsigned char *pdata, unsigned char len, unsigned char cmd)
{
    unsigned char m = 0;

    if (LCD_RS_CMD  == cmd)
    {
        for (m = 0; m < len ; m++)
        {
            spi_write_cmd(pdata[m]);
        }
    }
    else if (LCD_RS_DATA  == cmd)
    {
        for (m = 0; m < len ; m++)
        {
            spi_write_data((int)(pdata[m]));
        }
    }
}

void clr_screen(void)
{
    int i, page;

    for (page = 0; page < 8; page++)
    {
        spi_write_cmd(0x10);
        spi_write_cmd(0x00);
        spi_write_cmd(page + 0xb0);
        for (i = 0; i < 128; i++)
        {
            spi_write_data(0);
        }
    }
}

void full_screen(void)
{
    int i, page;

    for (page = 0; page < 8; page++)
    {
        spi_write_cmd(0x10);
        spi_write_cmd(0x00);
        spi_write_cmd(page + 0xb0);
        for (i = 0; i < 128; i++)
        {
            spi_write_data(0xFF);
        }
    }
}

void set_xy(unsigned char column, unsigned char page)
{
    unsigned char cmd[3];

    cmd[0] = 0xb0 | page;
    cmd[1] = 0x10 | (column >> 4);
    cmd[2] = 0x00 | (column & 0x0f);
    write_bytes_lcd(cmd, 3, LCD_RS_CMD);
}

void init_lcd(void)
{
    delay_ms(10);
    spi_write_cmd(0xe2);//reset
    delay_ms(10);
    spi_write_cmd(0xa0);// SEG Direction
    delay_ms(10);
    spi_write_cmd(0xc8);//COM Direction
    delay_ms(10);
    spi_write_cmd(0xa2);//Bias Select
    delay_ms(10);
    spi_write_cmd(0x2f);//Power Control
    delay_ms(10);
    spi_write_cmd(0x24);//Regulation Ratio
    delay_ms(10);
    spi_write_cmd(0x81);//Set EV_1
    delay_ms(10);
    spi_write_cmd(0x1e);//Set EV_2
    delay_ms(10);
    spi_write_cmd(0xaf);//Display ON
    delay_ms(10);
}
