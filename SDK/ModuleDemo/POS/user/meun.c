#include "meun.h"

/* Private define ------------------------------------------------------------*/


void BEEP_Start()
{
#if BEEP_PWM
    TIM_Cmd(TIM8, ENABLE);
#else
    GPIO_Config(BEEP_PORT, BEEP_PIN, OUTPUT_HIGH);
#endif
}
void BEEP_End()
{
#if BEEP_PWM
    TIM_Cmd(TIM8, DISABLE);
#else
    GPIO_Config(BEEP_PORT, BEEP_PIN, OUTPUT_LOW);
#endif

}

#if BEEP_PWM
void BEEP_PWM_Init(void)
{
    uint32_t value;

    PWM_InitTypeDef PWM_init_struct;
    PWM_init_struct.TIMx = TIM8;
    PWM_init_struct.LowLevelPeriod = 6400;
    PWM_init_struct.HighLevelPeriod = 6400;
    PWM_init_struct.SatrtLevel = OutputLow;
    GPIO_Config(BEEP_PORT, BEEP_PIN, PWM_OUT8);
    TIM_PWMInit(&PWM_init_struct);
    TIM_ModeConfig(TIM8, TIM_Mode_PWM);
}
#endif


static uint8_t hex_lookup[16] = { '0', '1', '2', '3', '4', '5', '6', '7','8','9', 'a', 'b', 'c', 'd', 'e', 'f' };
static void array2hex(uint8_t* in, int inlen, uint8_t* out)
{
	int i;
	for (i = 0; i < inlen; ++i)
	{
		out[2 * i] = hex_lookup[in[i] >> 4];
		out[2 * i + 1] = hex_lookup[in[i] & 0x0f];
	}
}

#define	MAX_INT_STR_SIZE	10
static int int2str(uint32_t value, uint8_t* out)
{
	int i,j;
	Boolean start=FALSE;
	uint32_t base_num=1000000000;
	i=0;
	while (base_num>0)
	{
		j=value/base_num;
		if(j>0||start==TRUE||base_num==1)
		{
			start=TRUE;
			out[i++]=hex_lookup[j];
		}
		value=value%base_num;
		base_num=base_num/10;
	}
	return i;//str len
}

#if (VERSIONS == EXIST_BT)
#define BNEP_STATUS_IDLE					0x00
#define BNEP_STATUS_CONNECTING		0x01
#define BNEP_STATUS_CONNECTED			0x10
#define BNEP_STATUS_DOWNLOADING		0x11
uint8_t bnep_status=BNEP_STATUS_IDLE;

#define APP_TOTAL_DOWNLOAD_LEN		0x80000	//512k
#define APP_SUB_LEN								1024	//subpackage len 1k
uint32_t app_total_received_len=0;
uint32_t app_received_len=0;
uint16_t app_crc16=0xffff;
Boolean load_data_error=FALSE;
#define BNEP_APP_LOAD_TIMEOUT			10000	//10s
tick bnep_last_tick=0;
tick download_progress_tick=0;
Boolean bt_progress_idle=FALSE;
uint8_t download_cmd[]={0x00,0xAA,0x55,0x01,0x00,0x04,0x00,0x00,0x82,0x8B};


// byte0： uuid length
//byte1-16: uuid
// 49535343-FE78-4AE5-8FA9-9FAFD205E455
uint8_t ble_service_uuid_lsps[] = {0x10,0x55,0xe4,0x05,0xd2,0xaf,0x9f,0xa9,0x8f,0xe5,0x4a,0x7d,0xfe,0x43,0x53,0x53,0x49};

/*
byte0: characterisitic  
byte1:characterisitic uuid length  
byte2-17:characterisitic uuid 
byte18:write/read payload length 
byte19-20:write/read payload
*/
//49535343-1E4D-4BD9-BA61-23C647249616
uint8_t ble_Characteristic_uuid_lsps_tx[] = {0x10,0x10,0x16,0x96,0x24,0x47,0xc6,0x23,0x61,0xba,0xd9,0x4b,0x4d,0x1e,0x43,0x53,0x53,0x49,0x01,0x00};
//49535343-8841-43F4-A8D4-ECBE34729BB3
uint8_t ble_Characteristic_uuid_lsps_rx[] = {0x0c,0x10,0xb3,0x9b,0x72,0x34,0xbe,0xec,0xd4,0xa8,0xf4,0x43,0x41,0x88,0x43,0x53,0x53,0x49,0x01,0x00};

#define IFLASH_NVRAM_ADDR 0x1000000+((1024-4)*1024)
#define NVRAM_LEN 170//Nvram 长度170bytes,每个设备信息34个字节.
//#define SPP_FLOWCONTROL
uint16_t ble_send_handle=0x002a;
uint8_t bt_name[15] = "YC3121bt";
uint8_t bt_cod[3] = {0x24,0x04,0x04};
uint8_t NvramData[NVRAM_LEN]={0};
Boolean btStatus=FALSE;
Boolean btInitSucc=TRUE;
void app_bt_init(void)
{
	uint8_t bt_addr[6] =  {0x00,0x00,0x00,0x33,0x22,0x10};
	uint8_t chip_id[6]={0};
	uint16_t temp_handle;

	BT_Init();
	enable_intr(INTR_BT);
	
	read_chipid(chip_id);
	array2hex(chip_id,3,bt_name+8);
	bt_name[14]='\0';
	memcpy(bt_addr,chip_id,3);

	if(BT_SetBleName(bt_name,sizeof(bt_name)-1)==TRUE) 
		MyPrintf("SetBleName_suc ble name:%s\n",bt_name);
	else
    {
        MyPrintf("SetBleName_failed\n");
        btInitSucc=FALSE;
    }
	
	if(BT_SetBtName(bt_name,sizeof(bt_name)-1) == TRUE)//bt与ble名字地址可以设置成一样
		MyPrintf("SetbtName_suc\n");
	else
    {
		MyPrintf("SetbtName_fail\n");
        btInitSucc=FALSE;
    }
	
	if(BT_SetBleAddr(bt_addr) == TRUE) 
		MyPrintf("SetBleAddr_suc\n");
	else
    {
		MyPrintf("SetBleAddr_fail\n"); 
        btInitSucc=FALSE;
    }
	
	if(BT_SetBtAddr(bt_addr) == TRUE)
		MyPrintf("SetBtAddr_suc\n");
	else
    {
		MyPrintf("SetBtAddr_fail\n");
        btInitSucc=FALSE;
    } 
	
	if(BT_SetParingMode(0x01) == TRUE)
		MyPrintf("set confirmkey mode success\n");
	else
    {
		MyPrintf("set confirmkey mode failed\n");
        btInitSucc=FALSE;
    } 
	
	if(BT_SetCOD(bt_cod) == TRUE) //设置bt3.0设备类型
		MyPrintf("set COD sucess\n");
	else
    {
		MyPrintf("set COD failed\n");
        btInitSucc=FALSE;
    } 

	
	if(BT_DeleteService() == TRUE) //  删除用户自定义服务
		MyPrintf("delete service sucess\n");
	else
    {
		MyPrintf("delete service failed\n");
        btInitSucc=FALSE;
    } 

	temp_handle=BT_AddBleService(ble_service_uuid_lsps,sizeof(ble_service_uuid_lsps));
	if( temp_handle!= 0)  //增加服务 返回handle无需保存
		MyPrintf("add service sucess,handle=%04x\n",temp_handle);
	else
    {
		MyPrintf("add service failed,return=%04x\n",temp_handle);
        btInitSucc=FALSE;
    } 
	
	ble_send_handle=BT_AddBleCharacteristic(ble_Characteristic_uuid_lsps_tx,sizeof(ble_Characteristic_uuid_lsps_tx));
	if( ble_send_handle!= 0)  //增加服务的TX特征,返回的handle需要保存，发ble数据时使用
		MyPrintf("add Characteristic tx sucess,handle=%04x\n",ble_send_handle);
	else
    {
		MyPrintf("add Characteristic tx failed,return=%04x\n",ble_send_handle);
        btInitSucc=FALSE;
    } 
	
	temp_handle=BT_AddBleCharacteristic(ble_Characteristic_uuid_lsps_rx,sizeof(ble_Characteristic_uuid_lsps_rx));
	if( temp_handle!= 0)  //增加服务的RX特征,返回的handle不需保存
		MyPrintf("add Characteristic rx sucess;handle=%04x\n",temp_handle);
	else
    {
		MyPrintf("add Characteristic rx failed,return=%04x\n",temp_handle);
        btInitSucc=FALSE;
    } 


	if(BT_SetVisibility(0x01,0x01,0x01) == TRUE)  //设置可发现
		MyPrintf("SetVisibility sucess\n");
	else
    {
		MyPrintf("SetVisibility failed\n");
        btInitSucc=FALSE;
    } 
	
	MyPrintf("bt version=%x\n",BT_GetVersion());
	
	qspi_flash_read(IFLASH_NVRAM_ADDR,NvramData,NVRAM_LEN);
	if(NvramData[34*0] > 0x05||NvramData[34*1] > 0x05||NvramData[34*2] > 0x05||NvramData[34*3] > 0x05||NvramData[34*4] > 0x05)
	{
		memset(NvramData,0x00,NVRAM_LEN);
	}
	int i=0;
	if(BT_SetNVRAM(NvramData,NVRAM_LEN) == TRUE)
	{
		MyPrintf("set nvram success:\n");
		while(i<NVRAM_LEN)
			MyPrintf("0x%02X ",NvramData[i++]);
		MyPrintf("\n");
	}
	else
    {
		MyPrintf("set nvram failed\n");
        btInitSucc=FALSE;
    } 
	
    if(btInitSucc==TRUE)
    {
	    MyPrintf("bt init finish\n");
    }
    else
    {
        memcpy(bt_name,"bt init fail",12);
        bt_name[12]=0;
        MyPrintf("bt init fail\n");
    }
    
}

