/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\QRDecode\QRDecode_demo\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    22-July-2020
  * @brief   QRDecode tft test demo program.
  ******************************************************************************
  * @attention
  *
  * COPYRIGHT 2020 Yichip Microelectronics
  *
  * The purpose of this firmware is to provide guidance to customers engaged in
    * programming work to guide them smoothly to product development,
    * so as to save their time.
    *
    * Therefore, Yichip microelectronics shall not be responsible for any direct,
    * indirect or consequential damages caused by any content of this gu jiang
    * and/or contained in this code used by the customer in its products.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "yc3121.h"
#include "yc_gpio.h"
#include "yc_uart.h"
#include "yc_spi.h"
#include "yc_st7789.h"
#include "font.h"
#include "yc_kscan.h"
#include "yc_lpm.h"
#include "YC_QRDecode.h"
#include "YC_QRDecode_TypeDef.h"
#include "board_config.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define QR_FLASH_BACKUP_ADDR   (0x1000000 + 4 * 200 * 1024)
#define QR_SCAN_TIME_OUT        (30)
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t camera_setpara[][2] =
{
    {0xfe, 0x00},
//  {0x14, 0x7f},//[1] updown [0]mirror //MIRROR
};
/* Private function prototypes -----------------------------------------------*/
static void UART_Configuration(void);
static void TFT_Configuration(void);
static void QRCode_test();

void TFT_Power_scan(void)
{
    ST7789_TFT_Picture(284, 4, gImage_full_energe);
}

void TFT_Single_scan(void)
{
    ST7789_TFT_Picture(4, 4, gImage_single_l);
}
void Display_InterfaceWindow(void)
{
    uint8_t Show_buff1[] = "Camera demo";
    uint8_t Show_buff2[] = "Please press menu key...";

    ST7789_TFT_Clear_White(0, 0, TFT_COL, TFT_ROW);
    ST7789_TFT_ShowChinese(100, 1, Yichip_ch, 24, 5, 1, WHITE, OLIVE);
    ST7789_TFT_Draw_Piece(0, 27, TFT_COL, 28, DGRAY);
    ST7789_TFT_ShowString(70, 80, 16 * sizeof(Show_buff1), 16, Show_buff1, 32, 1, WHITE, OLIVE);
    ST7789_TFT_ShowString(10, 150, 8 * sizeof(Show_buff2), 8, Show_buff2, 16, 1, WHITE, MAROON);
    TFT_Power_scan();
    TFT_Single_scan();
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    /*Change sysclk to 96M*/
    SYSCTRL_HCLKConfig(SYSCTRL_HCLK_Div_None);

    /*uart init*/
    UART_Configuration();

    MyPrintf("Yichip Yc3121 qrdecode TFT Demo V1.0.\r\n");
    MyPrintf("qrdecode version is :%x\n", SC_GetVersion());
    /*enable ldo*/
    GPIO_Config(GPIOA, GPIO_Pin_2, OUTPUT_HIGH);

    /*tft init */
    TFT_Configuration();
    Display_InterfaceWindow();

    while (1)
    {
        /*start scan test*/
        QRCode_test();
    }
}

/**
  * @brief  Serial port 0 initialization function.
  * @param  None
  * @retval None
  */
static void UART_Configuration(void)
{
    UART_InitTypeDef UART_InitStruct;

    /* Configure serial ports RX and TX for IO. */
    GPIO_Config(UART0_TX_PORT, UART0_TX_PIN, UART0_TXD);
    GPIO_Config(UART0_RX_PORT, UART0_RX_PIN, UART0_RXD);

    /* USARTx configured as follow:
    - BaudRate = 921600 baud
    - Word Length = 8 Bits
    - Stop Bit = 1 Stop Bit
    - Parity = No Parity
    - Hardware flow control disabled (RTS and CTS signals)
    - Receive and transmit enabled
    */

    UART_InitStruct.BaudRate = UARTBAUD;            //Configure serial port baud rate, the baud rate defaults to 128000.

    UART_InitStruct.DataBits = Databits_8b;
    UART_InitStruct.StopBits = StopBits_1;
    UART_InitStruct.Parity = Parity_None;
    UART_InitStruct.FlowCtrl = FlowCtrl_None;
    UART_InitStruct.Mode = Mode_duplex;

    UART_Init(UART0, &UART_InitStruct);
}

