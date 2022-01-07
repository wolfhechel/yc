/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\BlueTooth\BT&BLE\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    18-Feb-2020
  * @brief   BT&BLE test program.
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
#include "yc_bt.h"
#include "yc_systick.h"
#include "yc_qspi.h"
#include "yc_ipc.h"
#include "board_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define IPC_UART UART0
#define UART_TO_IPC_BAUDRATE	921600
//#define UART_TO_IPC   //开启此宏定义后。1、蓝牙收到的数据直接通过串口打印出。2、支持串口hci命令设置。

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);



/*
新建services数组格式如下：
byte0：service uuid length, 0x10即16个字节长度的uuid
byte1-16: uuid, 49535343-FE78-4AE5-8FA9-9FAFD205E455   
*/
uint8_t ble_service_uuid_test[] = {0x10,0x55,0xe4,0x05,0xd2,0xaf,0x9f,0xa9,0x8f,0xe5,0x4a,0x7d,0xfe,0x43,0x53,0x53,0x49};

/*
新建特征数组格式如下：
byte0: 特征的属性,即此uuid支持的功能。
    bit7-ExtendedProperites
    bit6-AuthenticatedSigned Writes
    bit5-Indicate
    bit4-Notify
    bit3-Write
    bit2-Write WithoutResponse
    bit1-Read
    bit0-Broadcast

byte1:characterisitic uuid length  
byte2-17:characterisitic uuid 
byte18:write/read payload length 
byte19-20:write/read payload
*/
//49535343-1E4D-4BD9-BA61-23C647249616
uint8_t ble_Characteristic_uuid_test_tx[] = {0x10,0x10,0x16,0x96,0x24,0x47,0xc6,0x23,0x61,0xba,0xd9,0x4b,0x4d,0x1e,0x43,0x53,0x53,0x49,0x01,0x00};
//49535343-8841-43F4-A8D4-ECBE34729BB3
uint8_t ble_Characteristic_uuid_test_rx[] = {0x0c,0x10,0xb3,0x9b,0x72,0x34,0xbe,0xec,0xd4,0xa8,0xf4,0x43,0x41,0x88,0x43,0x53,0x53,0x49,0x01,0x00};
//49535343-aca3-481c-91ec-d85e28a60318
uint8_t ble_Characteristic_uuid_test_ctrl[] = {0x18,0x10,0x18,0x03,0xa6,0x28,0x5e,0xd8,0xec,0x91,0x1c,0x48,0xa3,0xac,0x43,0x53,0x53,0x49,0x01,0x00};
  
//ble ff000服务
uint8_t ble_service_uuid_lsps[] = {0x02,0x00,0xff};
uint8_t ble_Characteristic_uuid_lsps_tx[] = {0x10,0x02,0x01,0xff};
uint8_t ble_Characteristic_uuid_lsps_rx[] = {0x0c,0x02,0x02,0xff};
uint8_t ble_Characteristic_uuid_flow_ctrl[] = {0x10,0x02,0x3,0xff,0x01,0x01};   //流控的uuid

//#define DingDangTongXue       //打开宏定义后可与叮当同学app连接打印

#define AiLiXiong               //打开宏定义后可与爱立熊app连接打印

#ifdef AiLiXiong
uint8_t Ai_Resp1[80]={
0x50,0x65,0x72,0x69,0x50,0x61,0x67,0x65,0x2b,0x45,0x30,0x42,0x45,0x7c,0x43,0x38,
0x3a,0x34,0x37,0x3a,0x38,0x43,0x3a,0x30,0x41,0x3a,0x45,0x30,0x3a,0x42,0x45,0x7c,
0x43,0x38,0x3a,0x34,0x37,0x3a,0x38,0x43,0x3a,0x30,0x41,0x3a,0x45,0x30,0x3a,0x42,
0x45,0x7c,0x56,0x31,0x2e,0x31,0x34,0x5f,0x33,0x30,0x34,0x64,0x70,0x69,0x7c,0x41,
0x36,0x33,0x32,0x34,0x32,0x31,0x31,0x35,0x33,0x32,0x39,0x37,0x31,0x7c,0x37,0x31,
};

#endif

uint16_t tx_handle=0;
uint16_t rx_handle=0;
uint16_t flow_ctrl_handle=0;

#define IFLASH_NVRAM_ADDR 0x1000000+((512-4)*1024)
#define NVRAM_LEN 170//Nvram 长度170bytes,每组Nvram长度为34bytes,最多可存储5组配对信息