void BT_IRQHandler()
{
	while(IPC_have_data())
	{
		BT_ParseBTData();
	}

	BT_CONFIG &= (~(1<<BT_INIT_FLAG));
}

static uint16_t CRC16(uint8_t *data, int len,uint16_t crc16_base)
{
	int i;

	if(len<=0) return 0xffff;

	for (i = 0; i < len; i++)
	{
		crc16_base  = (crc16_base >> 8) | (crc16_base << 8);
		crc16_base ^= (uint16_t)(data[i] & 0xff);
		crc16_base ^= (crc16_base & 0xff) >> 4;
		crc16_base ^= crc16_base << 12;
		crc16_base ^= (crc16_base & 0xff) << 5;
		crc16_base &= 0xffff;
	}
	return crc16_base;
}



#define NAME_OFFSET_INDEX         0
#define NAME_LEN_INDEX            1
#define NAME_START_INDEX          2
#define NAME_MAC_INDEX            16
#define NAME_MAC_LEN              6
#define NAME_LINKKEY_INDEX        22
#define NAME_LINKKEY_LEN          16
#define MAX_SUBPACKAGE_NAME_LEN		14
#define MAX_BT_MASTER_NAME_LEN		(MAX_SUBPACKAGE_NAME_LEN*4+1)
uint8_t bt_master_name[MAX_BT_MASTER_NAME_LEN];
/**
  * @brief  deal with data from bt core event 
  * @param  None
  * @retval None
  */
