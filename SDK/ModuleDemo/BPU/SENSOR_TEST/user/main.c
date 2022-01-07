/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\BPU\SENSOR_TEST\use\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    15-July-2020
  * @brief   Yc3121 SENSOR test program.
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
#include "yc_gpio.h"
#include "yc_uart.h"
#include "yc_ssc.h"
#include "misc.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t Datain[6] = {0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555};
uint32_t Dataout[6];

/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);
void NVIC_Configuration(void);
void ssc_enable(void);
void tamper_test(void);
void sensor_test(void);
void BPK_RW_test(void);

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    UART_Configuration();
    NVIC_Configuration();

    MyPrintf("YcChip Yc3121 SENSOR test Demo V1.0.\n");

    printv((uint8_t *)Datain, 24, "Datain");
    printv((uint8_t *)Dataout, 24, "Dataout");
    MyPrintf("\nf8400 LPM_CTRL is      %08x\n", lpm_read(LPM_CTRL));
    MyPrintf("f8404 LPM_SENSOR is    %08x\n", lpm_read(LPM_SENSOR));
    MyPrintf("f8414 LPM_GPIO_WKHI is %08x\n", lpm_read(LPM_GPIO_WKHI));
    MyPrintf("f8478 LPM_STATUS is    %08x\n", lpm_read(LPM_STATUS));
    MyPrintf("\n");

    ssc_enable();
    tamper_test();
    sensor_test();
    BPK_RW_test();
    SSC_ClearKeyCMD(ENABLE);
//	SSC_LPMLock();

    MyPrintf("f8400 LPM_CTRL is      %08x\n", lpm_read(LPM_CTRL));
    MyPrintf("f8404 LPM_SENSOR is    %08x\n", lpm_read(LPM_SENSOR));
    MyPrintf("f8414 LPM_GPIO_WKHI is %08x\n", lpm_read(LPM_GPIO_WKHI));
    MyPrintf("f8478 LPM_STATUS is    %08x\n", lpm_read(LPM_STATUS));
    MyPrintf("\n");
    while (1)
    {

    }
}

void NVIC_Configuration(void)
{
    NVIC_EnableIRQ(SEC_IRQn);
    NVIC_SetPriority(SEC_IRQn, 0);
}

/**
  * @brief  Serial port initialization function.
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
    - BaudRate = 115200 baud
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
  * @brief  ssc enable function.
  * @param  None
  * @retval None
  */
void ssc_enable(void)
{
    SSC_Interval(INTERVAL_999MS);
    SSC_LPMSheildingAlarmEnable(ENABLE);
}

/**
  * @brief  tamper test function.
  * @param  None
  * @retval None
  */
void tamper_test(void)
{
    TAMPER_InitTypeDef TAMPER_InitStruct;

    TAMPER_InitStruct.TAMPER_Port_mode     = TAMPER_Port_S01_active | TAMPER_Port_S23_active;
    TAMPER_InitStruct.TAMPER_Port_PullUp   = TAMPER_Port_S1_PU | TAMPER_Port_S3_PU | TAMPER_Port_S4_PU | TAMPER_Port_S5_PU | \
            TAMPER_Port_S6_PU | TAMPER_Port_S7_PU;
    TAMPER_InitStruct.TAMPER_Port_Enable   = TAMPER_Port_ALL;
    TAMPER_InitStruct.TAMPER_GlitchTimes   = TAMPER_GlitchTimes_8ms;
    TAMPER_InitStruct.TAMPER_PUPU_HoldTime = TAMPER_PUPU_HoldTime_always;

    SSC_TemperInit(&TAMPER_InitStruct);
}

/**
  * @brief  sensor test function.
  * @param  None
  * @retval None
  */
void sensor_test(void)
{
    SSC_SensorDur(SENSOR_DUR_8MS);
    SSC_SensorDelay(SENEOR_DELAY_4MS);
    SSC_LPMSensorCmd(LPM_BAT_VDT12L_ENABLE | LPM_BAT_VDT33H_ENABLE | \
                     LPM_BAT_VDT33L_ENABLE | LPM_TEMPERATURE_40_ENABLE | \
                     LPM_TEMPERATURE_120_ENABLE, ENABLE);
}

/**
  * @brief  BPK write & read test function.
  * @param  None
  * @retval None
  */
void BPK_RW_test(void)
{
    SSC_LPMKeyWrite(Datain, 6, 1);
    SSC_LPMKeyRead(Dataout, 6, 1);
    printv((uint8_t *)Dataout, 24, "Dataout");
    MyPrintf("\nf8414 LPM_GPIO_WKHI is %08x\n", lpm_read(LPM_GPIO_WKHI));
}

/**
  * @brief  ssc interrupt service function.
  * @param  None
  * @retval None
  */
void SEC_IRQHandler(void)
{
    MyPrintf("SEC_IRQHandler In\n");
    SSC_LPMKeyRead(Dataout, 6, 1);
    printv((uint8_t *)Dataout, 24, "Dataout");
    MyPrintf("f8478 LPM_STATUS is    %08x\n", lpm_read(LPM_STATUS));
    SSC_LPMClearStatusBit();
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/