uint16_t ble_send_handle=0x2a;
uint8_t bt_cod[3] = {0x24,0x04,0x04};
uint8_t NvramData[NVRAM_LEN]={0};


//write flash
uint8_t   ReserveNV(uint8_t* nvram);
extern tick SystickCount;
#ifdef UART_TO_IPC
HCI_TypeDef HCI_Tx;
HCI_TypeDef HCI_Rx;
uint8_t ipc_tx_buf[256];
uint8_t ipc_rx_buf[256];
#endif

void BT_IRQHandler()
{
	while(IPC_have_data())
	{
		#ifdef UART_TO_IPC
		if(TRUE==IPC_ReadBtData(&HCI_Rx))
		{
			UART_SendBuf(IPC_UART,(uint8_t*)&HCI_Rx,3);
			UART_SendBuf(IPC_UART,HCI_Rx.p_data,HCI_Rx.DataLen);
		}
		#else
		BT_ParseBTData();
		#endif
	}

	BT_CONFIG &= (~(1<<BT_INIT_FLAG));
}

void SYSTICK_IRQHandler()
{
	SystickCount++;
	if(SystickCount>=TICK_MAX_VALUE)	SystickCount=0;
}



static uint8_t hex_lookup[16] = { '0', '1', '2', '3', '4', '5', '6', '7','8','9', 'a', 'b', 'c', 'd', 'e', 'f' };
static void array2hex(uint8_t* in, int inlen, uint8_t* out)
{
	int i;
	for (i = 0; i < inlen; ++i)
	{
		out[2 * i] = hex_lookup[in[inlen-1-i] >> 4];
		out[2 * i + 1] = hex_lookup[in[inlen-1-i] & 0x0f];
	}
}

uint8_t genBleAdvData(uint8_t *ble_mac,uint8_t *ble_adv_data)
{
    int index=0;
    //property
    ble_adv_data[index++]=0x02;
    ble_adv_data[index++]=0x01;
    ble_adv_data[index++]=0x02;

    //manufacturer info
    ble_adv_data[index++]=0x09;
    ble_adv_data[index++]=0xff;//manufacturer info flag
    ble_adv_data[index++]=0x00;
    ble_adv_data[index++]=0x00;
    for(int i=0;i<6;i++)
    {
        ble_adv_data[index++]=ble_mac[5-i];
    }
    
    //tx power level
    ble_adv_data[index++]=0x02;
    ble_adv_data[index++]=0x0a;
    ble_adv_data[index++]=0x04;
    
    //16-bit uuids
    ble_adv_data[index++]=0x03;
    ble_adv_data[index++]=0x02;
    ble_adv_data[index++]=0xe7;
    ble_adv_data[index++]=0xfe;
    
    ble_adv_data[index++]='\0';   
    
    return 20;  //返回长度
}