void BT_Progress(void)
{	
	int eventCmd= BT_GetEventOpcode();
	switch(eventCmd)
	{	
		case DNS_RSP:
		{
			uint8_t recive_dns_rsp[MAX_BLUETOOTH_DATA_LEN];
			uint8_t tcp_addr[7];
			tcp_addr[0]=0x01;//tcp handle
			int DnsDataLen=0;
			DnsDataLen=BT_ReadBTData(recive_dns_rsp);
			if(DnsDataLen>0)
			{
				MyPrintf("DNS IP:");
				for(int i=0;i<DnsDataLen;i++)
				{
					MyPrintf("%d.",recive_dns_rsp[i]);
					if(i<4)	tcp_addr[1+i]=recive_dns_rsp[i];
				}
				MyPrintf("\n");
				//add port 443(0x1bb)
				tcp_addr[5]=0x01;
				tcp_addr[6]=0xbb;
				MyPrintf("start connect:www.baidu.com\n");
				if(BT_ConnectTcp(tcp_addr,sizeof(tcp_addr))==TRUE)
				{
					MyPrintf("Connect Tcp cmd ok\n");		
				}
				else 
				{
					MyPrintf("Connect Tcp cmd fail\n");
				}
			}
			else
			{
				MyPrintf("read dns_rsp error");
			}
		}
		break;
		
		case BNEP_RECEIVE_TCP_DATA:
			{
				bnep_last_tick=SysTick_GetTick();
				uint8_t i = 0;
				uint8_t bnep_recive_tcp_data[MAX_BLUETOOTH_DATA_LEN];
				int BnepDataLen=0;
				BnepDataLen=BT_ReadBTData(bnep_recive_tcp_data);
				if(BnepDataLen>0)
				{
					MyPrintf("recieve_tcp_data[%d]:\n",BnepDataLen-1);
					MyPrintf("tcp_handle:%02x\n",bnep_recive_tcp_data[0]);
					for(int i=1;i<BnepDataLen;i++)	MyPrintf("%02x ",bnep_recive_tcp_data[i]);
					MyPrintf("\n");	
					
					//download app
					app_received_len+=BnepDataLen-1;
					app_total_received_len+=BnepDataLen-1;
					app_crc16=CRC16((uint8_t*)&bnep_recive_tcp_data[1],BnepDataLen-1,app_crc16);
					MyPrintf("app_received_len=%d\r\n",app_total_received_len);
					if(app_received_len>=APP_SUB_LEN)
					{
						if(app_crc16!=0)
						{
							load_data_error=TRUE;
							MyPrintf("app download fail[app_crc16=%04x]\r\n",app_crc16);
						}
						app_crc16=0xffff;
						app_received_len=0;
						if(app_total_received_len>=APP_TOTAL_DOWNLOAD_LEN)
						{
							bnep_status=BNEP_STATUS_CONNECTED;
							if(load_data_error!=TRUE)
							{
								MyPrintf("app download ok\r\n");
								ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
								ST7789_TFT_ShowString(0, 152, 16*15, 16, "app download ok", 32, 1, WHITE, OLIVE);
							}
							else
							{
								MyPrintf("app download fail\r\n");
								ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
								ST7789_TFT_ShowString(0, 152, 16*17, 16, "app download fail", 32, 1, WHITE, OLIVE);
							}
						}
						else
						{
							if(BT_BnepSendTcpData( download_cmd,sizeof(download_cmd))!=TRUE)
							{
								MyPrintf("download cmd fail\n"); 
								ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
								ST7789_TFT_ShowString(0, 152, 16*17, 16, "download cmd fail", 32, 1, WHITE, OLIVE);
								BT_disconnectBnep();
								bnep_status=BNEP_STATUS_IDLE;
							}
						}
					}
					
				}
				else	MyPrintf("read bnep_recieve_tcp_data error");
			}
		break;
			
			case UDP_INFO:
			{
				uint8_t bnep_recive_udp_info[MAX_BLUETOOTH_DATA_LEN];
				if(BT_ReadBTData(bnep_recive_udp_info)>0)
				{
					MyPrintf("remote IP:");
					for(int i=0;i<4;i++)	MyPrintf("%d.",bnep_recive_udp_info[i]);
					MyPrintf(" port:%d\n",((bnep_recive_udp_info[8]<<8)|bnep_recive_udp_info[9]));	
					MyPrintf("local IP:");
					for(int i=0;i<4;i++)	MyPrintf("%d.",bnep_recive_udp_info[4+i]);
					MyPrintf(" port:%d\n",((bnep_recive_udp_info[10]<<8)|bnep_recive_udp_info[11]));	
				}
				else	MyPrintf("read udp info error");
			}
		break;
			
			case BNEP_RECEIVE_UDP_DATA:
			{
				uint8_t bnep_recive_udp_data[MAX_BLUETOOTH_DATA_LEN];
				int BnepDataLen=0;
				BnepDataLen=BT_ReadBTData(bnep_recive_udp_data);
				if(BnepDataLen>0)
				{
					MyPrintf("recive_udp_data[%d]:\n",BnepDataLen-1);
					for(int i=1;i<BnepDataLen;i++)	MyPrintf("%02x ",bnep_recive_udp_data[i]);
					MyPrintf("\n");	
				}
				else	MyPrintf("read bnep_recive_udp_data error");
			}
		break;
			
			case BNEP_CONNECT_FAIL:
			{
				MyPrintf("BNEP_CONNECT_FAIL\n");
				uint8_t i = 0;
				uint8_t bnep_connect_fail_reason[MAX_BLUETOOTH_DATA_LEN];
				int Datalen=0;
				Datalen=BT_ReadBTData(bnep_connect_fail_reason);
				if(Datalen>0)
				{
					switch(bnep_connect_fail_reason[0])
					{
						case 0:
						{
						   MyPrintf("BNEP_CONNECT_TIMEOUT\n");
						}
						break;
						case 1:
						{
							 MyPrintf("MOBILE_PHONE_NOT_SUPPORTED_BNEP\n");
						}
						break;
						case 2:
						{
							 MyPrintf("LINKKLY_ERROR \n");
						}
						break;
						case 3:
						{
							 MyPrintf("ROLE_SWITCH_FAIL \n");
						}
						break;
						
						case 4:
						{
							 MyPrintf("L2CAP Connect Fail \n");
						}
						break;
						
						case 5:
						{
							 MyPrintf("DHCP timeout \n");
						}
						break;
						
						case 6:
						{
							 MyPrintf("unpaired \n");
						}
						break;
						
						case 8:
						{
							 MyPrintf("baseband is connected \n");
						}
						break;
						
						default:
							MyPrintf("unknow  BNEP_CONNECT_FAIL code:%d\n",bnep_connect_fail_reason[0]);
							break;	
					}
					
				}
				if(bnep_status!=BNEP_STATUS_IDLE)
				{
					bnep_status=BNEP_STATUS_IDLE;
					ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
					ST7789_TFT_ShowString(0, 152, 16*19, 16, "connect server fail", 32, 1, WHITE, OLIVE);
					BT_disconnectBnep();
				}
			}
		break;
			case TCP_FAIL:
			{
				MyPrintf("TCP_FAIL\n");
				uint8_t i = 0;
				uint8_t tcp_connect_fail_reason[MAX_BLUETOOTH_DATA_LEN];
				int Datalen=0;
				Datalen=BT_ReadBTData(tcp_connect_fail_reason);
				if(Datalen>0)
				{
					switch(tcp_connect_fail_reason[0])
					{
						case 0:
						{
						   MyPrintf("TCP_CONNECT_TIMEOUT\n");
						}
						break;
						case 1:
						{
							 MyPrintf("DNS_NO_ACK\n");
						}
						break;
						case 2:
						{
							 MyPrintf("IP_ERROR \n");
						}
						break;
						case 3:
						{
							 MyPrintf("DISCONNECT_TIMEOUT \n");
						}
						break;
						
						default:
							MyPrintf("unknow  TCP_FAIL code:%d\n",tcp_connect_fail_reason[0]);
							break;	
					}
					
				}
				if(bnep_status!=BNEP_STATUS_IDLE)
				{
					bnep_status=BNEP_STATUS_IDLE;
					ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
					ST7789_TFT_ShowString(0, 152, 16*19, 16, "connect server fail", 32, 1, WHITE, OLIVE);
					BT_disconnectBnep();
				}
			}
		break;
			
		case TCP_CONNECT_LOG:
			{
				bnep_last_tick=SysTick_GetTick();
				uint8_t i = 0;
				uint8_t tcp_connect_Log[MAX_BLUETOOTH_DATA_LEN];
				int Datalen=0;
				Datalen=BT_ReadBTData(tcp_connect_Log);
				if(Datalen>0)
				{
					switch(tcp_connect_Log[0])
					{
						case 1:
						{
						   MyPrintf("RECIEVE_DHCP_OFFER\n");
						}
						break;
						case 3:
						{
							 MyPrintf("RECIEVE_DHCP_ACK\n");
							 uint8_t tcp_ip_addr[7]={0x00,0x8b,0xe0,0x38,0x57,0x30,0x3a};//ip:139.224.56.87 port:12346
							 MyPrintf("start connect:ip:139.224.56.87 port:12346\r\n");
							 if(BT_ConnectTcp(tcp_ip_addr,sizeof(tcp_ip_addr))==TRUE)
							 {
									MyPrintf("Connect Tcp cmd ok\n");		
							 }
							 else 
							 {
									MyPrintf("Connect Tcp cmd fail\n");
									ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
									ST7789_TFT_ShowString(0, 152, 16*19, 16, "Connect server fail", 32, 1, WHITE, OLIVE);
									bnep_status=BNEP_STATUS_IDLE;
									BT_disconnectBnep();
							 }
						}
						break;
						case 5:
						{
							 MyPrintf("RECIEVE_ARP_RESPONSE \n");
						}
						break;
						case 7:
						{
							 MyPrintf("RECIEVE_DNS_RESPONSE \n");
						}
						break;
						case 9:
						{
							 MyPrintf("RECIEVE_TCP_SYN_ACK \n");
						}
						break;
						default:
							break;	
					}
					
				}
			}
		break;
		
		
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
					#ifdef SPP_FLOWCONTROL
					if(BT_SetSppFlowcontrol(1) != TRUE)	MyPrintf("set Flowcontrol failed\n");
					#endif
				}
				else	MyPrintf("read bt data error\r\n");
			}
		break;
			
		case BLE_DATA:
			{
				uint8_t bleData[MAX_BLUETOOTH_DATA_LEN]={0};
				int BleDataLen=0;
				BleDataLen=BT_ReadBTData(bleData+2);
				if(BleDataLen>0)
				{
					MyPrintf("received %d bytes ble data:\n",BleDataLen);
					for(int i=0;i<BleDataLen;i++)	MyPrintf("%02x ",bleData[2+i]);
					MyPrintf("\n");
					
					//add handle
					bleData[0]=(ble_send_handle&0xff);
					bleData[1]=((ble_send_handle>>8)&0xff);
					BT_SendBleData(bleData,BleDataLen+2);
					MyPrintf("send %d bytes ble data[%02x %02x]:\n",BleDataLen,bleData[0],bleData[1]);
					for(int i=0;i<BleDataLen;i++)	MyPrintf("%02x ",bleData[2+i]);
					MyPrintf("\n");
				}
				else	MyPrintf("read ble data error\r\n");
			}
		break;

		case BT_CONNECTED:
			MyPrintf("bt connected\n");
			show_bt_status(TRUE);
		break;
			
		case BLE_CONNECTED:
			MyPrintf("ble connected\n");
			show_bt_status(TRUE);
		break;
			
		case BT_DISCONNECTED:
			MyPrintf("bt disconnected\n");
			show_bt_status(FALSE);
		break;
			
		case BLE_DISCONNECTED:
			MyPrintf("ble disconnected\n");
			show_bt_status(FALSE);
		break;
		
		case BNEP_CONNECT:
			bnep_last_tick=SysTick_GetTick();
			MyPrintf("bnep connected\n");
			show_bt_status(TRUE);
		break;
		
		case BB_DISCONNECT:
			MyPrintf("bt baseband disconnected\n");
			show_bt_status(FALSE);
			if(bnep_status!=BNEP_STATUS_IDLE)
			{
				bnep_status=BNEP_STATUS_IDLE;
				ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
				ST7789_TFT_ShowString(0, 152, 16*17, 16, "bnep disconnected", 32, 1, WHITE, OLIVE);
			}
		break;
		
		case TCP_CONNECT:
			{
				bnep_last_tick=SysTick_GetTick();
				MyPrintf("tcp connected\n");
				uint8_t connect_handle[MAX_BLUETOOTH_DATA_LEN];
				int connectHandleLen=0;
				connectHandleLen=BT_ReadBTData(connect_handle);
				if(connectHandleLen>0)
				{
					MyPrintf("handle:%02x\n",connect_handle[0]);
					bnep_status=BNEP_STATUS_CONNECTED;
					ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
					ST7789_TFT_ShowString(0, 152, 16*16, 16, "connected server", 32, 1, WHITE, OLIVE);
				}
				else	
				{
					MyPrintf("read connect_handle error\r\n");
				}
			}
		break;
		
		case TCP_DISCONNECT:
			{
				MyPrintf("tcp disconnected\n");
				uint8_t disconnect_handle[MAX_BLUETOOTH_DATA_LEN];
				int disconnectHandleLen=0;
				disconnectHandleLen=BT_ReadBTData(NvramData);
				if(disconnectHandleLen>0)
				{
					MyPrintf("handle:%02x\n",disconnect_handle[0]);
					BT_disconnectBnep();
				}
				else	
				{
					MyPrintf("read disconnect_handle error\r\n");
				}
				if(bnep_status!=BNEP_STATUS_IDLE)
				{
					bnep_status=BNEP_STATUS_IDLE;
					ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
					ST7789_TFT_ShowString(0, 152, 16*19, 16, "server disconnected", 32, 1, WHITE, OLIVE);
					BT_disconnectBnep();
				}
			}
		break;
		
		case BNEP_DISCONNECT:
			if(bnep_status!=BNEP_STATUS_IDLE)
			{
				bnep_status=BNEP_STATUS_IDLE;
				ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
				ST7789_TFT_ShowString(0, 152, 16*19, 16, "server disconnected", 32, 1, WHITE, OLIVE);
			}
			MyPrintf("bnep disconnected\n");
			show_bt_status(FALSE);
		break;
		
		case BNEP_CHIP_ERROR:
			MyPrintf("bnep chip error\n");
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
					MyPrintf("update new nvram to flash\r\n");
					qspi_flash_sectorerase(IFLASH_NVRAM_ADDR);
					qspi_flash_write(IFLASH_NVRAM_ADDR,NvramData,NVRAM_LEN);
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
			MyPrintf("request Passkey\r\n");
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
			
		case BT_NAME:
			{
				uint8_t btName[MAX_BLUETOOTH_DATA_LEN];
				int btNameLen=0;
				btNameLen=BT_ReadBTData(btName);
				if(btNameLen>0)
				{
					//bt master name join
					if((btName[NAME_OFFSET_INDEX]+MAX_SUBPACKAGE_NAME_LEN)<MAX_BT_MASTER_NAME_LEN)
					{
						memcpy(bt_master_name+btName[NAME_OFFSET_INDEX],btName+NAME_START_INDEX,MAX_SUBPACKAGE_NAME_LEN);
						if((btName[NAME_OFFSET_INDEX]+MAX_SUBPACKAGE_NAME_LEN)>=btName[NAME_LEN_INDEX])
						{
							bt_master_name[btName[NAME_LEN_INDEX]]=0;
							MyPrintf("receive bt master name:%s\n",bt_master_name);
							MyPrintf("mac:");
							for(int i=0;i<NAME_MAC_LEN;i++)	MyPrintf("%02x ",btName[NAME_MAC_INDEX+i]);
							MyPrintf("\r\n");
							MyPrintf("link key:");
							for(int i=0;i<NAME_LINKKEY_LEN;i++)	MyPrintf("%02x ",btName[NAME_LINKKEY_INDEX+i]);
							MyPrintf("\r\n");
						}
					}
					else
					{
						bt_master_name[MAX_BT_MASTER_NAME_LEN-1]=0;
						MyPrintf("bt master name len[%d]>MAX_BT_MASTER_NAME_LEN[%d]\r\n",btName[NAME_LEN_INDEX],MAX_BT_MASTER_NAME_LEN-1);
						MyPrintf("receive bt master name:%s...\n",bt_master_name);
						MyPrintf("mac:");
						for(int i=0;i<NAME_MAC_LEN;i++)	MyPrintf("%02x ",btName[NAME_MAC_INDEX+i]);
						MyPrintf("\r\n");
						MyPrintf("link key:");
						for(int i=0;i<NAME_LINKKEY_LEN;i++)	MyPrintf("%02x ",btName[NAME_LINKKEY_INDEX+i]);
						MyPrintf("\r\n");
					}
				}
				else	MyPrintf("read bt master name error\r\n");
			}
		break;
			
		case -1:
			bt_progress_idle=TRUE;//no event
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
	//load app timeout check
	if(bnep_last_tick!=0)
	{
		if(SysTick_IsTimeOut(bnep_last_tick,BNEP_APP_LOAD_TIMEOUT))
		{
			if(bnep_status==BNEP_STATUS_CONNECTING)
			{
				bnep_status=BNEP_STATUS_IDLE;
				MyPrintf("connect server timeout\r\n");
				ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
				ST7789_TFT_ShowString(0, 152, 16*15, 16, "Connect timeout", 32, 1, WHITE, OLIVE);
				BT_disconnectBnep();
			}
			else if(bnep_status==BNEP_STATUS_DOWNLOADING)
			{
				bnep_status=BNEP_STATUS_CONNECTED;
				MyPrintf("download app timeout\r\n");
				ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
				ST7789_TFT_ShowString(0, 152, 16*20, 16, "download app timeout", 32, 1, WHITE, OLIVE);
			}
		}
	}
	//update download progress
	if(bnep_status==BNEP_STATUS_DOWNLOADING)
	{
		if(SysTick_IsTimeOut(download_progress_tick,5000)==TRUE)
		{
			download_progress_tick=SysTick_GetTick();
			uint8_t strBuf[MAX_INT_STR_SIZE+2];
			int strLen=int2str((app_total_received_len*100)/APP_TOTAL_DOWNLOAD_LEN,strBuf);
			strBuf[strLen]='%';
			strLen +=1;
			strBuf[strLen]=0;
			ST7789_TFT_Clear_White(16*14, 152, 320, 152+42);//清百分比
			ST7789_TFT_ShowString(16*14, 152, 16*strLen, 16, strBuf, 32, 1, WHITE, OLIVE);
		}
	}
}

