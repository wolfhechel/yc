#ifndef __QRDECODE_TYPEDEF_H__
#define __QRDECODE_TYPEDEF_H__

#define CAMERA_SETPARA_NUM_MAX  (100)

typedef struct _keyboard_config_CB_
{
    unsigned char mode;	//0: T KeyBoard 1:Matrix KeyBoard 2:powerKey exit
		unsigned char line_num;
	  unsigned char col_num;
    unsigned char cancel_position;
    unsigned char line1;
    unsigned char line2;
    unsigned char line3;
    unsigned char line4;
    unsigned char line5;
    unsigned char line6;
    unsigned char line7;
    unsigned char line8;
    unsigned char line9;
    unsigned char line10;
} keyboard_config_CB;

typedef enum {UNKNOW =0,GC6153 = 1, BF3A02 = 2, BF30A2 = 3} camera_type_ENUM;
typedef enum {CAMERA_LCD = 0, CAMERA_TFT = 1} camera_disp_ENUM;

typedef struct _lcd_config_CB_
{
    Boolean is_init_lcd;
	camera_disp_ENUM display_type;
    unsigned char lcd_pin_rst;
    unsigned char lcd_pin_cs;
    unsigned char lcd_pin_clk;
    unsigned char lcd_pin_mosi;
    unsigned char lcd_pin_a0;
    unsigned char lcd_pin_bl;     //bit7=1:  high light          =0:  low  light
	unsigned char blank_line_number;  //Its implementation begins to count
    unsigned char column_number;    //display column size
    unsigned char start_column; // disstart cloumn 
    Boolean is_turn_right; // picture turn right 90
    unsigned char spi_baudspeed;//use spi SPI_BaudSpeed define
} lcd_config_CB;

typedef struct _camera_setpara_CB_
{
    unsigned char camera_setpara_num;
    unsigned char camera_setpara[CAMERA_SETPARA_NUM_MAX][2];
}camera_setpara_CB;

typedef struct _camera_config_CB_
{
    unsigned char freq_mclk;
    camera_type_ENUM camera_type;
    unsigned char pin_sda;
    unsigned char pin_scl;
    unsigned char pin_reset;
    unsigned char pin_pwdn;
    unsigned char pin_mclk;
    unsigned char pin_pclk;
    unsigned char pin_data;
    unsigned char pin_cs;
    camera_setpara_CB camera_setparaCB;
} camera_config_CB;

typedef struct _QRDecode_Config_CB_
{
    lcd_config_CB      lcd_configCB;
    keyboard_config_CB keyboard_configCB;
    camera_config_CB   camera_configCB;
    unsigned int       flash_backup_addr;
	unsigned char      timeout;				//unit:  s
} QRDecode_Config_CB;

typedef enum {
QR_CANCEL = 0,
QR_SUCCESS = 1,
QR_CAMERA_PWM_INIT_FAIL = -1,
QR_CAMERA_IIC_INIT_FAIL = -2,
QR_CAMERA_OPEN_FAIL = -3,
QR_CAMERA_READ_FAIL = -4,
QR_KEY_PARA_FAIL = -5,
QR_CANCEL_KEY= -6,
QR_CANCEL_TIMEOUT= -7,
QR_BT_CONNECT_FAIL= -8,
QR_FLASH_PARA_FAIL= -9,
QR_BT_RECEIVE_FAIL= -10,
QR_SPI_RECV_DATA_FAIL= -11,
}yc_qr_return_type_ENUM;


#endif