/**
  * @brief  TFT related IO port and SPI related configuration
  * @param  None
  * @retval None
  */
static void TFT_Configuration(void)
{

    St7789TFT_InitTypedef St7789TFT_InitStruct;
    St7789TFT_InitStruct.SPIx = ST7789VTFTSPI;
    St7789TFT_InitStruct.IO_TFT_SDA_PIN = ST7789_TFT_SDA_PIN;
    St7789TFT_InitStruct.IO_TFT_SDA_Port = ST7789_TFT_SDA_PORT;
    St7789TFT_InitStruct.IO_TFT_SCL_PIN = ST7789_TFT_SCL_PIN;
    St7789TFT_InitStruct.IO_TFT_SCL_Port = ST7789_TFT_SCL_PORT;
    St7789TFT_InitStruct.IO_TFT_CS_PIN = ST7789_TFT_CS_PIN;
    St7789TFT_InitStruct.IO_TFT_CS_Port = ST7789_TFT_CS_PORT;
    St7789TFT_InitStruct.IO_TFT_RES_PIN = ST7789_TFT_RST_PIN;
    St7789TFT_InitStruct.IO_TFT_RES_Port = ST7789_TFT_RST_PORT;
    St7789TFT_InitStruct.IO_TFT_BL_PIN = ST7789_TFT_BL_PIN;
    St7789TFT_InitStruct.IO_TFT_BL_Port = ST7789_TFT_BL_PORT;
    St7789TFT_InitStruct.IO_TFT_A0_PIN = ST7789_TFT_A0_PIN;
    St7789TFT_InitStruct.IO_TFT_A0_Port = ST7789_TFT_A0_PORT;

    ST7789_TFT_GpioInit(&St7789TFT_InitStruct);
    ST7789_TFT_Init();

}
/**
  * @brief  qrdecode init and enter scan
  * @param  timeout :   qrdecode time(s)
  * @retval  None
  */