void show_bt_status(Boolean status)
{
	if(status==TRUE)
	{
		btStatus=TRUE;
		ST7789_TFT_Picture_Q(80, 4, gImage_bt);
	}
	else
	{
		btStatus=FALSE;
		ST7789_TFT_Clear_White(80, 4, 80+15, 4+19);
	}
}
#endif

void Display_InterfaceWindow(void)
{
    st7789_tft_clear(WHITE);
    ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
    ST7789_TFT_ShowChinese(100, 1, Yichip_ch, 24, 5, 1, WHITE, OLIVE);
    ST7789_TFT_Draw_Piece(0, 27, TFT_COL, 28, DGRAY);
    ST7789_TFT_ShowChinese(165, 70, Welcom_ch, 36, 4, 1, WHITE, OLIVE);
    ST7789_TFT_ShowString(175, 111, 128, 16, "YC3121-E", 32, 1, WHITE, OLIVE);
    ST7789_TFT_Picture_Q(25, 80, gImage_Yichip);
    ST7789_TFT_ShowString(10, 160, 300, 12, "TUSN:00000302Q3NL01638914", 24, 1, WHITE, OLIVE);
		#if (VERSIONS == EXIST_BT)
		ST7789_TFT_ShowString(10, 185, 300, 12, "bt name:", 24, 1, WHITE, OLIVE);
		ST7789_TFT_ShowString(106, 185, 300, 12, bt_name, 24, 1, WHITE, OLIVE);
		show_bt_status(FALSE);
		#endif
    TFT_Power_scan();
    TFT_Single_scan();
}

uint8_t meunState = 0;
uint8_t meun2State = 0;
uint8_t changeFlag = 0;

static void Display_MainMeun(void)
{
    ST7789_TFT_Clear_White(0, 70, TFT_COL, TFT_ROW);
    ST7789_TFT_ShowChinese(5, 29, xiaofei_ch, 32, 4, 1, WHITE, OLIVE);
    ST7789_TFT_ShowChinese(155, 29, saoyisao_ch, 32, 5, 1, WHITE, OLIVE);
    ST7789_TFT_ShowChinese(5, 72, chexiao_ch, 32, 4, 1, WHITE, OLIVE);
    ST7789_TFT_ShowChinese(155, 72, jiaoyichaxun_ch, 32, 5, 1, WHITE, OLIVE);
    ST7789_TFT_ShowChinese(5, 115, guanli_ch, 32, 4, 1, WHITE, OLIVE);
    ST7789_TFT_ShowChinese(155, 115, tupianceshi_ch, 32, 5, 1, WHITE, OLIVE);
		#if (VERSIONS == EXIST_BT)
    ST7789_TFT_ShowString(5, 158,  96,  16, "7.BNEP", 32, 1, WHITE, OLIVE);
		#else
		ST7789_TFT_ShowChinese(5, 158, jiesuan_ch, 32, 4, 1, WHITE, OLIVE);
		#endif
    ST7789_TFT_ShowChinese(155, 158, TPTest_ch, 32, 5, 1, WHITE, OLIVE);
    ST7789_TFT_ShowChinese(5, 201, CARDTest_ch, 32, 5, 1, WHITE, OLIVE);
}

uint8_t card_flag = 0;
uint8_t msr = 0;
uint8_t nfc = 0;
uint8_t iccard = 0;

static void Display_card(void)
{
    ST7789_TFT_Clear_White(0, 30, 150, 150);
    ST7789_TFT_ShowChinese(20, 65, CARD_ch, 32, 4, 1, WHITE, BLACK);
    if (msr == 1)
    {
        ST7789_TFT_Picture_Q(180, 40, gImage_msr);
        ST7789_TFT_ShowChinese(106, 225, cancelback_ch, 12, 9, 1, WHITE, BLACK);
    }
    else if (nfc == 1)
    {
        ST7789_TFT_Picture_Q(180, 50, gImage_nfc);
        ST7789_TFT_ShowChinese(106, 225, cancelback_ch, 12, 9, 1, WHITE, BLACK);
    }
    else if (iccard == 1)
    {
        ST7789_TFT_Picture_Q(180, 45, gImage_iccard);
        ST7789_TFT_ShowChinese(106, 225, cancelback_ch, 12, 9, 1, WHITE, BLACK);
    }
}

#if (BOARD_VER == MPOS_BOARD_V2_1)
    //临时定义
    #define KEY_NUM_0   1
    #define KEY_NUM_1   2
    #define KEY_NUM_2   3
    #define KEY_NUM_3   4
    #define KEY_NUM_4   5
    #define KEY_NUM_5   6
    #define KEY_NUM_6   7
    #define KEY_NUM_7   8
    #define KEY_NUM_8   9
    #define KEY_NUM_9   0
    #define KEY_UP      10
    #define KEY_DOWN    11
    #define KEY_CONFIRM 12
    #define KEY_CANCEL  13
    #define KEY_CLEAR   14
    #define KEY_MENU    15
    #define KEY_F1      16
    #define KEY_F2      17
#endif

static void MSR_Test(void)
{
    uint8_t cfmt, tflag;
    uint16_t nResult = 0, ret;
    track_data tdata[MAX_TRACK_NUM];

    card_flag = 1;
    Display_card();
    MyPrintf("start MSR test\n");
    MSR_Init();
    MyPrintf("please swiping card\n");
    while (card_flag)
    {
        if (KEY_Scan() == KEY_CANCEL)
        {
            card_flag = 0;
            msr = 0;
            ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
            ST7789_TFT_ShowString(10, 30,  80,  16, "1.MSR", 32, 1, WHITE, OLIVE);
            ST7789_TFT_ShowString(10, 72,  80,  16, "2.NFC", 32, 1, WHITE, OLIVE);
            ST7789_TFT_ShowString(10, 114, 128, 16, "3.ICCARD", 32, 1, WHITE, OLIVE);
        }
        else
        {
            nResult = MSR_DetectSwipingCard();
            if (nResult == DETECT_SWIPING_CARD)
            {
                cfmt = tflag = 0;
                tdata[1].len = 0;
                ret = MSR_GetDecodeData(tdata, TRACK_SELECT_2, &cfmt, &tflag);

                if (ret == SUCCESS || ret != SUCCESS)
                {
                    if ((tdata[1].len) > 0)
                    {

                        ST7789_TFT_Clear_White(0, 145, TFT_COL, 185);
                        UART_SendBuf(UART0, tdata[1].buf, tdata[1].len);
                        ST7789_TFT_ShowString(32, 145, 256, 16, tdata[1].buf + 1, 32, 1, WHITE, OLIVE);
                        BEEP_Start();
                        delay_ms(200);
                        BEEP_End();
                    }
                }
                MSR_ENABLE(DISABLE);
                MSR_ENABLE(ENABLE);
            }
        }
    }
}

/*
函数功能:  APDU 传输测试
*/
unsigned char Nfc_Apdu_Transmission_Test()
{
    uint8_t err_no, ResBuff[256];
    uint16_t ResLen;
    const uint8_t ApduBuff[] = {0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00};
    err_no = YC_EMV_Contactless_L1_APDU_Transceive(ApduBuff, sizeof(ApduBuff), ResBuff, &ResLen);
    return err_no;
}

extern EMV_Poll_Type_t Poll_Type;

