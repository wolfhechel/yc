/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\QRDecode\QRDecode_demo\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    22-July-2020
  * @brief   QRDecode lcd test demo program.
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
#include "yc_lcd.h"
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
//	{0x14, 0x7f},//[1] updown [0]mirror //MIRROR
};

/* Private function prototypes -----------------------------------------------*/
static void UART_Configuration(void);
static void LCD_Configuration(void);
static uint8_t Bctc_Qrtest_parse_cmd(void);
static void QRCode_LCD_test();


/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    /*change sysclk to 96M*/
    SYSCTRL_HCLKConfig(SYSCTRL_HCLK_Div_None);
    
   /*uart init*/
    UART_Configuration();
    
    MyPrintf("Yichip Yc3121 qrdecode Demo V1.0.\r\n");
    MyPrintf("qrdecode version is :%x\n",SC_GetVersion());

    /*lcd init*/
    LCD_Configuration();
    
    /*start scan test*/
    QRCode_LCD_test();
    
    while (1)
    {
        
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

    UART_InitStruct.BaudRate = UARTBAUD;			//Configure serial port baud rate, the baud rate defaults to 128000.
    UART_InitStruct.DataBits = Databits_8b;
    UART_InitStruct.StopBits = StopBits_1;
    UART_InitStruct.Parity = Parity_None;
    UART_InitStruct.FlowCtrl = FlowCtrl_None;
    UART_InitStruct.Mode = Mode_duplex;

    UART_Init(UART0, &UART_InitStruct);
}



/**
  * @brief  lcd ,camera,config,qrdecode scan 
  * @param  None
  * @retval  None
  */
static void QRCode_LCD_test()
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
    
    p_QRDecode_Config.lcd_configCB.display_type = CAMERA_LCD;
    p_QRDecode_Config.lcd_configCB.start_column = QR_LCD_START_COLUMN;//(128 - 64)/2;
    p_QRDecode_Config.lcd_configCB.is_init_lcd = TRUE;
    p_QRDecode_Config.lcd_configCB.lcd_pin_rst = QR_LCD_RST;//14;
    p_QRDecode_Config.lcd_configCB.lcd_pin_cs  = QR_LCD_CS;//15;
    p_QRDecode_Config.lcd_configCB.lcd_pin_clk = QR_LCD_CLK;//10;
    p_QRDecode_Config.lcd_configCB.lcd_pin_mosi = QR_LCD_MOSI;//3;
    p_QRDecode_Config.lcd_configCB.lcd_pin_a0  = QR_LCD_A0;//11;
    p_QRDecode_Config.lcd_configCB.lcd_pin_bl  = QR_LCD_BL;//39 | (1 << 7);
    p_QRDecode_Config.lcd_configCB.spi_baudspeed = QR_LCD_SPI_BOUDSPEED;//0xFF;
    p_QRDecode_Config.lcd_configCB.blank_line_number = QR_LCD_BLANK_LINE_NUM;//0;
    p_QRDecode_Config.lcd_configCB.column_number = QR_LCD_COLUMN_NUM;//64;
    p_QRDecode_Config.lcd_configCB.is_turn_right = FALSE;

    p_QRDecode_Config.camera_configCB.camera_type = UNKNOW;
    p_QRDecode_Config.camera_configCB.freq_mclk   = QR_LCD_CAMERA_FREQ;//24;
    p_QRDecode_Config.camera_configCB.pin_sda   = QR_LCD_CAMERA_SDA;//17;
    p_QRDecode_Config.camera_configCB.pin_scl   = QR_LCD_CAMERA_SCL;//16;
    p_QRDecode_Config.camera_configCB.pin_reset = QR_LCD_CAMERA_RST;//41;
    p_QRDecode_Config.camera_configCB.pin_pwdn  = QR_LCD_CAMERA_PD;//5;
    p_QRDecode_Config.camera_configCB.pin_mclk  = QR_LCD_CAMERA_MCLK;//40;
    p_QRDecode_Config.camera_configCB.pin_pclk  = QR_LCD_CAMERA_PCLK;//38;
    p_QRDecode_Config.camera_configCB.pin_data  = QR_LCD_CAMERA_DATA;//2;
    p_QRDecode_Config.camera_configCB.pin_cs    = QR_LCD_CAMERA_CS;//42;
    p_QRDecode_Config.camera_configCB.camera_setparaCB.camera_setpara_num = sizeof(camera_setpara) / 2;

    for (uint8_t i = 0; i < p_QRDecode_Config.camera_configCB.camera_setparaCB.camera_setpara_num; i++)
    {
        p_QRDecode_Config.camera_configCB.camera_setparaCB.camera_setpara[i][0] = camera_setpara[i][0];
        p_QRDecode_Config.camera_configCB.camera_setparaCB.camera_setpara[i][1] = camera_setpara[i][1];
    }
    p_QRDecode_Config.flash_backup_addr = QR_FLASH_BACKUP_ADDR;
    p_QRDecode_Config.timeout = QR_SCAN_TIME_OUT;

    MyPrintf("start scanning\n");
    
    QR_Enter_Ret = YC_QRDecode(&p_QRDecode_Config, &len, message);
    
    if(QR_Enter_Ret == QR_SUCCESS)
    {
        MyPrintf("qrdecode success \n");
        MyPrintf("len : %d \n",len);
        MyPrintf("message : ");
        
        for(uint8_t i = 0; message[i] != '\0'; i++ )
        {
            MyPrintf("%c",message[i]);
        }
        MyPrintf("\n");
    }
    else
    {
        MyPrintf("qrdecode faild , you can reset and scan again\n");
    }
}

/**
  * @brief  lcd config
  * @param  None
  * @retval  None
  */
static void LCD_Configuration(void)
{
    LCD_InitTypedef LCD_ConfigureStructure;

    LCD_ConfigureStructure.SPIx = SPI0;
    LCD_ConfigureStructure.IO_LCD_SDA_Port = GPIOA;
    LCD_ConfigureStructure.IO_LCD_SDA_PIN  = GPIO_Pin_3;
    LCD_ConfigureStructure.IO_LCD_SCL_Port = GPIOA;
    LCD_ConfigureStructure.IO_LCD_SCL_PIN  = GPIO_Pin_10;
    LCD_ConfigureStructure.IO_LCD_CS_Port  = GPIOA;
    LCD_ConfigureStructure.IO_LCD_CS_PIN   = GPIO_Pin_15;
    LCD_ConfigureStructure.IO_LCD_RES_Port = GPIOA;
    LCD_ConfigureStructure.IO_LCD_RES_PIN  = GPIO_Pin_14;
    LCD_ConfigureStructure.IO_LCD_A0_Port  = GPIOA;
    LCD_ConfigureStructure.IO_LCD_A0_PIN   = GPIO_Pin_11;
    LCD_ConfigureStructure.IO_LCD_BL_Port  = GPIOC;
    LCD_ConfigureStructure.IO_LCD_BL_PIN   = GPIO_Pin_7;
    
    lcd_io_init(&LCD_ConfigureStructure);

    /* Initialize the LCD screen */
    init_lcd();

    clr_screen();
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