uint8_t genBleScan_RspData(uint8_t *ble_name,uint8_t name_len,uint8_t *ble_scanrsp_data)
{
    int index=0;
    uint8_t len=name_len;
    
    MyPrintf("len %d \r\n",len);
    ble_scanrsp_data[index++]=name_len+1;
    ble_scanrsp_data[index++]=0x09;
    
    for(int i=0;i<name_len;i++)
    {
        ble_scanrsp_data[index++]=ble_name[i];
    }
    
    ble_scanrsp_data[index++]='\0';   
    
    return name_len+2;  //返回长度
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    #ifdef DingDangTongXue
	uint8_t bt_name[20] = "FlashToy_AA1E_BLE";
	uint8_t bt_addr[6] =  {0x1E,0xAA,0x21,0x02,0x21,0x20};
    #endif
    #ifdef AiLiXiong
    uint8_t bt_name[20] = "PeriPage+E0BE_BLE";
	uint8_t bt_addr[6] =  {0xBE,0xE0,0x0A,0x8C,0x47,0xC8};
    #endif

    //uint8_t bt_name[20] = "YC3121_BTBLE";
	//uint8_t bt_addr[6] =  {0xBE,0xE0,0x0A,0x8C,0x47,0xC8};

    
	uint8_t chip_id[6]={0};
    uint8_t ble_adv_data[31]={0x02,0x01,0x02,0x00};
	uint16_t temp_handle;
    uint8_t len = 0;

	int UARTCMD = 0;
	uint8_t getbtstatus = 0;
	int i = 0;
	#ifdef UART_TO_IPC
	HCI_Tx.p_data=ipc_tx_buf;
	HCI_Rx.p_data=ipc_rx_buf;
	#endif
	UART_Configuration();
	MyPrintf("bt ble demo\n");
	SysTick_Config(CPU_MHZ/100);

	BT_Init();
	enable_intr(INTR_BT);

	read_chipid(chip_id);

	#ifndef UART_TO_IPC
	if(BT_SetBleName(bt_name,sizeof(bt_name)-1)==TRUE)
		MyPrintf("SetBleName_suc ble name:%s\n",bt_name);
	else
		MyPrintf("SetBleName_failed\n");

	if(BT_SetBtName(bt_name,sizeof(bt_name)-1) == TRUE)//bt与ble名字地址可以设置成一样
		MyPrintf("SetbtName_suc\n");
	else
		MyPrintf("SetbtName_fail\n");

	if(BT_SetBleAddr(bt_addr) == TRUE)
		MyPrintf("SetBleAddr_suc\n");
	else
		MyPrintf("SetBleAddr_fail\n");

	if(BT_SetBtAddr(bt_addr) == TRUE)
		MyPrintf("SetBtAddr_suc\n");
	else
		MyPrintf("SetBtAddr_fail\n");

	if(BT_SetParingMode(0x03) == TRUE)//设置配对模式为confirmkey
		MyPrintf("set confirmkey mode success\n");
	else
		MyPrintf("set confirmkey mode failed\n");

	if(BT_SetCOD(bt_cod) == TRUE) //设置COD
		MyPrintf("set COD sucess\n");
	else
		MyPrintf("set COD failed\n");

	if(BT_SetLpmMode(0x00) == TRUE) //设置低功耗模式
		MyPrintf("set lpm mode sucess\n");
	else
		MyPrintf("set lpm mode failed\n");


	if(BT_DeleteService() == TRUE) //  删除用户自定义服务
		MyPrintf("delete service sucess\n");
	else
		MyPrintf("delete service failed\n");

    //增加FF00服务
	temp_handle=BT_AddBleService(ble_service_uuid_lsps,sizeof(ble_service_uuid_lsps));
	if( temp_handle!= 0)  //增加服务 返回handle无需保存
		MyPrintf("add service sucess,handle=%04x\n",temp_handle);
	else
		MyPrintf("add service failed,return=%04x\n",temp_handle);

	tx_handle=BT_AddBleCharacteristic(ble_Characteristic_uuid_lsps_tx,sizeof(ble_Characteristic_uuid_lsps_tx));
	if( tx_handle!= 0)  //增加服务特征  write返回的handle需要保存，发数据使用
		MyPrintf("add Characteristic tx sucess,handle=%04x\n",tx_handle);
	else
		MyPrintf("add Characteristic tx failed,return=%04x\n",tx_handle);

	rx_handle=BT_AddBleCharacteristic(ble_Characteristic_uuid_lsps_rx,sizeof(ble_Characteristic_uuid_lsps_rx));
	if( rx_handle!= 0)
		MyPrintf("add Characteristic rx sucess;handle=%04x\n",rx_handle);
	else
		MyPrintf("add Characteristic rx failed,return=%04x\n",rx_handle);

	flow_ctrl_handle=BT_AddBleCharacteristic(ble_Characteristic_uuid_flow_ctrl,sizeof(ble_Characteristic_uuid_flow_ctrl));
	if( flow_ctrl_handle!= 0)
		MyPrintf("add Characteristic flow_ctrl sucess;handle=%04x\n",flow_ctrl_handle);
	else
		MyPrintf("add Characteristic flow_ctrl failed,return=%04x\n",flow_ctrl_handle);
    
    
    #ifdef AiLiXiong
    temp_handle=BT_AddBleService(ble_service_uuid_test,sizeof(ble_service_uuid_test));
	if( temp_handle!= 0)  //增加服务 返回handle无需保存
		MyPrintf("add service sucess,handle=%04x\n",temp_handle);
	else
		MyPrintf("add service failed,return=%04x\n",temp_handle);

	temp_handle=BT_AddBleCharacteristic(ble_Characteristic_uuid_test_tx,sizeof(ble_Characteristic_uuid_test_tx));
	if( temp_handle!= 0)  //增加服务特征  write返回的handle需要保存，发数据使用
		MyPrintf("add Characteristic tx sucess,handle=%04x\n",temp_handle);
	else
		MyPrintf("add Characteristic tx failed,return=%04x\n",temp_handle);

	temp_handle=BT_AddBleCharacteristic(ble_Characteristic_uuid_test_rx,sizeof(ble_Characteristic_uuid_test_rx));
	if( temp_handle!= 0)
		MyPrintf("add Characteristic rx sucess;handle=%04x\n",temp_handle);
	else
		MyPrintf("add Characteristic rx failed,return=%04x\n",temp_handle);
    #endif
    

    len = genBleAdvData(bt_addr,ble_adv_data);
    if(BT_SetLEAdvData(ble_adv_data,len) == TRUE)
		MyPrintf("BT_SetLEAdvData sucess\n");
	else
		MyPrintf("BT_SetLEAdvData failed\n");

    len = genBleScan_RspData(bt_name,sizeof(bt_name)-3,ble_adv_data);
    if(BT_SetLEScanData(ble_adv_data,len) == TRUE)
		MyPrintf("BT_SetLEScanData sucess\n");
	else
		MyPrintf("BT_SetLEScanData failed\n");

	if(BT_SetVisibility(0x01,0x01,0x01) == TRUE)  //设置可发现
		MyPrintf("SetVisibility sucess\n");
	else
		MyPrintf("SetVisibility failed\n");

	MyPrintf("bt version=%x\n",BT_GetVersion());

	qspi_flash_read(IFLASH_NVRAM_ADDR,NvramData,NVRAM_LEN);
	if(NvramData[34*0] > 0x05||NvramData[34*1] > 0x05||NvramData[34*2] > 0x05||NvramData[34*3] > 0x05||NvramData[34*4] > 0x05)
	{
		memset(NvramData,0x00,NVRAM_LEN);
	}
	i=0;
	if(BT_SetNVRAM(NvramData,NVRAM_LEN) == TRUE)
	{
		MyPrintf("set nvram success:\n");
		while(i<NVRAM_LEN)
			MyPrintf("0x%02X ",NvramData[i++]);
		MyPrintf("\n");
	}
	else
		MyPrintf("set nvram failed\n");
	#else
	MyPrintf("uart baud rate is:%d\n",UART_TO_IPC_BAUDRATE);
	#endif

	while (1)
	{
		//uart cmd to ipc
		#ifdef UART_TO_IPC
		if(UART_IsRXFIFONotEmpty(IPC_UART))
		{
			HCI_Tx.type = UART_ReceiveData(IPC_UART);
			while(!UART_IsRXFIFONotEmpty(IPC_UART));
			HCI_Tx.opcode = UART_ReceiveData(IPC_UART);
			while(!UART_IsRXFIFONotEmpty(IPC_UART));
			HCI_Tx.DataLen = UART_ReceiveData(IPC_UART);
			i=0;
			while(i<HCI_Tx.DataLen)
			{
				while(!UART_IsRXFIFONotEmpty(IPC_UART));
				HCI_Tx.p_data[i++] = UART_ReceiveData(IPC_UART);
			}
			IPC_TxData(&HCI_Tx);
		}
		#else
		BT_Progress();
		#endif
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

	/* Configure serial ports 0 RX and TX for IO. */
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
	#ifdef UART_TO_IPC
	UART_InitStruct.BaudRate = UART_TO_IPC_BAUDRATE;			//Configure serial port baud rate, the baud rate defaults to 128000.
	#else
	UART_InitStruct.BaudRate = UARTBAUD;
	#endif
	UART_InitStruct.DataBits = Databits_8b;
	UART_InitStruct.StopBits = StopBits_1;
	UART_InitStruct.Parity = Parity_None;
	UART_InitStruct.FlowCtrl = FlowCtrl_None;
	UART_InitStruct.Mode = Mode_duplex;

	UART_Init(UART0, &UART_InitStruct);
}


/**
  * @brief  deal with data from bt core event
  * @param  None
  * @retval None
  */


void BT_Progress()
{
	int eventCmd= BT_GetEventOpcode();
	switch(eventCmd)
	{
		case BT_DATA:
			{
				uint8_t i = 0;
				uint8_t btData[MAX_BLUETOOTH_DATA_LEN];
				int BtDataLen=0;
				BtDataLen=BT_ReadBTData(btData);
				if(BtDataLen>0)
				{
					MyPrintf("received %d bytes bt data:\n",BtDataLen);
					for(int i=0;i<BtDataLen;i++)	MyPrintf("%02x ",btData[i]);
					MyPrintf("\n");

					BT_SendSppData(btData,BtDataLen);
					MyPrintf("send %d bytes bt data:\n",BtDataLen);
					for(int i=0;i<BtDataLen;i++)	MyPrintf("%02x ",btData[i]);
					MyPrintf("\n");
					#if (MEM_SPP_FLOWCTRL_FLAG_VALUE==0x01)
					if(BT_SetSppFlowcontrol(1) != TRUE)	MyPrintf("set Flowcontrol failed\n");
					#endif
				}
				else	MyPrintf("read bt data error\r\n");
			}
		break;

		case BLE_DATA:
			{
				uint8_t bleData_rec[MAX_BLUETOOTH_DATA_LEN]={0};
                uint8_t bleData_send[MAX_BLUETOOTH_DATA_LEN]={0};
				int BleDataLen_rec=0;
                int BleDataLen_send=0;
				BleDataLen_rec=BT_ReadBTData(bleData_rec);
				if(BleDataLen_rec>0)
				{
                    if ((flow_ctrl_handle+1) == (bleData_rec[1]*0x100+bleData_rec[0]))
                    {
                        //send package num
                        //add handle
                        bleData_send[0]=(flow_ctrl_handle&0xff);
                        bleData_send[1]=((flow_ctrl_handle>>8)&0xff);
                        bleData_send[2]= 0x01;
                        bleData_send[3]= 0x04;
                        BleDataLen_send = 4;
                        BT_SendBleData(bleData_send,BleDataLen_send);
                        MyPrintf("send %d bytes ble data[%02x %02x]:\n",BleDataLen_send-2,bleData_send[0],bleData_send[1]);
                        for(int i=0;i<(BleDataLen_send-2);i++)	MyPrintf("%02x ",bleData_send[2+i]);
                        MyPrintf("\n");
                        
                        //send support mtu
                        bleData_send[2]= 0x02;
                        bleData_send[3]= 0x64;
                        bleData_send[4]= 0x00;
                        BleDataLen_send = 5;
                        BT_SendBleData(bleData_send,BleDataLen_send);
                        MyPrintf("send %d bytes ble data[%02x %02x]:\n",BleDataLen_send-2,bleData_send[0],bleData_send[1]);
                        for(int i=0;i<(BleDataLen_send-2);i++)	MyPrintf("%02x ",bleData_send[2+i]);
                        MyPrintf("\n");
                    }
                    
                    if (rx_handle == (bleData_rec[1]*0x100+bleData_rec[0]))
                    {
                        bleData_send[0]=(flow_ctrl_handle&0xff);
                        bleData_send[1]=((flow_ctrl_handle>>8)&0xff);
                        bleData_send[2]= 0x01;
                        bleData_send[3]= 0x01;
                        BleDataLen_send = 4;
                        BT_SendBleData(bleData_send,BleDataLen_send);
                        MyPrintf("send %d bytes ble data[%02x %02x]:\n",BleDataLen_send-2,bleData_send[0],bleData_send[1]);
                        for(int i=0;i<(BleDataLen_send-2);i++)	MyPrintf("%02x ",bleData_send[2+i]);
                        MyPrintf("\n");
                        
                        #ifdef DingDangTongXue
                        if (BleDataLen_rec == 5)
                        {
                            if ((bleData_rec[2]==0x10)&&(bleData_rec[3]==0xff)&&(bleData_rec[4]==0x40))
                            {
                                bleData_send[0]=(tx_handle&0xff);
                                bleData_send[1]=((tx_handle>>8)&0xff);
                                bleData_send[2]= 0x00;
                                BleDataLen_send = 3;
                                BT_SendBleData(bleData_send,BleDataLen_send);
                            }
                        }
                        
                        if (BleDataLen_rec == 6)
                        {
                            if ((bleData_rec[2]==0x10)&&(bleData_rec[3]==0xff)&&(bleData_rec[4]==0xff))
                            {
                                bleData_send[0]=(tx_handle&0xff);
                                bleData_send[1]=((tx_handle>>8)&0xff);
                                bleData_send[2]= (bleData_rec[5]&0x0f << 4) + (bleData_rec[5]>>4);
                                BleDataLen_send = 3;
                                BT_SendBleData(bleData_send,BleDataLen_send);
                            }
                            
                            //end flag
                            if ((bleData_rec[2]==0x10)&&(bleData_rec[3]==0xff)&&(bleData_rec[4]==0xf1)&&(bleData_rec[5]==0x45))
                            {
                                bleData_send[0]=(tx_handle&0xff);
                                bleData_send[1]=((tx_handle>>8)&0xff);
                                bleData_send[2]= 0xaa;
                                BleDataLen_send = 3;
                                BT_SendBleData(bleData_send,BleDataLen_send);
                            }
                        }
                        #endif
                        
                        #ifdef AiLiXiong
                        if (BleDataLen_rec == 5)
                        {
                            if ((bleData_rec[2]==0x10)&&(bleData_rec[3]==0xff)&&(bleData_rec[4]==0x70))
                            {
                                bleData_send[0]=(tx_handle&0xff);
                                bleData_send[1]=((tx_handle>>8)&0xff);
                                for (int i=0;i<80;i++)
                                {
                                    bleData_send[2+i] = Ai_Resp1[i];
                                }
                                BleDataLen_send = 82;
                                BT_SendBleData(bleData_send,BleDataLen_send);
                            }
                        }
                        
                        if (BleDataLen_rec == 6)
                        {
                            if ((bleData_rec[2]==0x10)&&(bleData_rec[3]==0xff)&&(bleData_rec[4]==0xff))
                            {
                                bleData_send[0]=(tx_handle&0xff);
                                bleData_send[1]=((tx_handle>>8)&0xff);
                                
                                int i= bleData_rec[5]>>4;
                                int j= bleData_rec[5]<<4;
                                bleData_send[2]= j+i;
                                BleDataLen_send = 3;
                                BT_SendBleData(bleData_send,BleDataLen_send);
                            }
                            
                            if ((bleData_rec[2]==0x10)&&(bleData_rec[3]==0xff)&&(bleData_rec[4]==0x20)&&(bleData_rec[5]==0xf0))
                            {
                                bleData_send[0]=(tx_handle&0xff);
                                bleData_send[1]=((tx_handle>>8)&0xff);
                                bleData_send[2]= 0x49;
                                bleData_send[3]= 0x50;
                                bleData_send[4]= 0x2d;
                                bleData_send[5]= 0x33;
                                bleData_send[6]= 0x30;
                                bleData_send[7]= 0x30;
                                BleDataLen_send = 8;
                                BT_SendBleData(bleData_send,BleDataLen_send);
                            }

                            //end flag
                            if ((bleData_rec[2]==0x10)&&(bleData_rec[3]==0xff)&&(bleData_rec[4]==0xfe)&&(bleData_rec[5]==0x45))
                            {
                                bleData_send[0]=(tx_handle&0xff);
                                bleData_send[1]=((tx_handle>>8)&0xff);
                                bleData_send[2]= 0xaa;
                                BleDataLen_send = 3;
                                BT_SendBleData(bleData_send,BleDataLen_send);
                            }
                        }
                        
                        if (BleDataLen_rec == 7)
                        {
                            if ((bleData_rec[2]==0x10)&&(bleData_rec[3]==0xff)&&(bleData_rec[4]==0x10)&&(bleData_rec[5]==0x00)&&(bleData_rec[6]==0x00))
                            {
                                bleData_send[0]=(tx_handle&0xff);
                                bleData_send[1]=((tx_handle>>8)&0xff);
                                bleData_send[2]= 0x4f;
                                bleData_send[3]= 0x4b;
                                BleDataLen_send = 4;
                                BT_SendBleData(bleData_send,BleDataLen_send);
                            }
                        }
                        #endif
                        
                    }
                    
					MyPrintf("received %d bytes ble data[handle:%02x %02x]:\n",BleDataLen_rec-2,bleData_rec[0],bleData_rec[1]);
					for(int i=0;i<(BleDataLen_rec-2);i++)	MyPrintf("%02x ",bleData_rec[2+i]);
					MyPrintf("\n");
				}
				else	MyPrintf("read ble data error\r\n");
			}
		break;

		case BT_CONNECTED:
			MyPrintf("bt connected\n");
		break;

		case BLE_CONNECTED:
			MyPrintf("ble connected\n");
		break;

		case BT_DISCONNECTED:
			MyPrintf("bt disconnected\n");
		break;

		case BLE_DISCONNECTED:
			MyPrintf("ble disconnected\n");
		break;

		case NVRAM_DATA:
			{
				uint8_t NvramData[MAX_BLUETOOTH_DATA_LEN];
				int NvramDataLen=0;
				NvramDataLen=BT_ReadBTData(NvramData);
				if(NvramDataLen>0)
				{
					MyPrintf("receive %dbytes Nvramdata:",NvramDataLen);
					for(int i=0;i<NvramDataLen;i++)
					{
						MyPrintf("%02X ",NvramData[i]);
					}
					MyPrintf("\n");
					ReserveNV(NvramData);//将最新的5个设备信息（NVRAM） 写入flash ,
				}
				else
					MyPrintf("read Nvram data error\r\n");
			}
		break;

		case CONFIRM_GKEY:
			{
				uint8_t GkeyData[MAX_BLUETOOTH_DATA_LEN];
				int GkeyDataLen=0;
				GkeyDataLen=BT_ReadBTData(GkeyData);
				if(GkeyDataLen>0)
				{
					MyPrintf("receive %dbytes CONFIRM_GKEY:",GkeyDataLen);
					for(int i=0;i<GkeyDataLen;i++)
					{
						MyPrintf("%02X ",GkeyData[i]);
					}
					MyPrintf("\n");
				}
				else	MyPrintf("read CONFIRM_GKEY data error\r\n");

				if(BT_ConfirmGkey(0) == TRUE)//BT_ConfirmGkey参数为0则继续配对，1则取消配对。
					MyPrintf("set confirmkey  success");
				else
					MyPrintf("set confirmkey  failed\n");
		}
		break;

		case PASSKEY:
			MyPrintf("request Passkey");
		break;

		case PAIRING_COMPLETED:
			{
				uint8_t PairingData[MAX_BLUETOOTH_DATA_LEN];
				int PairingDataLen=0;
				PairingDataLen=BT_ReadBTData(PairingData);
				if(PairingDataLen>0)
				{
					MyPrintf("receive %dbytes paring data:",PairingDataLen);
					for(int i=0;i<PairingDataLen;i++)
					{
						MyPrintf("%02X ",PairingData[i]);
					}
					MyPrintf("\n");
				}
				else	MyPrintf("read PAIRING_COMPLETED data error\r\n");
			}
		break;

		case PAIRING_STATE:
			{
				uint8_t PairingData[MAX_BLUETOOTH_DATA_LEN];
				int PairingDataLen=0;
				uint16_t pairing_status=0;
				PairingDataLen=BT_ReadBTData(PairingData);
				if(PairingDataLen==2)
				{
					pairing_status |=PairingData[0];
					pairing_status |=(PairingData[1]<<8);
					switch(pairing_status)
					{
						case 0x0001:
							MyPrintf("BT Pairing Success\r\n");
						break;

						case 0x0101:
							MyPrintf("BT Pairing fail\r\n");
						break;

						case 0x0080:
							MyPrintf("BLE Pairing Success\r\n");
						break;

						case 0x0180:
							MyPrintf("BLE Pairing fail\r\n");
						break;

						default:
							MyPrintf("unknown pairing_status[%04x]\r\n",pairing_status);
						break;
					}
				}
				else	MyPrintf("read PAIRING_STATE data error\r\n");
			}
		break;

		case -1:
			//no event
		break;

		default:
		{
			MyPrintf("unknown event:%02x\r\n",eventCmd);
			uint8_t unknownData[MAX_BLUETOOTH_DATA_LEN];
			int unknownDataLen=0;
			unknownDataLen=BT_ReadBTData(unknownData);
			if(unknownDataLen>0)
			{
				MyPrintf("datalen:%d\r\n",unknownDataLen);
				for(int i=0;i<unknownDataLen;i++)
				{
					MyPrintf("%02X ",unknownData[i]);
				}
				MyPrintf("\n");
			}
		}
		break;
	}
}

uint8_t   ReserveNV(uint8_t* nvram)
{
	MyPrintf("new nvram data,updata to flash\r\n");
	qspi_flash_sectorerase(IFLASH_NVRAM_ADDR);
	return qspi_flash_write(IFLASH_NVRAM_ADDR,nvram,NVRAM_LEN);
}

/************************ (C) COPYRIGHT Yichip Microelectronics *****END OF FILE****/