/*
函数功能: 进行TypeA，TypeB 轮询寻卡，有卡进行冲突检测，并激活
*/
unsigned char Nfc_Active_card()
{
    unsigned char i, err_no = EMV_ERR_NONE;
    Poll_Type.Type_A = 0;
    Poll_Type.Type_B = 0;
    if (!Poll_Type.Type_A)
    {
        if (YC_EMV_Contactless_L1_WUPA() != EMV_ERR_TIME_OUT)
        {
            Poll_Type.Type_A = 1;
            YC_EMV_Contactless_L1_HLTA();
        }
        if (!Poll_Type.Type_B)
        {
            if (YC_EMV_Contactless_L1_WUPB(NULL) != EMV_ERR_TIME_OUT)
            {
                Poll_Type.Type_B = 1;
            }
        }
    }
    if (!(Poll_Type.Type_A || Poll_Type.Type_B)) return EMV_ERR_NO_CARD;

    if (Poll_Type.Type_A && Poll_Type.Type_B) return EMV_ERR_COLLISION;

    else if (Poll_Type.Type_A) err_no = EMV_A_Collision_Detect();

    else if (Poll_Type.Type_B) err_no = EMV_B_Collision_Detect();

    if (err_no == EMV_ERR_NONE)
    {
        if (Poll_Type.Type_A) err_no = YC_EMV_Contactless_L1_RATS();

        else if (Poll_Type.Type_B) err_no = YC_EMV_Contactless_L1_ATTRIB();
    }
    return err_no;
}

#define ON  1
#define OFF 0

uint8_t nfc_init = 0;
#define SOFTWARE_RESET 1
#define EXTERNAL_IRRIGATION_CLOCK 1

static void NFC_Spi_Init(void)
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
    SPI_Init(SPI1, &SPI_InitStruct);
}

static void TFT_Spi_Init(void)
{
    SPI_InitTypeDef SPI_InitStruct;

    SPI_InitStruct.BaudRatePrescaler = SPI_BaudRatePrescaler_1;
    SPI_InitStruct.CPHA = SPI_CPHA_First_Edge;
    SPI_InitStruct.CPOL = SPI_CPOL_Low;
    SPI_InitStruct.Mode =  SPI_Mode_Master;
    SPI_InitStruct.RW_Delay = 1;
    #if (BOARD_VER == MPOS_BOARD_V2_1)
    MyPrintf("MPOS_BOARD_V2_1 not support POS demo\r\n");
    #else
    GPIO_Config(ST7789_TFT_SCL_PORT, ST7789_TFT_SCL_PIN, SPID1_SCK);
    GPIO_Config(ST7789_TFT_CS_PORT, ST7789_TFT_CS_PIN, SPID1_NCS);
    GPIO_Config(ST7789_TFT_SDA_PORT, ST7789_TFT_SDA_PIN, SPID1_MOSI);
    SPI_Init(ST7789VTFTSPI, &SPI_InitStruct);
    #endif
}

/*
函数功能:  NFC时钟配置
*/
static void NFC_Systick_Init(void)
{
    IpcInit();
    GPIO_Config(NFC_12M_CLK_PORT, NFC_12M_CLK_PIN, NFC_CLK_OUT);
}

/*
函数功能:  NFC上电初始化配置
形参：Software_reset ：置1 NFC Reset 接主控IO控制；置0 NFC Reset 拉高
	  External_irrigation_clock ：置1 ：NFC时钟不由晶体供给；置0 NFC时钟由晶体供给
*/
static void NFC_CoreReg_Init(uint8_t Software_reset, uint8_t External_irrigation_clock)
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
            WRITEREG(130, 0xA8);
            WRITEREG(137, 0x24);
            WRITEREG(129, 0x5b);
        }
        dc_cali_alg(3);
        YC_EMV_HwReset();
        if (External_irrigation_clock)
        {
            WRITEREG(130, 0xA8);
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
            WRITEREG(130, 0xA8);
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

void NFC_Test(void)
{
    card_flag = 1;
    uint8_t errno = 0;
    uint8_t Resart = 0;
    Display_card();
    MyPrintf("start NFC test\n");
    //	nfc_init = 1;
    NFC_Spi_Init();
    NFC_CoreReg_Init(SOFTWARE_RESET, EXTERNAL_IRRIGATION_CLOCK);
    #if (BOARD_VER == EPOS_BOARD_V1_0)
    GPIO_Config(NFC_TVDD_PORT, NFC_TVDD_PIN, OUTPUT_LOW); //nfc tvdd enable
    #endif
    /*Contactless Init*/
    YC_EMV_Contactless_L1_Init();
    /*Open Rf*/
    YC_EMV_Contactless_L1_Rf_Switch(ON);
    MyPrintf("please swiping card\n");
    NFC_time_start();
    while (card_flag)
    {
        if (KEY_Scan() == KEY_CANCEL)
        {
            card_flag = 0;
            nfc = 0;
            YC_EMV_Contactless_L1_Rf_Switch(OFF);
            #if (BOARD_VER == EPOS_BOARD_V1_0)
            GPIO_Config(NFC_TVDD_PORT, NFC_TVDD_PIN, OUTPUT_HIGH); //nfc tvdd disable
            #endif
            RST_Disable(); //nfc disable
            TFT_Spi_Init();
            ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
            ST7789_TFT_ShowString(10, 30,  80,  16, "1.MSR", 32, 1, WHITE, OLIVE);
            ST7789_TFT_ShowString(10, 72,  80,  16, "2.NFC", 32, 1, WHITE, OLIVE);
            ST7789_TFT_ShowString(10, 114, 128, 16, "3.ICCARD", 32, 1, WHITE, OLIVE);
        }
        else
        {
            if (Resart == 1)
            {
                NFC_CoreReg_Init(SOFTWARE_RESET, EXTERNAL_IRRIGATION_CLOCK);
                Nfc_SysTick_Delay_Ms(500);
                YC_EMV_Contactless_L1_Init();
                YC_EMV_Contactless_L1_Rf_Switch(ON);
                Resart = 0;
#ifdef SDK_DEBUG
                MyPrintf("NFC Resart EOT_SHOUT\n");
#endif
            }
            errno = Nfc_Active_card();
            if (errno == EOT_SHOUT)
            {
#ifdef SDK_DEBUG
                MyPrintf("NFC Active_card EOT_SHOUT\n");
#endif
                Resart = 1;
            }
            if (errno == EMV_ERR_NONE)
            {
                errno = Nfc_Apdu_Transmission_Test();
                if (errno == EOT_SHOUT)
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
                    TFT_Spi_Init();
                    ST7789_TFT_Clear_White(0, 145, TFT_COL, TFT_ROW);
                    ST7789_TFT_ShowString(56, 145, 208, 16, "SWIP SUCCESS!", 32, 1, WHITE, BLACK);
                    BEEP_Start();
                    delay_ms(200);
                    BEEP_End();
                }
                else
                {
                    MyPrintf("Nfc Apdu Transmission Test Fail\n");
                    MyPrintf("Please Swipe Your Card!\n");
                }
                NFC_Spi_Init();
                YC_EMV_Contactless_L1_Rf_Switch(OFF);

                Nfc_SysTick_Delay_Ms(6);

                YC_EMV_Contactless_L1_Rf_Switch(ON);
            }
        }
    }
}

static void ISO7816_IO_Config(void)
{
    sci_pin.clk.gpio_group = GPIOC;
    sci_pin.clk.gpio_pin = GPIO_Pin_13;

    sci_pin.io.gpio_group = GPIOC;
    sci_pin.io.gpio_pin = GPIO_Pin_14;

    sci_pin.reset.gpio_group = GPIOC;
    sci_pin.reset.gpio_pin = GPIO_Pin_15;

    sci_pin.detect.gpio_group = DET_PORT;
    sci_pin.detect.gpio_pin = DET_PIN;
    GPIO_Config(sci_pin.detect.gpio_group, sci_pin.detect.gpio_pin, INPUT | PULL_UP);
    sci_pin.detect_state = 1;

    period_7816 = (CPU_MHZ / 4000000 / 2) - 1;
    contact_reset_l_cnt = 42000 * (CPU_MHZ / 1000000) / 4;
    contact_atr_rfb_time_cnt = 47000 * (CPU_MHZ / 1000000) / 4;
    contact_atr_rab_time_cnt = (20164 * (CPU_MHZ / 1000000) / 4) * 372;

//	atr_time_start();
}

static uint16_t ISO7816_CardDet(void)
{
    uint8_t Reg_Int1;
    uint8_t ATRLen;
    uint16_t Ret;
    uint16_t RecLen;
    uint16_t SendLen;
    uint8_t ApduCmd[280];
    uint8_t ts, t0;


    uint8_t SELECT_PSE[] = {0x00, 0xA4, 0x04, 0x00, 0x0E,
                            0x31, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00
                           };  /* SELECT_PPSE */
    uint8_t i = 0;

TOResetCard:
    atr_time_start();
    delay_ms(100); 			 //5秒
    //激活卡片
    Ret = ISO7816_OperateSelect(ISO7816_ACTIVE_CARD, 1);
    if (Ret == ISO7816_CARD_STUTES_ERROR)
    {
        return 0;
    }
TOWarmResetCard:
    //接收复位信息
    Ret = ISO7816_GetAtr(&g_7816Para.aAtr[1], &ATRLen);
    ts =  g_7816Para.aAtr[1];
    t0 = g_7816Para.aAtr[2];

    //清除atr相关计算器
    if (OK != Ret)
    {
        if ((ISO7816_ATR_LEN_ERROR == Ret))
        {
            if ((g_7816Para.bFlag & BIT1) == 0)
            {
                delay_ms(10);
                ISO7816_OperateSelect(ISO7816_WARM_RESET, 1);
                return 0;
            }
        }
        iso7816_deactive();
        delay_ms(5);
        return 0;
    }
    g_7816Para.aAtr[0] = ATRLen;
    delay_ms(50);

    //自动发送C0
    g_7816Para.bFlag |= BIT2;
    memset(&g_T1.bIFSC, 0, sizeof(g_T1));

    g_T1.bIFSC = 32;
    //ATR数据处理
    Ret = ISO7816_DisposeATR(&g_7816Para.aAtr[1], ATRLen);
    if (OK != Ret)
    {
        if ((Ret == ISO7816_ATR_TA1_ERROR) || (Ret == ISO7816_ATR_TB1_ERROR)
                || (Ret == ISO7816_ATR_TB3_ERROR) || (Ret == ISO7816_ATR_TA3_ERROR)
                || (Ret == ISO7816_ATR_TC2_ERROR) || (Ret == ISO7816_ATR_TC3_ERROR)
                || (Ret == ISO7816_ATR_TA2_ERROR) || (Ret == ISO7816_ATR_TD2_ERROR)
                || (Ret == ISO7816_ATR_TB2_ERROR))
        {
            if ((g_7816Para.bFlag & BIT1) == 0)
            {
                ISO7816_OperateSelect(ISO7816_WARM_RESET, 1);
                return 0;
            }
            else
            {
                ISO7816_OperateSelect(ISO7816_DEACTIVE_CARD, 1);
                delay_ms(10);
                return 0;
            }
        }
        else
        {
            ISO7816_OperateSelect(ISO7816_DEACTIVE_CARD, 1);
            delay_ms(10);
            return 0;
        }
    }
    MyPrintf("reset ok\n");
    return 1;
}