void QRCode_Enter(uint8_t timeout)
{
    uint32_t len = 0;
    uint8_t show_messsage_len = 0;
    yc_qr_return_type_ENUM QR_Enter_Ret = QR_SUCCESS;
    uint8_t message[1024] = {0};
    uint32_t i = 0;
    QRDecode_Config_CB  p_QRDecode_Config;

    /*enable halk clk*/
    QSPI_CTRL  |= (0x80);

    /*enable gpio32 function*/
    uint32_t value = lpm_bt_read(LPMBT_CONTROL1);
    value &= ~(1 << 17);
    lpm_bt_write(LPMBT_CONTROL1, value);

    p_QRDecode_Config.lcd_configCB.display_type = CAMERA_TFT;
    p_QRDecode_Config.lcd_configCB.start_column = QR_TFT_START_COLUMN;//(320 - 236)/2;
    p_QRDecode_Config.lcd_configCB.is_init_lcd = TRUE;
    p_QRDecode_Config.lcd_configCB.lcd_pin_rst = QR_TFT_RST;//11;
    p_QRDecode_Config.lcd_configCB.lcd_pin_cs  = QR_TFT_CS;//5;
    p_QRDecode_Config.lcd_configCB.lcd_pin_clk = QR_TFT_CLK;//40;
    p_QRDecode_Config.lcd_configCB.lcd_pin_mosi = QR_TFT_MOSI;//39;
    p_QRDecode_Config.lcd_configCB.lcd_pin_a0  = QR_TFT_A0;//10;
    p_QRDecode_Config.lcd_configCB.lcd_pin_bl  = QR_TFT_BL;//12 | (1 << 7);

    p_QRDecode_Config.keyboard_configCB.mode = QR_KEY_T_MATRIX;
    p_QRDecode_Config.keyboard_configCB.line_num = QR_KEY_LINE_NUM;
    p_QRDecode_Config.keyboard_configCB.col_num = QR_KEY_COL_NUM;
    p_QRDecode_Config.keyboard_configCB.cancel_position = QR_KEY_CANCEL_POSTION;
    p_QRDecode_Config.keyboard_configCB.line1 = QR_KEY_LINE_1;
    p_QRDecode_Config.keyboard_configCB.line2 = QR_KEY_LINE_2;
    p_QRDecode_Config.keyboard_configCB.line3 = QR_KEY_LINE_3;
    p_QRDecode_Config.keyboard_configCB.line4 = QR_KEY_LINE_4;
    p_QRDecode_Config.keyboard_configCB.line5 = QR_KEY_LINE_5;
    p_QRDecode_Config.keyboard_configCB.line6 = QR_KEY_LINE_6;

    p_QRDecode_Config.camera_configCB.camera_type = UNKNOW;
    p_QRDecode_Config.camera_configCB.freq_mclk   = QR_CAMERA_FREQ;//24;
    p_QRDecode_Config.camera_configCB.pin_sda   = QR_CAMERA_SDA;//28;
    p_QRDecode_Config.camera_configCB.pin_scl   = QR_CAMERA_SCL;//29;
    p_QRDecode_Config.camera_configCB.pin_reset = QR_CAMERA_RST;//32;
    p_QRDecode_Config.camera_configCB.pin_pwdn  = QR_CAMERA_PD;//31;
    p_QRDecode_Config.camera_configCB.pin_mclk  = QR_CAMERA_MCLK;//35;
    p_QRDecode_Config.camera_configCB.pin_pclk  = QR_CAMERA_PCLK;//34;
    p_QRDecode_Config.camera_configCB.pin_data  = QR_CAMERA_DATA;//30;
    p_QRDecode_Config.camera_configCB.pin_cs    = QR_CAMERA_CS;//42;
    p_QRDecode_Config.camera_configCB.camera_setparaCB.camera_setpara_num = sizeof(camera_setpara) / 2;

    for (uint8_t i = 0; i < p_QRDecode_Config.camera_configCB.camera_setparaCB.camera_setpara_num; i++)
    {
        p_QRDecode_Config.camera_configCB.camera_setparaCB.camera_setpara[i][0] = camera_setpara[i][0];
        p_QRDecode_Config.camera_configCB.camera_setparaCB.camera_setpara[i][1] = camera_setpara[i][1];
    }
    p_QRDecode_Config.flash_backup_addr = QR_FLASH_BACKUP_ADDR;
    p_QRDecode_Config.timeout = timeout;

    ST7789_TFT_Clear_White(0, 0, TFT_COL, TFT_ROW);
    QR_Enter_Ret = YC_QRDecode(&p_QRDecode_Config, &len, message);

    ST7789_TFT_Clear_White(0, 0, TFT_COL, TFT_ROW);

    if (QR_Enter_Ret ==  QR_SUCCESS)
    {

        UART_SendBuf(UART0, message, len);
        if (len > 250) show_messsage_len = 250;
        else show_messsage_len = len;
        ST7789_TFT_ShowString(0, 0, show_messsage_len * 12,  12, message, 24, 1, WHITE, OLIVE);

        return;
    }

    switch (QR_Enter_Ret)
    {
        case QR_CANCEL:
            ST7789_TFT_ShowString(50, 100, 12 * sizeof("QR_CANCEL"), 12, (uint8_t *)"QR_CANCEL", 24, 1, WHITE, MAROON);
            break;
        case QR_CAMERA_PWM_INIT_FAIL:
            ST7789_TFT_ShowString(50, 100, 12 * sizeof("QR_CAMERA_PWM_INIT_FAIL"), 12, (uint8_t *)"QR_CAMERA_PWM_INIT_FAIL", 24, 1, WHITE, MAROON);
            break;
        case QR_CAMERA_IIC_INIT_FAIL:
            ST7789_TFT_ShowString(50, 100, 12 * sizeof("QR_CAMERA_IIC_INIT_FAIL"), 12, (uint8_t *)"QR_CAMERA_IIC_INIT_FAIL", 24, 1, WHITE, MAROON);
            break;
        case QR_CAMERA_OPEN_FAIL:
            ST7789_TFT_ShowString(50, 100, 12 * sizeof("QR_CAMERA_OPEN_FAIL"), 12, (uint8_t *)"QR_CAMERA_OPEN_FAIL", 24, 1, WHITE, MAROON);
            break;
        case QR_CAMERA_READ_FAIL:
            ST7789_TFT_ShowString(50, 100, 12 * sizeof("QR_CAMERA_READ_FAIL"), 12, (uint8_t *)"QR_CAMERA_READ_FAIL", 24, 1, WHITE, MAROON);
            break;
        case QR_KEY_PARA_FAIL:
            ST7789_TFT_ShowString(50, 100, 12 * sizeof("QR_KEY_PARA_FAIL"), 12, (uint8_t *)"QR_KEY_PARA_FAIL", 24, 1, WHITE, MAROON);
            break;
        case QR_CANCEL_KEY:
            ST7789_TFT_ShowString(50, 100, 12 * sizeof("QR_CANCEL_KEY"), 12, (uint8_t *)"QR_CANCEL_KEY", 24, 1, WHITE, MAROON);
            break;
        case QR_CANCEL_TIMEOUT:
            ST7789_TFT_ShowString(50, 100, 12 * sizeof("QR_CANCEL_TIMEOUT"), 12, (uint8_t *)"QR_CANCEL_TIMEOUT", 24, 1, WHITE, MAROON);
            break;
        default:
            ST7789_TFT_ShowString(50, 100, 12 * sizeof("UNUSUAL"), 21, (uint8_t *)"UNUSUAL", 24, 1, WHITE, MAROON);
            break;
    }
    ST7789_TFT_ShowChinese(100, 1, Yichip_ch, 24, 5, 1, WHITE, OLIVE);
    ST7789_TFT_Draw_Piece(0, 27, TFT_COL, 28, DGRAY);
    TFT_Power_scan();
    TFT_Single_scan();

    return ;
}

