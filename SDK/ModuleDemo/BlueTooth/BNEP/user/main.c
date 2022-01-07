/**
  ******************************************************************************
  * @file    3121Demo\ModuleDemo\BlueTooth\BNEP\user\main.c
  * @author  Yichip Application Team
  * @version V1.0.0
  * @date    18-Feb-2020
  * @brief   BNEP test program.
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
//#define UART_TO_IPC

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void UART_Configuration(void);


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


void show_menu()
{
	MyPrintf("====================================\n");
	MyPrintf(" case 0: show menu\n");
	MyPrintf(" case 1: connect bnep(last pair phone)\n");
	MyPrintf(" case 2: connect tcp(bnep test server:47.102.118.39[12345])\n");
	MyPrintf(" case 3: dns connect tcp(webtcp.tongxinmao.com)\n");
	MyPrintf(" case 4: send tcp data\n");
	MyPrintf(" case 5: send tcp big packet data\n");
	MyPrintf(" case 6: send udp data\n");
	MyPrintf(" case 7: disconnect tcp(47.102.118.39)\n");
	MyPrintf(" case 8: disconnect tcp(webtcp.tongxinmao.com)\n");
	MyPrintf(" case 9: disconnect bnep\n");
	MyPrintf("====================================\n");
}


/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

Boolean need_connect=FALSE;
uint8_t tcpdata[MAX_TCP_DATA_LEN+1];
uint8_t bigData[MAX_BIG_DATA_LEN+1];
uint8_t udpData[MAX_BLUETOOTH_DATA_LEN];
int main(void)
{
	uint8_t bt_name[17] = "YC3121bnep";
	uint8_t dns_data[]= "webtcp.tongxinmao.com";
	uint8_t phone_mac_addr[22];
	uint8_t tcp_ip_addr[7]={0x00,0x2f,0x66,0x76,0x27,0x30,0x39};//47.102.118.39:12345
	uint8_t bt_addr[6] =  {0x00,0x00,0x00,0x33,0x22,0x10};//The lowest bit of the highest byte of a BNEP device MAC address must be 0
	uint8_t chip_id[6]={0};
	uint8_t tmpCmd;
	unsigned char newDomainString[]={0x00};
	int UARTCMD = 0;
	uint8_t getbtstatus = 0;
	int i = 0;
	#ifdef UART_TO_IPC
	HCI_Tx.p_data=ipc_tx_buf;
	HCI_Rx.p_data=ipc_rx_buf;
	#endif
	UART_Configuration();
	MyPrintf("bnep demo\n");

	for(i=1;i<sizeof(tcpdata);i++)	tcpdata[i]=i;
	for(i=1;i<sizeof(bigData);i++)	bigData[i]=(i&0xff);

	udpData[0]=192;//UDP remote ip[192.168.2.114]
	udpData[1]=168;
	udpData[2]=2;
	udpData[3]=114;

	udpData[4]=0x35;//local port[13579]
	udpData[5]=0x0b;

	udpData[6]=0x30;//remote port[12347]
	udpData[7]=0x3b;

	for(i=8;i<sizeof(udpData);i++)	udpData[i]=i;

	memset(phone_mac_addr,0,sizeof(phone_mac_addr));

	SysTick_Config(CPU_MHZ/100);
	BT_Init();
	enable_intr(INTR_BT);

	read_chipid(chip_id);
	array2hex(chip_id,3,bt_name+10);
	bt_name[16]='\0';
	memcpy(bt_addr,chip_id,3);

	#ifndef UART_TO_IPC
	if(BT_SetBtName(bt_name,sizeof(bt_name)-1) == TRUE)//bt与ble名字地址可以设置成一样
		MyPrintf("SetbtName_suc\n");
	else
		MyPrintf("SetbtName_fail\n");

	if(BT_SetBtAddr(bt_addr) == TRUE)
		MyPrintf("SetBtAddr_suc\n");
	else
		MyPrintf("SetBtAddr_fail\n");

	if(BT_SetVisibility(0x01,0x01,0x00) == TRUE)  //设置可发现
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

		/*menu*/
	show_menu();
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


		if (UART_IsRXFIFONotEmpty(UART0))
		{
			switch(UART_ReceiveData(UART0))
			{
				case '0':
					show_menu();
				break;

				case '1':
					if(BT_ConnectBnep( phone_mac_addr,sizeof(phone_mac_addr))==TRUE)
					{
						MyPrintf("Connect Bnep cmd ok\n");
					}
					else
					{
						MyPrintf("Connect Bnep cmd fail\n");
					}
				break;

				case '2':
					MyPrintf("start connect:47.102.118.39:12345\n");
					if(BT_ConnectTcp(tcp_ip_addr,sizeof(tcp_ip_addr))==TRUE)
					{
						MyPrintf("Connect Tcp cmd ok\n");
					}
					else
					{
						MyPrintf("Connect Tcp cmd fail\n");
					}
					break;

				case '3':
					if(BT_DnsReq(dns_data, sizeof(dns_data)-1)==TRUE)
					{
						need_connect=TRUE;
						MyPrintf("Dns Req cmd ok\n");
					}
					else
		      {
						MyPrintf("Dns Req cmd fail\n");
					}
				break;

				case '4':
				{
					tcpdata[0]=0x00;//tcp handle
					MyPrintf("Send Tcp Data[%d]:\n",sizeof(tcpdata)-1);
					MyPrintf("handle:%02x\n",tcpdata[0]);
					for(int i=0;i<(sizeof(tcpdata)-1);i++)	MyPrintf("%02x ",tcpdata[1+i]);
					if(BT_BnepSendTcpData(tcpdata,sizeof(tcpdata))==TRUE)
					{
						MyPrintf("\nSend Tcp Data sucess\n");
					}
					else
					{
						MyPrintf("Send Tcp Data fail\n");
					}
				}
				break;

				case '5':
				{
					bigData[0]=0x00;//tcp handle
					MyPrintf("Send Tcp big Data[%d]:\n",sizeof(bigData)-1);
					MyPrintf("handle:%02x\n",bigData[0]);
					for(int i=0;i<(sizeof(bigData)-1);i++)	MyPrintf("%02x ",bigData[1+i]);
					if(BT_BnepSendTcpBigData(bigData,sizeof(bigData))==TRUE)
					{
						MyPrintf("\nSend Tcp big Data sucess\n");
					}
					else
					{
						MyPrintf("Send Tcp big Data fail\n");
					}
				}
				break;

				case '6':
				{
					MyPrintf("Send UDP Data[%d]:\n",sizeof(udpData)-8);
					for(int i=8;i<sizeof(udpData);i++)	MyPrintf("%02x ",udpData[i]);
					MyPrintf("\nremote ip:");
					for(int i=0;i<4;i++)	MyPrintf("%d.",udpData[i]);
					MyPrintf("remote port:%d ",((udpData[6]<<8)|udpData[7]));
					MyPrintf("local port:%d",((udpData[4]<<8)|udpData[5]));
					if(BT_BnepSendUdpData(udpData,sizeof(udpData))==TRUE)
					{
						MyPrintf("\nSend UDP Data sucess\n");
					}
					else
					{
						MyPrintf("Send UDP Data fail\n");
					}
				}
				break;

			case '7':
				MyPrintf("start disconnect:47.102.118.39\n");
				if(BT_DisconnectTcp(0x00))
				{
		      MyPrintf("Disconnect Tcp cmd ok\n");
				}
				else
		    {
					MyPrintf("Disconnect Tcp cmd fail\n");
				}
				break;

			case '8':
				MyPrintf("start disconnect:webtcp.tongxinmao.com\n");
				if(BT_DisconnectTcp(0x01))
				{
		      MyPrintf("Disconnect Tcp cmd ok\n");
				}
				else
		    {
					MyPrintf("Disconnect Tcp cmd fail\n");
				}
				break;

			case '9':
				MyPrintf("start disconnect bnep\n");
				if(BT_disconnectBnep())
				{
		      MyPrintf("Disconnect bnep cmd ok\n");
				}
				else
		    {
					MyPrintf("Disconnect bnep cmd fail\n");
				}
				break;

				default:
					break;
			}
		}
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
#define NAME_OFFSET_INDEX			0
#define NAME_LEN_INDEX				1
#define NAME_START_INDEX			2
#define NAME_MAC_INDEX				16
#define NAME_MAC_LEN				6
#define NAME_LINKKEY_INDEX			22
#define NAME_LINKKEY_LEN			16
#define MAX_SUBPACKAGE_NAME_LEN		14
#define MAX_BT_MASTER_NAME_LEN		(MAX_SUBPACKAGE_NAME_LEN*4+1)
uint8_t bt_master_name[MAX_BT_MASTER_NAME_LEN];
void BT_Progress()
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
				//add port 10002(0x2712)
				tcp_addr[5]=0x27;
				tcp_addr[6]=0x12;
				if(need_connect)
				{
					need_connect=FALSE;
					MyPrintf("start connect:webtcp.tongxinmao.com\n");
					if(BT_ConnectTcp(tcp_addr,sizeof(tcp_addr))==TRUE)
					{
						MyPrintf("Connect Tcp cmd ok\n");
					}
					else
					{
						MyPrintf("Connect Tcp cmd fail\n");
					}
				}
			}
			else
			{
				MyPrintf("read dns_rsp error\n");
			}
		}
		break;

		case BNEP_RECEIVE_TCP_DATA:
			{
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
				}
				else	MyPrintf("read bnep_recieve_tcp_data error\n");
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
				else	MyPrintf("read udp info error\n");
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
				else	MyPrintf("read bnep_recive_udp_data error\n");
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
			}
		break;

		case TCP_CONNECT_LOG:
			{
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
					#if (MEM_SPP_FLOWCTRL_FLAG_VALUE==0x01)
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
				BleDataLen=BT_ReadBTData(bleData);
				if(BleDataLen>0)
				{
					MyPrintf("received %d bytes ble data[handle:%02x %02x]:\n",BleDataLen-2,bleData[0],bleData[1]);
					for(int i=0;i<(BleDataLen-2);i++)	MyPrintf("%02x ",bleData[2+i]);
					MyPrintf("\n");

					//add handle
					bleData[0]=(ble_send_handle&0xff);
					bleData[1]=((ble_send_handle>>8)&0xff);
					BT_SendBleData(bleData,BleDataLen);
					MyPrintf("send %d bytes ble data[%02x %02x]:\n",BleDataLen-2,bleData[0],bleData[1]);
					for(int i=0;i<(BleDataLen-2);i++)	MyPrintf("%02x ",bleData[2+i]);
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

		case BNEP_CONNECT:
			MyPrintf("bnep connected\n");
		break;

		case BB_DISCONNECT:
			MyPrintf("bt baseband disconnected\n");
		break;

		case TCP_CONNECT:
			{
				MyPrintf("tcp connected\n");
				uint8_t connect_handle[MAX_BLUETOOTH_DATA_LEN];
				int connectHandleLen=0;
				connectHandleLen=BT_ReadBTData(connect_handle);
				if(connectHandleLen>0)
				{
					MyPrintf("handle:%02x\n",connect_handle[0]);
				}
				else
				{
					MyPrintf("read connect_handle error\n");
				}
			}
		break;

		case TCP_DISCONNECT:
			{
				MyPrintf("tcp disconnected\n");
				uint8_t disconnect_handle[MAX_BLUETOOTH_DATA_LEN];
				int disconnectHandleLen=0;
				disconnectHandleLen=BT_ReadBTData(disconnect_handle);
				if(disconnectHandleLen>0)
				{
					MyPrintf("handle:%02x\n",disconnect_handle[0]);
				}
				else
				{
					MyPrintf("read disconnect_handle error\n");
				}
			}
		break;

		case BNEP_DISCONNECT:
			MyPrintf("bnep disconnected\n");
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
				else	MyPrintf("read bt master name error\n");
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