void ICCARD_Test(void)
{
    uint8_t swip_flag = 0;

    card_flag = 1;
    Display_card();
    MyPrintf("start ICCARD test\n");
    ISO7816_IO_Config();
    MyPrintf("please swiping card\n");
    while (card_flag)
    {
        if (KEY_Scan() == KEY_CANCEL)
        {
            card_flag = 0;
            iccard = 0;
            ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
            ST7789_TFT_ShowString(10, 30,  80,  16, "1.MSR", 32, 1, WHITE, OLIVE);
            ST7789_TFT_ShowString(10, 72,  80,  16, "2.NFC", 32, 1, WHITE, OLIVE);
            ST7789_TFT_ShowString(10, 114, 128, 16, "3.ICCARD", 32, 1, WHITE, OLIVE);
        }
        else
        {
            if (GPIO_ReadInputDataBit((GPIO_TypeDef)sci_pin.detect.gpio_group, sci_pin.detect.gpio_pin) == 0 && swip_flag == 0)
            {
                if (ISO7816_CardDet() == 1)
                {
                    swip_flag = 1;
                    if ((g_7816Para.aAtr[1] == 0x3b) || (g_7816Para.aAtr[1] == 0x3f))
                    {
                        ST7789_TFT_ShowString(56, 145, 208, 16, "SWIP SUCCESS!", 32, 1, WHITE, BLACK);
                        BEEP_Start();
                        delay_ms(200);
                        BEEP_End();
                    }
                }
            }
            else
            {
                swip_flag = 0;
            }
        }
    }
}

static void card_test_switch(uint8_t key)
{
    switch (key)
    {
    case KEY_CANCEL:
        changeFlag = 0;
        Display_InterfaceWindow();
        break;
    case KEY_NUM_1:
        msr = 1;
        MSR_Test();
        break;
    case KEY_NUM_2:
        nfc = 1;
        NFC_Test();
        break;
    case KEY_NUM_3:
        iccard = 1;
        ICCARD_Test();
        break;
    default:
        break;
    }
}

static void Display_cardtest(void)
{
    if (changeFlag == 0)
    {
        ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
        ST7789_TFT_ShowString(10, 30,  80,  16, "1.MSR", 32, 1, WHITE, OLIVE);
        ST7789_TFT_ShowString(10, 72,  80,  16, "2.NFC", 32, 1, WHITE, OLIVE);
        ST7789_TFT_ShowString(10, 114, 128, 16, "3.ICCARD", 32, 1, WHITE, OLIVE);
        changeFlag = 1;
        KEY_Scan_check();
    }
    while (changeFlag)
    {
        card_test_switch(KEY_Scan());
    }
}

static void Display_picturetest(void)
{
    uint32_t i = 0;

    if (changeFlag == 0)
    {
        changeFlag = 1;
        KEY_Scan_check();
    }

    while (changeFlag)
    {
        ST7789_TFT_Picture_Q(0, 0, gImage_pic_test1);
        while (i < 10)
        {
            i++;
            switch (KEY_Scan())
            {
            case KEY_CANCEL:
                changeFlag = 0;
                ST7789_TFT_Clear_White(0, 0, TFT_COL, 30);
                ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
                ST7789_TFT_Draw_Piece(0, 27, TFT_COL, 28, DGRAY);
                goto bk;
                break;
            default:
                break;
            }
        }
        i = 0;
        ST7789_TFT_Picture_Q(0, 0, gImage_pic_test2);
        while (i < 10)
        {
            i++;
            switch (KEY_Scan())
            {
            case KEY_CANCEL:
                changeFlag = 0;
                ST7789_TFT_Clear_White(0, 0, TFT_COL, 30);
                ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
                ST7789_TFT_Draw_Piece(0, 27, TFT_COL, 28, DGRAY);
                goto bk;
                break;
            default:
                break;
            }
        }
        i = 0;
    }
bk:
    Display_InterfaceWindow();
    TFT_Power_scan();
    TFT_Single_scan();
}

#if (VERSIONS == EXIST_BT)
static void Display_bnepTest(void)
{
	uint8_t keyCode=0;
	if (changeFlag == 0)
	{
		ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
		ST7789_TFT_ShowString(0, 30,  16*16,  16, "1.connect server", 32, 1, WHITE, OLIVE);
		ST7789_TFT_ShowString(0, 72,  16*20,  16, "2.download 512k data", 32, 1, WHITE, OLIVE);
		ST7789_TFT_ShowString(0, 114, 16*12, 16, "3.disconnect", 32, 1, WHITE, OLIVE);
		changeFlag = 1;
		KEY_Scan_check();
	}
	while (changeFlag)
	{
		keyCode=KEY_Scan();
		switch(keyCode)
		{
			case KEY_CANCEL:
				changeFlag=0;
				break;
			
			case KEY_NUM_1://connect server
				if(btStatus==FALSE)
				{
					if(bnep_status==BNEP_STATUS_IDLE)
					{
						uint8_t phone_mac_addr[22];
						memset(phone_mac_addr,0,sizeof(phone_mac_addr));
						if(BT_ConnectBnep( phone_mac_addr,sizeof(phone_mac_addr))==TRUE)
						{
							bnep_status=BNEP_STATUS_CONNECTING;
							bnep_last_tick=SysTick_GetTick();
							MyPrintf("Connect Bnep cmd ok\n");
							ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
							ST7789_TFT_ShowString(0, 152, 16*13, 16, "Connecting...", 32, 1, WHITE, OLIVE);
						}
						else 
						{
							MyPrintf("Connect Bnep cmd fail\n"); 
							ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
							ST7789_TFT_ShowString(0, 152, 16*16, 16, "Connect cmd fail", 32, 1, WHITE, OLIVE);
						}
					}
					else
					{
						MyPrintf("bnep connected:bnep_status=%02x\r\n",bnep_status);
						ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
						ST7789_TFT_ShowString(0, 152, 16*14, 16, "bnep connected", 32, 1, WHITE, OLIVE);
					}
				}
				else
				{
					MyPrintf("bt/ble is connected\n"); 
					ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
					ST7789_TFT_ShowString(0, 152, 16*19, 16, "bt/ble is connected", 32, 1, WHITE, OLIVE);
				}
				break;
			
			case KEY_NUM_2:
				if(bnep_status==BNEP_STATUS_CONNECTED)
				{
					app_received_len=0;
					app_total_received_len=0;
					load_data_error=FALSE;
					app_crc16=0xffff;
					if(BT_BnepSendTcpData( download_cmd,sizeof(download_cmd))==TRUE)
					{
						bnep_last_tick=SysTick_GetTick();
						MyPrintf("download cmd ok\n");
						bnep_status=BNEP_STATUS_DOWNLOADING;
						download_progress_tick=SysTick_GetTick();
						ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
						ST7789_TFT_ShowString(0, 152, 16*14, 16, "downloading...", 32, 1, WHITE, OLIVE);
					}
					else 
					{
						MyPrintf("download cmd fail\n"); 
						ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
						ST7789_TFT_ShowString(0, 152, 16*17, 16, "download cmd fail", 32, 1, WHITE, OLIVE);
					}
				}
				else
				{
					if(bnep_status==BNEP_STATUS_DOWNLOADING)
					{
						MyPrintf("BNEP_STATUS_DOWNLOADING:bnep_status=%02x\r\n",bnep_status);
						ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
						ST7789_TFT_ShowString(0, 152, 16*14, 16, "downloading...", 32, 1, WHITE, OLIVE);
					}
					else
					{
						MyPrintf("bnep not connect:bnep_status=%02x\r\n",bnep_status);
						ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
						ST7789_TFT_ShowString(0, 152, 16*18, 16, "server not connect", 32, 1, WHITE, OLIVE);
					}
				}
				break;
			
			case KEY_NUM_3:
				if(bnep_status!=BNEP_STATUS_IDLE)
				{
					if(BT_disconnectBnep()==TRUE)
					{
						MyPrintf("disconnect cmd ok\n");
						bnep_last_tick=SysTick_GetTick();
						ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
						ST7789_TFT_ShowString(0, 152, 16*16, 16, "disconnecting...", 32, 1, WHITE, OLIVE);
					}
					else 
					{
						MyPrintf("disconnect cmd fail\n"); 
						ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
						ST7789_TFT_ShowString(0, 152, 16*19, 16, "disconnect cmd fail", 32, 1, WHITE, OLIVE);
					}
				}
				else
				{
					if(bnep_status==BNEP_STATUS_IDLE)
					{
						MyPrintf("bnep not connect:bnep_status=%02x\r\n",bnep_status);
						ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
						ST7789_TFT_ShowString(0, 152, 16*18, 16, "server not connect", 32, 1, WHITE, OLIVE);
					}
					else
					{
						MyPrintf("connecting...:bnep_status=%02x\r\n",bnep_status);
						ST7789_TFT_Clear_White(0, 152, 320, 152+84);//清两行
						ST7789_TFT_ShowString(0, 152, 16*13, 16, "connecting...", 32, 1, WHITE, OLIVE);
					}
				}
				break;
				
			default:
				break;
		}
		do
		{
			bt_progress_idle=FALSE;
			BT_Progress();
		}
		while(!bt_progress_idle);
		if(keyCode!=0)
		{
			KEY_Scan_check();
		}
	}
	MyPrintf("exit download test\r\n");
	if(bnep_status!=BNEP_STATUS_IDLE)
	{
		BT_disconnectBnep();
		while(btStatus);
		bnep_status=BNEP_STATUS_IDLE;
	}
	meunState=0;
	Display_InterfaceWindow();
	TFT_Power_scan();
	TFT_Single_scan();
}
#endif
/*************************************************QR_Code start*****************************************************************/
#include "YC_QRDecode.h"
#include "YC_QRDecode_TypeDef.h"

