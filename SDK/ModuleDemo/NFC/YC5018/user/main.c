/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\NFC\YC5018\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.1
  * @date    21-July-2020
  * @brief
  ******************************************************************************
  * @attention
  *
  * COPYRIGHT 2019 Yichip Microelectronics
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
#include "yc_systick.h"
#include "yc_ipc.h"
#include "yc_nfc_common.h"
#include "yc_emv_contactless_l1.h"
#include "yc_timer.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SOFTWARE_RESET 1
#define EXTERNAL_IRRIGATION_CLOCK 1

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern EMV_Poll_Type_t Poll_Type;

/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void NFC_Spi_Init(void);
void NFC_Systick_Init(void);
void NFC_CoreReg_Init(uint8_t Software_reset, uint8_t External_irrigation_clock);
unsigned char Nfc_Apdu_Transmission_Test(void);
unsigned char Nfc_Active_card(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

int main(void)
{
    uint8_t errno = 0;
    uint8_t Resart = 0;

    UART_Configuration();
#if (BOARD_VER == EPOS_BOARD_V1_0)
    GPIO_Config(GPIOA, GPIO_Pin_2, OUTPUT_HIGH);
#endif
    MyPrintf("\nYC5018 RF only, Date: %s, %s\r\n", __DATE__, __TIME__);
    NFC_Spi_Init();
	NFC_CoreReg_Init(SOFTWARE_RESET, EXTERNAL_IRRIGATION_CLOCK);
#if (BOARD_VER == EPOS_BOARD_V1_0)
    GPIO_Config(NFC_TVDD_PORT, NFC_TVDD_PIN, OUTPUT_LOW); //NFC tvdd enable
#endif

	#ifdef READ_RxVpp
	Nfc_SysTick_Delay_Ms(200);
	Aset_read_power_xRSSI_RXVPP();
	NFC_CoreReg_Init(SOFTWARE_RESET, EXTERNAL_IRRIGATION_CLOCK);
	#endif
    /*Contactless Init*/
    YC_EMV_Contactless_L1_Init();
    /*Open Rf*/
    YC_EMV_Contactless_L1_Rf_Switch(ON);
    MyPrintf("please swiping card\n");
    NFC_time_start();

    while (1)
    {	
		if(Resart == 1)
		{
			NFC_CoreReg_Init(SOFTWARE_RESET, EXTERNAL_IRRIGATION_CLOCK);
			SysTick_Delay_Ms(500);
			YC_EMV_Contactless_L1_Init();
			YC_EMV_Contactless_L1_Rf_Switch(ON);
			Resart = 0;
			#ifdef SDK_DEBUG
			MyPrintf("NFC Resart EOT_SHOUT\n");
			#endif
		}
		GPIO_Config(GPIOA, GPIO_Pin_14, OUTPUT_LOW);
		SysTick_Delay_Ms(50);
		errno = Nfc_Active_card();
		if(errno == EOT_SHOUT)
		{	
			#ifdef SDK_DEBUG
			MyPrintf("NFC Active_card EOT_SHOUT\n");
			#endif
			Resart = 1;
		}
        if (errno == EMV_ERR_NONE)
        {
			errno = Nfc_Apdu_Transmission_Test();
			if(errno == EOT_SHOUT)
			{
				Resart = 1;
				#ifdef SDK_DEBUG
				MyPrintf("NFC Apdu Transmission EOT_SHOUT\n");
				#endif

			}
            if (errno == EMV_ERR_NONE)
            {
                MyPrintf("Nfc Apdu Transmission Test Sucess\n");
                MyPrintf("Please Swipe Your Card!\n");
            }
            else
            {
                MyPrintf("Nfc Apdu Transmission Test Fail\n");
                MyPrintf("Please Swipe Your Card!\n");
            }

            YC_EMV_Contactless_L1_Rf_Switch(OFF);

            SysTick_Delay_Ms(6);

            YC_EMV_Contactless_L1_Rf_Switch(ON);
        }
    }
}

/**
  * @brief  Serial port 0 initialization function.
  * @param  None
  * @retval None
  */
void UART_Configuration(void)
{
    UART_InitTypeDef UART_InitStruct;

    /* Configure serial ports RX and TX for IO. */
    GPIO_Config(UART0_TX_PORT, UART0_TX_PIN, UART0_TXD);
    GPIO_Config(UART0_RX_PORT, UART0_RX_PIN, UART0_RXD);

    /* USARTx configured as follow:
    - BaudRate = baud
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

void NFC_Spi_Init(void)
{
    SPI_InitTypeDef SPI_InitStruct;
    SPI_InitStruct.Mode = SPI_Mode_Master;
    SPI_InitStruct.BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStruct.CPHA = SPI_CPHA_First_Edge;
    SPI_InitStruct.CPOL = SPI_CPOL_Low;
    SPI_InitStruct.RW_Delay = 3;

    GPIO_Config(NFC_SPI_MISO_PORT, NFC_SPI_MISO_PIN, SPID1_MISO);
    GPIO_Config(NFC_SPI_MOSI_PORT, NFC_SPI_MOSI_PIN, SPID1_MOSI);
    GPIO_Config(NFC_SPI_SCL_PORT, NFC_SPI_SCL_PIN, SPID1_SCK);
    GPIO_Config(NFC_SPI_CS_PORT, NFC_SPI_CS_PIN, SPID1_NCS);
    SPI_Init(NFC_SPI, &SPI_InitStruct);
}

/*
函数功能:  APDU 传输测试
*/
unsigned char Nfc_Apdu_Transmission_Test(void)
{
    uint8_t err_no, ResBuff[256];
    uint16_t ResLen;
    const uint8_t ApduBuff[] = {0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00};
    err_no = YC_EMV_Contactless_L1_APDU_Transceive(ApduBuff, sizeof(ApduBuff), ResBuff, &ResLen);
    return err_no;
}

/*
函数功能: 进行TypeA，TypeB 轮询寻卡，有卡进行冲突检测，并激活
*/
unsigned char Nfc_Active_card(void)
{
    unsigned char i, err_no = EMV_ERR_NONE;
    Poll_Type.Type_A = 0;
    Poll_Type.Type_B = 0;
    if (!Poll_Type.Type_A)
    {
		err_no = YC_EMV_Contactless_L1_WUPA();
		if(err_no == EOT_SHOUT )
			return EOT_SHOUT;
        if (err_no!= EMV_ERR_TIME_OUT)
        {
            Poll_Type.Type_A = 1;
            YC_EMV_Contactless_L1_HLTA();
        }
        if (!Poll_Type.Type_B)
        {
			err_no = YC_EMV_Contactless_L1_WUPB(NULL);
			if(err_no == EOT_SHOUT )
				return EOT_SHOUT;
            if (err_no != EMV_ERR_TIME_OUT)
            {
                Poll_Type.Type_B = 1;
            }
        }
    }
    if (!(Poll_Type.Type_A || Poll_Type.Type_B)) 
		return EMV_ERR_NO_CARD;

    if (Poll_Type.Type_A && Poll_Type.Type_B) 
		return EMV_ERR_COLLISION;

    else if (Poll_Type.Type_A)
	{
		err_no = EMV_A_Collision_Detect();
		if(err_no == EOT_SHOUT )
				return EOT_SHOUT;
	}
    else if (Poll_Type.Type_B) 
	{
		err_no = EMV_B_Collision_Detect();
		if(err_no == EOT_SHOUT )
				return EOT_SHOUT;
	}
    if (err_no == EMV_ERR_NONE)
    {
        if (Poll_Type.Type_A) 
		{
			err_no = YC_EMV_Contactless_L1_RATS();
			if(err_no == EOT_SHOUT )
				return EOT_SHOUT;
		}

        else if (Poll_Type.Type_B) 
		{
			err_no = YC_EMV_Contactless_L1_ATTRIB();
			if(err_no == EOT_SHOUT )
				return EOT_SHOUT;
		}
    }
    return err_no;
}

/*
函数功能:  EMV时钟配置
*/
void NFC_Systick_Init(void)
{
    IpcInit();
    GPIO_Config(NFC_12M_CLK_PORT, NFC_12M_CLK_PIN, NFC_CLK_OUT);
}
/*
函数功能:  EMV上电初始化配置
形参：Software_reset ：置1 EMV Reset 接主控IO控制；置0 EMV Reset 拉高
	  External_irrigation_clock ：置1 ：EMV时钟不由晶体供给；置0 EMV时钟由晶体供给
*/
void NFC_CoreReg_Init(uint8_t Software_reset, uint8_t External_irrigation_clock)
{
    uint8_t RegBuff[12];

    if (Software_reset)
    {
        if (External_irrigation_clock)
        {
            NFC_Systick_Init();
        }

        Nfc_SysTick_Delay_Ms(10);

        YC_EMV_HwReset();
        if (External_irrigation_clock)
        {
            WRITEREG(130, 0xB4);
            WRITEREG(137, 0x24);
            WRITEREG(129, 0x5b);
        }
		dc_cali_alg(3);
        YC_EMV_HwReset();
        if (External_irrigation_clock)
        {
            WRITEREG(130, 0xB4);
            WRITEREG(137, 0x24);
            WRITEREG(129, 0x5b);
            PRINT("130 = 0x%x\r\n", READREG(130));
            PRINT("137 = 0x%x\r\n", READREG(137));
            PRINT("129 = 0x%x\r\n", READREG(129));
        }
    }
    else
    {
        if (External_irrigation_clock)
        {
            NFC_Systick_Init();
        }
        Nfc_SysTick_Delay_Ms(500);
        WRITEREG(171, 0x04);
        WRITEREG(174, 0x04);
        if (External_irrigation_clock)
        {
            WRITEREG(130, 0xB4);
            WRITEREG(137, 0x24);
            WRITEREG(129, 0x5b);

            PRINT("130 = 0x%x\r\n", READREG(130));
            PRINT("137 = 0x%x\r\n", READREG(137));
            PRINT("129 = 0x%x\r\n", READREG(129));

        }

        RegBuff[0] = READREG(1);
        RegBuff[1] = READREG(148);
        RegBuff[2] = READREG(83);
        RegBuff[3] = READREG(65);
        RegBuff[4] = READREG(66);
        RegBuff[5] = READREG(68);
        RegBuff[6] = READREG(69);
        RegBuff[7] = READREG(149);
        RegBuff[8] = READREG(150);
        RegBuff[9] = READREG(151);
        RegBuff[10] = READREG(171);
        RegBuff[11] = READREG(174);

        dc_cali_alg(3);

        WRITEREG(1, RegBuff[0]);
        WRITEREG(148, RegBuff[1]);
        WRITEREG(183, RegBuff[2]);
        WRITEREG(65, RegBuff[3]);
        WRITEREG(66, RegBuff[4]);
        WRITEREG(68, RegBuff[5]);
        WRITEREG(69, RegBuff[6]);
        WRITEREG(149, RegBuff[7]);
        WRITEREG(150, RegBuff[8]);
        WRITEREG(151, RegBuff[9]);
        WRITEREG(171, RegBuff[10]);
        WRITEREG(174, RegBuff[11]);
    }
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