/**
  * @brief  Qrdecode Test
  * @param  None
  * @retval  None
  */
static void QRCode_test()
{
    uint8_t keyCode = 0;
    uint8_t timeout = 0;

    while (1)
    {
        do
        {
            keyCode = KEY_Scan();
        }
        while (keyCode != KEY_MENU);

        while (1)
        {
            QRCode_Enter(QR_SCAN_TIME_OUT);
            ST7789_TFT_ShowString(0, 200, sizeof("CANCEL             CONFIRM") * 12,  12, (uint8_t *)"CANCEL             CONFIRM", 24, 1, WHITE, OLIVE);
            while (1)
            {
                keyCode = KEY_Scan();
                if (keyCode == KEY_CANCEL)
                {
                    Display_InterfaceWindow();
                    return;//exit
                }
                else if (keyCode == KEY_CONFIRM || timeout != 0)
                {
                    break; // continue
                }
            }
        }
    }
}

/**
  * @brief  Bctc_Qrtest_parse_cmd
  * @param  None
  * @retval  cmd
  */
static uint8_t Bctc_Qrtest_parse_cmd(void)
{
    const uint8_t s[] = "101099";
    uint8_t buf[10], r;
    uint8_t src[] = {"\nffff\n"};
    uint8_t src1[] = {"00"};
    if (UART_IsRXFIFONotEmpty(UART0))
    {
        delay_ms(50);
        r = UART_RecvBuf(UART0, buf, sizeof(buf));
        buf[r] = '\0';
        if (strncmp((char *)buf, (char *)s, 6) == 0)
        {
            if (strncmp((char *)buf + 6, (char *)src1, 2) == 0)
            {
                UART_SendBuf(UART0, src, sizeof(src) - 1);
            }
            else
            {
                return (uint8_t)((buf[6] - 0x30) * 10 + (buf[7] - 0x30));
            }
        }
    }
    return 0;
}


/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