#define QR_FLASH_BACKUP_ADDR   (0x1000000 + 4 * 200 * 1024)
#define QR_SCAN_TIME_OUT       (0)

uint8_t get_gpio_num(uint16_t groupx, uint16_t pinx)
{
    for (int i = 0; i < GPIO_PIN_NUM; i++)
    {
        if ((1 << i) == pinx)
        {
            return (uint8_t)(groupx * GPIO_PIN_NUM + i);
        }
    }
}

uint8_t camera_setpara[][2] =
{
    {0xfe, 0x00},
//	{0x14, 0x7f},//[1] updown [0]mirror //MIRROR
};

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
        BEEP_Start();
        delay_ms(200);
        BEEP_End();
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
    BEEP_Start();
    delay_ms(100);
    BEEP_End();
    delay_ms(50);
    BEEP_Start();
    delay_ms(100);
    BEEP_End();
    delay_ms(50);
    BEEP_Start();
    delay_ms(100);
    BEEP_End();
    return ;
}

void QRCode_test()
{
    uint8_t keyCode;
    while (1)
    {
        QRCode_Enter(QR_SCAN_TIME_OUT);
        ST7789_TFT_ShowString(0, 200, sizeof("CANCEL             CONFIRM") * 12,  12, "CANCEL             CONFIRM", 24, 1, WHITE, OLIVE);
        while (1)
        {
            keyCode = KEY_Scan();
            if (keyCode == KEY_CANCEL)
            {
                Display_InterfaceWindow();
                return;//退出扫码
            }
            else if (keyCode == KEY_CONFIRM)
            {
                break; // 继续扫码
            }
        }
    }
}
/*************************************************QR_Code end*****************************************************************/

/*************************************************Touch start*****************************************************************/

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
    PAGE0 = 0,
    PAGE1,
    PAGE2,
    PAGE3
} TP_MENU_TypeDef;
/* Private define ------------------------------------------------------------*/

#define TP_X_LEFT_BORDER     (14)
#define TP_X_RIGHT_BORDER    (320 - TP_X_LEFT_BORDER)
#define TP_Y_HIGT_BORDER     (35)
#define TP_Y_LOW_BORDER      (240 - 16*3 - 5)

#define TP_CLEAR_X_LEFT      (320/2 - 5*16/2)
#define TP_CLEAR_Y_LEFT      (240 - 45)
#define TP_CLEAR_X_RIGHT     (TP_CLEAR_X_LEFT + 16*5)
#define TP_CLEAR_Y_RIGHT     (240)

#define TP_EXIT_X_LEFT      (13)
#define TP_EXIT_Y_LEFT      (240 - 45)
#define TP_EXIT_X_RIGHT     (TP_EXIT_X_LEFT + 16*4)
#define TP_EXIT_Y_RIGHT     (240)

#define TP_SAVE_X_LEFT      (320 -13-16*4)
#define TP_SAVE_Y_LEFT      (240 - 45)
#define TP_SAVE_X_RIGHT     (TP_SAVE_X_LEFT + 16*4)
#define TP_SAVE_Y_RIGHT     (240)

/**
  * @brief  ADC initialization function.
  * @param  None
  * @retval None
  */
static void ADC_Configuration(void)
{
    ADC_InitTypeDef ADCInitStruct;
    ADCInitStruct.ADC_Mode = ADC_GPIO;
    ADCInitStruct.ADC_Channel = ADC_CHANNEL_6;
    ADC_Init(&ADCInitStruct);
}

static void Tp_menu(TP_MENU_TypeDef type)
{
    switch (type)
    {
    case PAGE0 :
        ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
        ST7789_TFT_ShowChinese(56, 36, calibrated_ch, 32, 7, 1, WHITE, RED);
        St7789_tft_DrawRectangle(115, 88, 205, 129, BLACK);
        ST7789_TFT_ShowChinese(136, 89, confirm_ch, 24, 2, 1, WHITE, MAROON);
        ST7789_TFT_ShowString(132, 113,  56,  8, (uint8_t *)"CONFIRM", 16, 1, WHITE, MAROON);
        St7789_tft_DrawRectangle(115, 136, 205, 177, BLACK);
        ST7789_TFT_ShowChinese(136, 137, cancel_ch, 24, 2, 1, WHITE, MAROON);
        ST7789_TFT_ShowString(136, 161,  48,  8, (uint8_t *)"CANCEL", 16, 1, WHITE, MAROON);
        St7789_tft_DrawRectangle(115, 184, 205, 225, BLACK);
        ST7789_TFT_ShowChinese(136, 185, clear_ch, 24, 2, 1, WHITE, MAROON);
        ST7789_TFT_ShowString(140, 209,  48,  8, (uint8_t *)"CLEAR", 16, 1, WHITE, MAROON);
        break;
    case PAGE1 :
        ST7789_TFT_ShowChinese(136, 89, confirm_ch, 24, 2, 0, YELLOW, MAROON);
        ST7789_TFT_ShowString(132, 113,  56,  8, (uint8_t *)"CONFIRM", 16, 0, YELLOW, MAROON);
        delay_ms(500);
        ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
        break;
    case PAGE2 :
        ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
        for (uint8_t i = 0; i < 5 ; i++)
        {
            St7789_tft_DrawRectangle(1 + i, 29 + i, 318 - i, 240 - 16 * 3 - i, BLACK);
        }

        ST7789_TFT_ShowString(320 - 13 - 16 * 4, 240 - 45, 16 * 4, 16, (uint8_t *)"Save", 32, 1, WHITE, PURPLE);
        ST7789_TFT_ShowString(13, 240 - 45, 5 * 16, 16, (uint8_t *)"Exit", 32, 1, WHITE, PURPLE);
        ST7789_TFT_ShowString(320 / 2 - 5 * 16 / 2, 240 - 45, 16 * 5, 16, (uint8_t *)"Clear", 32, 1, WHITE, PURPLE);
        break;
    case PAGE3 :
        ST7789_TFT_ShowString(320 / 2 - 5 * 16 / 2, 240 - 45, 16 * 5, 16, (uint8_t *)"Clear", 32, 1, YELLOW, MAROON);
        ST7789_TFT_Clear_White(5, 34, 313, 188);
        ST7789_TFT_ShowString(320 / 2 - 5 * 16 / 2, 240 - 45, 16 * 5, 16, (uint8_t *)"Clear", 32, 1, WHITE, PURPLE);
        break;
    }
}

static Boolean Touch_Cali(void)
{
    uint8_t cal_cnt = 3;
    uint8_t Show_buff1[] = "Calibration success";
    uint8_t Show_buff2[] = "Calibration failure please recalibrate";
    while (cal_cnt --)
    {
        if (Tp_Calibrate() == TRUE)
        {
            ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
            ST7789_TFT_ShowString(10, 80, 8 * sizeof(Show_buff1), 8, Show_buff1, 16, 1, WHITE, MAROON);
            delay_ms(2000);
            ST7789_TFT_Clear_White(0, 30, TFT_COL, TFT_ROW);
            return TRUE;
        }
        else
        {
            ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
            ST7789_TFT_ShowString(10, 80, 8 * sizeof(Show_buff2), 8, Show_buff2, 16, 1, WHITE, MAROON);
            delay_ms(2000);
            ST7789_TFT_Clear_White(0, 28, TFT_COL, TFT_ROW);
        }
    }
}

static void Touch_Drow_Line(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    St7899_tft_DrawLine(sx, sy, ex, ey, BLACK);
    St7899_tft_DrawLine(sx - 1, sy, ex - 1, ey, BLACK);
    St7899_tft_DrawLine(sx, sy + 1, ex, ey + 1, BLACK);
    St7899_tft_DrawLine(sx + 1, sy, ex + 1, ey, BLACK);
    St7899_tft_DrawLine(sx, sy - 1, ex, ey - 1, BLACK);
}

static void Touch_Display(void)
{
    uint8_t keyCode = 0;
    Tp_Sign_Para_TypeDef Sign_Para = {0};

    ADC_Configuration();

    /*Select whether to calibrate*/

    Tp_menu(PAGE0);
    while (1)
    {
        keyCode = KEY_Scan();
        if (keyCode == KEY_CONFIRM)
        {
            Tp_menu(PAGE1);
            Touch_Cali();
            break;
        }
        else if (keyCode == KEY_CANCEL)
        {
            /*Use the default calibration parameters*/
            Constant.kx = TP_CONSTANT_X;
            Constant.ky = TP_CONSTANT_Y;
            fac.kx = TP_FAC_KX;
            fac.ky = TP_FAC_KY;
            break;
        }
        else if (keyCode == KEY_CLEAR)
        {
            return ;
        }
    }

    /*Start the signature*/

    Tp_menu(PAGE2);

    while (1)
    {
        if (Tp_DetectPressing() == TRUE)
        {
            Tp_GetPoint(&Sign_Para.nx, &Sign_Para.ny);

            TP_PRINT("Sign_Para.nx :%d,Sign_Para.ny :%d\n", Sign_Para.nx, Sign_Para.ny);
            TP_PRINT("Sign_Para.lx :%d,Sign_Para.ly :%d\n", Sign_Para.lx, Sign_Para.ly);

            if (Tp_DetectPressing() == FALSE)
            {
                Sign_Para.pressflag = 0;
            }
            /*Continuous press*/
            if (Sign_Para.pressflag == 2)
            {
                if (Sign_Para.nx > TP_X_LEFT_BORDER && Sign_Para.nx < TP_X_RIGHT_BORDER && \
                        Sign_Para.ny > TP_Y_HIGT_BORDER && Sign_Para.ny < TP_Y_LOW_BORDER)
                {
                    if (Tp_AbsolouteDifference(Sign_Para.nx, Sign_Para.lx) < 30 && Tp_AbsolouteDifference(Sign_Para.ny, Sign_Para.ly) < 30)
                    {
                        Touch_Drow_Line(Sign_Para.nx, Sign_Para.ny, Sign_Para.lx, Sign_Para.ly);
                    }
                    Sign_Para.lx = Sign_Para.nx;
                    Sign_Para.ly = Sign_Para.ny;
                }
            }
            /*first press*/

            else if (Sign_Para.nx > TP_X_LEFT_BORDER && Sign_Para.nx < TP_X_RIGHT_BORDER && \
                     Sign_Para.ny > TP_Y_HIGT_BORDER && Sign_Para.ny < TP_Y_LOW_BORDER)
            {
                Sign_Para.pressflag ++;
                Sign_Para.lx = Sign_Para.nx;
                Sign_Para.ly = Sign_Para.ny;
            }
            /*press clear*/

            else if (Sign_Para.nx > TP_CLEAR_X_LEFT && Sign_Para.nx < TP_CLEAR_X_RIGHT && \
                     Sign_Para.ny > TP_CLEAR_Y_LEFT && Sign_Para.ny < TP_CLEAR_Y_RIGHT)
            {
                Tp_menu(PAGE3);
            }
            /*press exit*/

            else if (Sign_Para.nx > TP_EXIT_X_LEFT && Sign_Para.nx < TP_EXIT_X_RIGHT && \
                     Sign_Para.ny > TP_EXIT_Y_LEFT && Sign_Para.ny < TP_EXIT_Y_RIGHT)
            {
                return ;
            }

            /*Save exit*/
            else if (Sign_Para.nx > TP_SAVE_X_LEFT && Sign_Para.nx < TP_SAVE_X_RIGHT && \
                     Sign_Para.ny > TP_SAVE_Y_LEFT && Sign_Para.ny < TP_SAVE_Y_RIGHT)
            {
                return ;
            }
        }
        else
        {
            /*no press*/
            Sign_Para.pressflag = 0;
        }
    }
}

static void Display_touch(void)
{
    uint8_t keyCode = 0;
    uint8_t Show_buff2[] = "Whether or not to continue";
    
    while (1)
    {
        Touch_Display();
        ST7789_TFT_Clear_White(0, 0, TFT_COL, TFT_ROW);
        ST7789_TFT_ShowChinese(100, 1, Yichip_ch, 24, 5, 1, WHITE, OLIVE);
        ST7789_TFT_Draw_Piece(0, 27, TFT_COL, 28, DGRAY);
        TFT_Power_scan();
        TFT_Single_scan();
        ST7789_TFT_ShowString(10, 80, 8 * sizeof(Show_buff2), 8, Show_buff2, 16, 1, WHITE, MAROON);
        ST7789_TFT_ShowString(0, 180, 20 * 16,  16, (uint8_t *)"CANCEL       CONFIRM", 32, 1, WHITE, OLIVE);
        while (1)
        {
            keyCode = KEY_Scan();
            if (keyCode == KEY_CANCEL)
            {
                Display_InterfaceWindow();
                return;
            }
            else if (keyCode == KEY_CONFIRM)
            {
                break;
            }
        }
    }
}

/*************************************************Touch end*****************************************************************/

static void Display_expense(void)
{
    uint8_t key = 0;
    uint8_t num[4] = {"0.00"};

    meunState = 0;

    if (changeFlag == 0)
    {
        ST7789_TFT_Clear_White(0, 29, TFT_COL, TFT_ROW);
        ST7789_TFT_Clear_Gary(0, 28, TFT_COL, 65);
        ST7789_TFT_ShowChinese(112, 31, xiaofei_ch + 128, 32, 3, 1, WHITE, BLACK);
        ST7789_TFT_ShowChinese(72, 83, shurujinge_ch, 32, 6, 1, WHITE, OLIVE);
        ST7789_TFT_ShowString(128, 136, 64, 16, num, 32, 1, WHITE, RED);
        ST7789_TFT_ShowChinese(16, 198, shurucuowu_ch, 32, 9, 1, WHITE, OLIVE);
        changeFlag = 1;
        KEY_Scan_check();
    }
    while (changeFlag)
    {
        key = KEY_Scan();
        switch (key)
        {
        case KEY_CANCEL:
            changeFlag = 0;
            Display_InterfaceWindow();
            break;
        default:
            break;
        }
    }
}

static void MainWindow_switch(uint8_t keyvalue)
{
    switch (keyvalue)
    {
    case KEY_MENU:
        meunState = 1;
        Display_MainMeun();
        break;
    case KEY_NUM_0:
        meunState = 2;
        Display_expense();
        break;
    case KEY_NUM_1:
        meunState = 2;
        Display_expense();
        break;
    case KEY_NUM_2:
        meunState = 2;
        Display_expense();
        break;
    case KEY_NUM_3:
        meunState = 2;
        Display_expense();
        break;
    case KEY_NUM_4:
        meunState = 2;
        Display_expense();
        break;
    case KEY_NUM_5:
        meunState = 2;
        Display_expense();
        break;
    case KEY_NUM_6:
        meunState = 2;
        Display_expense();
        break;
    case KEY_NUM_7:
        meunState = 2;
        Display_expense();
        break;
    case KEY_NUM_8:
        meunState = 2;
        Display_expense();
        break;
    case KEY_NUM_9:
        meunState = 2;
        Display_expense();
        break;
    default:
        break;
    }
}

static void Meun1_switch(uint8_t keyvalue)
{
    switch (keyvalue)
    {
    case KEY_CANCEL:
        meunState = 0;
        Display_InterfaceWindow();
        break;
    case KEY_NUM_1:
        meunState = 0;
        Display_expense();
        break;
    case KEY_NUM_2:
        meunState = 0;
        QRCode_test();
        break;
    case KEY_NUM_6:
        meunState = 0;
        Display_picturetest();
        break;
		
		#if (VERSIONS == EXIST_BT)
    case KEY_NUM_7:
        meunState = 0;
        Display_bnepTest();
        break;
		#endif
		
    case KEY_NUM_8:
        meunState = 0;
        Display_touch();
        break;
    case KEY_NUM_9:
        meunState = 0;
        Display_cardtest();
        break;
    default:
        break;
    }
}

// 检测按键状态，最终修改meunState和changeFlag，或什么都不做
void MeunState_Check_Update(void)
{
    uint8_t keyvalue = 0;

    keyvalue = KEY_Scan();
    if (meunState == 0)
    {
        MainWindow_switch(keyvalue);
    }
    else if (meunState == 1)
    {
        Meun1_switch(keyvalue);
    }
}

void TFT_Power_scan(void)
{
    ST7789_TFT_Picture(284, 4, gImage_full_energe);
//	ST7789_TFT_Picture(284, 4, gImage_low_power);
//	ST7789_TFT_Picture(284, 4, gImage_power_1);
//	ST7789_TFT_Picture(284, 4, gImage_power_2);
//	ST7789_TFT_Picture(284, 4, gImage_power_3);
//	ST7789_TFT_Picture(284, 4, gImage_full_energe);
}

void TFT_Single_scan(void)
{
//	ST7789_TFT_Picture(4, 4, gImage_signal_h);
//	ST7789_TFT_Picture(4, 4, gImage_single3);
//	ST7789_TFT_Picture(4, 4, gImage_single2);
//	ST7789_TFT_Picture(4, 4, gImage_single1);
    ST7789_TFT_Picture(4, 4, gImage_single_l);
}

void TFT_SPI_Init(void)
{
    #if (BOARD_VER == MPOS_BOARD_V2_1)
    MyPrintf("MPOS_BOARD_V2_1 not support POS demo\r\n");
    #else
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
    #endif
}


