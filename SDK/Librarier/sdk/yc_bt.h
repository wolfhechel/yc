/**
  ******************************************************************************
  * @file    yc_bt.h
  * @author  Yichip
  * @version V1.1
  * @date    28-May-2020
  * @brief   Yichip bluetooth encapsulation.The HCI protocol refers to YC Dual mode module spec-v4.15
*
  ******************************************************************************
  */


#ifndef __YC_BT_H__
#define __YC_BT_H__

#include "yc_uart.h"
#include "yc_ipc.h"


#define HCI_CMD                           0x01
#define HCI_EVENT                         0x02
#define MAX_BLUETOOTH_DATA_LEN            255
#define MAX_TCP_DATA_LEN                  235
#define MAX_BIG_DATA_LEN                  1360



/*BIT judge*/
#define BIT_SET_BT_ADDR                   0
#define BIT_SET_BLE_ADDR                  1
#define BIT_SET_VISIBILITY                2
#define BIT_SET_BT_NAME                   3
#define BIT_SET_BLE_NAME                  4
#define BIT_SEND_SPP_DATA                 5
#define BIT_SEND_BLE_DATA                 6
#define BIT_STATUS_REQUEST                7
#define BIT_SET_PAIRING_MODE              8
#define BIT_SET_PINCODE                   9
#define BIT_GET_VERSION                   10
#define BIT_BT_DISCONNECT                 11
#define BIT_BLE_DISCONNECT                12
#define BIT_SET_NVRAM                     13
#define BIT_SET_TXPOWER                   14
#define BIT_ENTER_SLEEP_MODE              15
#define BIT_CONFIRM_GKEY                  16
#define BIT_POWER_SET                     17
#define BIT_GET_CREADIT_GIVEN             18
#define BIT_PASSKEY_ENTRY                 19
#define BIT_LE_SET_PARING                 20
#define BIT_LE_SET_ADV_DATA               21
#define BIT_LE_SET_SCAN_DATA              22
#define BIT_LE_SEND_CONN_UPDATE           23
#define BIT_LE_SET_ADV_PARM               24
#define BIT_REJECT_JUSTWORK               25
#define BIT_LE_SET_FIXED_PASSKEY          26
#define BIT_SET_PSCAN_INTERVAL            27
#define BIT_MODULE_READY                  28
#define	BIT_SET_COD                       29
#define BIT_SET_CREDIT                    30
#define BIT_DELETE_SERVICE                31
#define BIT_ADD_UUID                      32
#define BIT_DNS_REQ                       33
#define BIT_CONNECT_BNEP                  34
#define BIT_CONNECT_TCP                   35
#define BIT_BNEP_SEND_TCP_DATA            36
#define BIT_DISCONNECT_TCP                37
#define BIT_BNEP_SEND_UDP_DATA            38
#define BIT_BNEP_SEND_TCP_BIG_DATA        39
#define BIT_BNEP_SEND_TCP_BIG_DATA_FINISH 40
#define BIT_BNEP_SEND_TCP_BIG_DATA_STATUS 41
#define BIT_SET_LPM_MODE                  42
/*CMD*/
#define HCI_CMD_SET_BT_ADDR               0x00  //设置 BT3.0 地址
#define HCI_CMD_SET_BLE_ADDR              0x01  //设置 BLE 地址
#define HCI_CMD_SET_VISIBILITY            0x02  //设置可发现和广播
#define HCI_CMD_SET_BT_NAME               0x03  //设置 BT3.0 名称
#define HCI_CMD_SET_BLE_NAME              0x04  //设置 BLE 名称
#define HCI_CMD_SEND_SPP_DATA             0x05  //发送 BT3.0（SPP）数据
#define HCI_CMD_SEND_BLE_DATA             0x09  //发送 BLE 数据
#define HCI_CMD_STATUS_REQUEST            0x0B  //请求蓝牙状态
#define HCI_CMD_SET_PAIRING_MODE          0x0C  //设置配对模式
#define HCI_CMD_SET_PINCODE               0x0D  //设置配对码
#define HCI_CMD_VERSION_REQUEST           0x10  //查询模块固件版本
#define HCI_CMD_BT_DISCONNECT             0x11  //断开 BT3.0 连接
#define HCI_CMD_BLE_DISCONNECT            0x12  //断开 BLE 连接
#define HCI_CMD_SET_NVRAM                 0x26  //下发 NV 数据
#define HCI_CMD_ENTER_SLEEP_MODE          0x27  //进入睡眠模式
#define HCI_CMD_CONFIRM_GKEY              0x28  //Numeric Comparison 配对方式中对密钥的比较（见HCI_EVENT_GKEY注释）
#define HCI_CMD_GET_CREADIT_GIVEN         0x29
#define HCI_CMD_PASSKEY_ENTRY             0x30  //passkey配对方式中对密钥的比较
#define HCI_CMD_LE_SET_PAIRING            0x33
#define HCI_CMD_LE_SET_ADV_DATA           0x34
#define HCI_CMD_LE_SET_SCAN_DATA          0x35
#define HCI_CMD_LE_SEND_CONN_UPDATE_REQ   0x36
#define HCI_CMD_LE_SET_ADV_PARM           0x37
#define HCI_CMD_SET_TX_POWER              0x42
#define HCI_CMD_REJECT_JUSTWORK           0x49
#define HCI_CMD_LE_SET_FIXED_PASSKEY      0x61
#define	HCI_CMD_DELETE_CUSTOMIZE_SERVICE  0x76
#define	HCI_CMD_ADD_SERVICE_UUID          0x77
#define	HCI_CMD_ADD_CHARACTERISTIC_UUID   0x78
#define	HCI_CMD_SET_COD                   0x15  //设置 COD
#define HCI_CMD_CONNECT_BNEP              0x7e //建立BNEP连接
#define HCI_CMD_CONNECT_TCP               0x7f //建立TCP连接
#define HCI_CMD_DISCONNECT_TCP            0x80 //断开TCP连接
#define HCI_CMD_BNEP_SEND_TCP_DATA        0x81 //发送TCP数据
#define HCI_CMD_DNS_REQ                   0x82 //域名解析指令
#define HCI_CMD_SEND_UDP_DATA             0x84 //发送UDP数据
#define HCI_CMD_SEND_BIG_DATA             0x85 //发送TCP大包数据
#define HCI_CMD_SET_LPM_MODE              0xff //开关低功耗


/*EVENT*/
#define HCI_EVENT_BT_CONNECTED            0x00  /**BT3.0 连接建立*/
#define HCI_EVENT_BLE_CONNECTED           0x02  /**BLE 连接建立*/
#define HCI_EVENT_BT_DISCONNECTED         0x03  /**BT3.0 连接断开*/
#define HCI_EVENT_BLE_DISCONNECTED        0x05  /**BLE 连接断开*/
#define HCI_EVENT_CMD_COMPLETE            0x06  /**命令已完成*/
#define HCI_EVENT_SPP_DATA_RECEIVED       0x07  /**接收到 BT3.0 数据（SPP）*/
#define HCI_EVENT_BLE_DATA_RECEIVED       0x08  /**接收到 BLE 数据*/
#define HCI_EVENT_I_AM_READY              0x09  /**模块准备好*/
#define HCI_EVENT_STAUS_RESPONSE          0x0A  /**状态回复*/
#define HCI_EVENT_NVRAM_CHANGED           0x0D  /**上传 NVRAM 数据*/
#define HCI_EVENT_GKEY                    0x0E  /**HCI 包格式错误*/
#define HCI_EVENT_UART_EXCEPTION          0x0F  /*cmd format error*/
#define HCI_EVENT_GET_PASSKEY             0x10  /**PASSKEY 配对方式中通知MCU 返回密钥,MCU收到此事件后需发送HCI_CMD_PASSKEY_ENTRY指令回复蓝牙模块*/
#define HCI_EVENT_PAIRING_COMPLETED       0x11  /**配对完成的事件，通知 MCU配对完成*/
#define HCI_EVENT_LE_PAIRING_STATE        0x14  /**上报配对状态*/
#define	HCI_EVENT_UUID_HANDLE             0x29
#define HCI_EVENT_BNEP_CONNECT            0x2c  /**通知MCU BNEP连接成功*/
#define HCI_EVENT_BNEP_DISCONNECT         0x2d  /**通知MCU BNEP断开连接*/
#define HCI_EVENT_BNEP_CONNECT_FAIL       0x2e  /**通知MCU BNEP连接失败原因*/
#define HCI_EVENT_TCP_CONNECT             0x2f /**通知MCU TCP连接成功*/
#define HCI_EVENT_TCP_DISCONNECT          0x30 /**通知MCU TCP断开连接*/
#define HCI_EVENT_BNEP_RECEIVE_TCP_DATA   0x31  /*接收到TCP数据*/
#define HCI_EVNET_TCP_FAIL                0x32  /*TCP连接或者断开失败*/
#define HCI_EVENT_TCP_CONNECT_LOG         0x33  /*TCP连接LOG*/
#define HCI_EVENT_BNEP_CHIP_ERROR         0x34  /*芯片不支持BNEP*/
#define HCI_EVENT_DNS_RSP                 0x35  /*DNS返回的ip地址*/
#define HCI_EVENT_BB_DISCONNECT           0x36  /*经典蓝牙baseband断开后返回的事件*/
#define HCI_EVENT_BT_NAME                 0x37  /*BT master name*/
#define HCI_EVENT_UDP_INFO                0x38  /*UDP信息头,包含远端IP、本地IP、远端端口、本地端口,后面会紧随一包UDP数据*/
#define HCI_EVENT_BNEP_RECEIVE_UDP_DATA   0x39  /*接收到UDP数据*/
#define HCI_EVENT_BIG_DATA_FINISH         0x3a  /*大数据分包发送完成事件*/
#define HCI_EVENT_BIG_DATA_STATUS         0x3b  /*大数据分包发送状态*/


/**
  * @brief  event code enum definition
  */
typedef enum
{
	BT_CONNECTED        		=HCI_EVENT_BT_CONNECTED,
	BLE_CONNECTED       		=HCI_EVENT_BLE_CONNECTED,
	BT_DISCONNECTED     		=HCI_EVENT_BT_DISCONNECTED,
	BLE_DISCONNECTED    		=HCI_EVENT_BLE_DISCONNECTED,
	BT_DATA             		=HCI_EVENT_SPP_DATA_RECEIVED,
	BLE_DATA            		=HCI_EVENT_BLE_DATA_RECEIVED,
	NVRAM_DATA          		=HCI_EVENT_NVRAM_CHANGED,
	CONFIRM_GKEY        		=HCI_EVENT_GKEY,
	PASSKEY             		=HCI_EVENT_GET_PASSKEY,
	PAIRING_COMPLETED   		=HCI_EVENT_PAIRING_COMPLETED,
	PAIRING_STATE        		=HCI_EVENT_LE_PAIRING_STATE,
	BNEP_CONNECT        		=HCI_EVENT_BNEP_CONNECT,
	TCP_CONNECT         		=HCI_EVENT_TCP_CONNECT,
	BNEP_RECEIVE_TCP_DATA		=HCI_EVENT_BNEP_RECEIVE_TCP_DATA,
	TCP_DISCONNECT       		=HCI_EVENT_TCP_DISCONNECT,
	BNEP_DISCONNECT      		=HCI_EVENT_BNEP_DISCONNECT,
	BNEP_CONNECT_FAIL    		=HCI_EVENT_BNEP_CONNECT_FAIL,
	TCP_FAIL             		=HCI_EVNET_TCP_FAIL,
	TCP_CONNECT_LOG      		=HCI_EVENT_TCP_CONNECT_LOG,
	BNEP_CHIP_ERROR      		=HCI_EVENT_BNEP_CHIP_ERROR,
	DNS_RSP              		=HCI_EVENT_DNS_RSP,
	BB_DISCONNECT        		=HCI_EVENT_BB_DISCONNECT,
	BT_NAME              		=HCI_EVENT_BT_NAME,
	UDP_INFO             		=HCI_EVENT_UDP_INFO,
	BNEP_RECEIVE_UDP_DATA		=HCI_EVENT_BNEP_RECEIVE_UDP_DATA

}BT_EventCodeEnum;



typedef struct
{
	uint8_t* pBuf;
    	int BufSize;
    	int count;		 /*!< count of valid data */
    	int ReadIndex;
    	int WriteIndex;
} BT_BufTypeDef;

/**
  * @brief   Set the address of the bluetooth 3.0 device
  * @param  bt_addr: pointer to  address(6 bytes)
  * @retval TRUE:set addr success.
  *         FALSE:set addr failed
  */
Boolean BT_SetBtAddr(uint8_t * bt_addr);


/**
  * @brief  Set the address of the bluetooth 4.0 device
  * @param  bt_addr: pointer to  address(6 bytes)
  * @retval TRUE:set addr success.
  *         FALSE:set addr failed
  */
Boolean BT_SetBleAddr(uint8_t * bt_addr);


/**
  * @brief   Set the name of the bluetooth 3.0 device
  * @param  bt_addr: pointer to  address information.
  *         name_len: name length,not include '\0'
  * @retval TRUE:set name success.
  *         FALSE:set name failed
  */
Boolean BT_SetBtName(uint8_t * bt_name, uint16_t name_len);


/**
  * @brief   Set the name of the bluetooth 4.0 device
  * @param  ble_name: pointer to  name information.
  *         name_len: name length,not include '\0'
  * @retval TRUE:set name success.
  *         FALSE:set name failed
  */
Boolean BT_SetBleName(uint8_t* ble_name, uint16_t name_len);


/**
  * @brief  Set visibility mode.
  * @param  bt_discoverable: 0--bt_discoverable OFF;1--bt_discoverable ON.
  *         bt_connectability:0--bt_connectability OFF;1--bt_connectability ON.
  *         ble_discoverable:0--ble_discoverable OFF;1--ble_discoverable ON.
  * @retval TRUE:set visibility success.
  *         FALSE:set visibility failed
  */
Boolean BT_SetVisibility(Boolean bt_discoverable, Boolean bt_connectability, Boolean ble_discoverable);


/**
  * @brief  Bt send spp(bt3.0) data
  * @param  spp_data: pointer to  spp data
  *         len: spp data length(max len is 255)
  * @retval TRUE:send spp data success.
  *         FALSE:send spp data failed
  */
Boolean BT_SendSppData(uint8_t * spp_data, uint16_t len);


/**
  * @brief  Send ble data
  * @param  ble_data: pointer to  ble data ,includes two bytes handle(byte0-byte1 is ble handle).
  *         len: data length(max len is 255)
  * @retval TRUE:send ble data success.
  *         FALSE:send ble data failed
  */
  Boolean BT_SendBleData(uint8_t * ble_data, uint16_t len);


/**
  * @brief  get bluetooth connect status
  * @param  None
  * @retval bit0:BT 3.0 Can be discover
  *         bit1:BT 3.0 Can be connected
  *         bit2:BT 4.0 Can be discover and connected
  *         bit4:BT 3.0 connected
  *         bit5:BT 4.0 connected
  *         bit7:get status timer out
  */
uint8_t Bt_GetBtStatus(void);



/**
  * @brief  set classic bluetooth paring mode
  * @param  mode:the paramater can be one of the following value
                0x00:pincode
                0x01:just work
                0x02:passkey
                0x03:confirm
  * @retval	   ture or FALSE
  */
  Boolean BT_SetParingMode(uint8_t mode);



/**
  * @brief  set classic bluetooth paring PIN code
  * @param  Pincode:the pointer ponint to pincode data
            len: normally the value can be 0x01~0x10,
  * @retval	   ture or FALSE
  */
Boolean BT_SetPincode(uint8_t* Pincode,uint8_t len);



/**
  * @brief  get the bluetooth module firmware version number
  * @param  none
  * @retval firmware version number(the value is 1~65535),0 is fail
  *
  */
uint32_t BT_GetVersion(void);


/**
  * @brief  bt disconnect
  * @param  none
  * @retval TRUE:bt disconnected success.
  *         FALSE:bt disconnected failed
  */
Boolean BT_BtDisconnect(void);

/**
  * @brief  Set NVRAM
  * @param  NvData:the pointer point to NVRAMData
	* 				len:bnep is 34(one group),other is 170(5*34=170 five group).
  * @retval TRUE:bt disconnected success.
  *         FALSE:bt disconnected failed
  */
Boolean BT_SetNVRAM(uint8_t * NvData,int len);


/**
  * @brief  Bt Enter Sleep mode(enter_hibernate)
  * @param  none
  * @retval TRUE:enter sleep mode success
  *
  */
Boolean BT_EnterSleepMode(void);


/**
  * @brief  Bt set LPM mode
  * @param  mode:0x00-close LPM mode; 0x01-open LPM mode
  * @retval TRUE or FALSE
  *
  */
Boolean BT_SetLpmMode(uint8_t mode);



/**
  * @brief  Numeric Comparison key matching in paring mode
  * @param  mismatching:the paramater can be the follwing value
                        0x00: key match
                        0x01: key mismatch
  * @retval TRUE:set ConfirmGkey success
  *         FALSE:set ConfirmGkey failed
 */
Boolean BT_ConfirmGkey(uint8_t isMatching);


/**
  * @brief  set spp flow control
  * @param  packetNum:the flow control packet number
  * @retval TRUE:success
  *         FALSE:failed
 */
Boolean BT_SetSppFlowcontrol(uint8_t packetNum);


/**
  * @brief  Comparison of keys in  PASSKEY paring
  * @param  key_data: point to key data,length is 4 bytes,(data in hexadecimal form,
  *                   starting with high bytes;for example ,218372(10) = 0x35504 ->key_data[]={0x00,0x03,0x55,0x04})
  * @retval TRUE:set passkey success
  *         FALSE:set passkey  failed
  */
Boolean BT_PasskeyEntry(uint8_t *key_data);

/**
  * @brief  Set ble paring mode
  * @param  mode: the paramater can be the follwing value
  *               0x00:none
  *               0x01:just work
  *               0x02:pass key
  *               0x81:secure connect just work
  *               0x82:secure connect numeric
  *               0x83:secure connect pass key
  * @retval TRUE:set ble paring success
  *         FALSE:set ble paring  failed
  */
Boolean BT_SetLEParing(uint8_t mode);

/**
  * @brief    Send ble adv data
  * @param    adv_data: pointer to adv data.
	*  					DataLen: data length(must be 0x1f)
  * @retval	  TRUE:set ble adv  data success.
  *           FALSE:set ble adv data failed
  */
Boolean BT_SetLEAdvData(uint8_t*  adv_data, int DataLen);


/**
  * @brief  set ble scan data
  * @param  scan_data: pointer to  scan data.
  *         DataLen: data length(length < 0x20)
  * @retval TRUE:set ble scan  data success.
  *         FALSE:set ble scan data failed
  */
Boolean BT_SetLEScanData(uint8_t* scan_data, int DataLen);

/**
  * @brief  ble send update ble connect parameter data
  * @param  data:
  *             byte0-byte1:min connect interval
  *             byte2-byte3:max connect interval
  *             byte4-byte5:Slave  latency
  *             byte6-byte7:Connection Supervision Timeout
  *         len:len must be 8
  * @retval TRUE:success.
  *         FALSE:failed
  */
Boolean BT_SetLESendConnUpdate(uint8_t *data,int len);


/**
  * @brief  Set ble adv interval
  * @param  ble_adv_interval: ble adv interval is ble_adv_interval*0.625ms
  * @retval TRUE:set ble adv parm success.
  *         FALSE:set ble adv parm failed
  */
Boolean BT_SetLEAdvInterval(uint16_t ble_adv_interval);

/**
  * @brief  Set reject just work
  * @param  justwork:
  *                 0:turn off reject just work
  *                 1:turn on reject just work
  * @retval TRUE:set ble scan  data success.
  *         FALSE:set ble scan data failed
  */
Boolean BT_RejectJustWork(uint8_t justwork);


/**
  * @brief  Set fixed passkey
  * @param  key: pointer to key data ,key length is 5 bytes
  * @retval TRUE:set fixed passkey  success.
  *         FALSE:set ble fixed passkey failed
  */
Boolean BT_Set_FixedPasskey(uint8_t* key);


/**
  * @brief  ble disconnect
  * @param  none
  * @retval TRUE:ble disconnected success.
  *         FALSE:ble disconnected failed
  */
Boolean BT_BleDisconnect(void);


/**
  * @brief  Set spp(bt3.0) class of device
  * @param  bt_cod:cod buf(must be 3 byte)
  * @retval TRUE:success.
  *         FALSE:failed
  */
Boolean BT_SetCOD(uint8_t* bt_cod);




/**
  * @brief  Set RF tx power
  * @param  power: 0:0db
  *                1:3db
  *                2:5db
  *                3:-3db
  *                4:-5db
  * @retval TRUE:set SetTxPower  success.
  *         FALSE:set SetTxPower failed
  */
Boolean BT_SetTxPower(uint8_t power);


/**
  * @brief  Delete ble custom Service
  * @param  none
  * @retval TRUE:success.
  *         FALSE:failed
  */
Boolean BT_DeleteService(void);


/**
  * @brief  add ble custom Service
  * @param  ble_service_uuid:
	*														byte0: uuid length(2 or 16)
	*														byte1-2(16): uuid
	*					service_uuid_len: sizeof(ble_service_uuid)
  * @retval 0:failed
  *         other:Service handle
  */
uint16_t BT_AddBleService(uint8_t* ble_service_uuid, uint16_t service_uuid_len);


/**
  * @brief  add ble custom Characteristic
  * @param  ble_Characteristic_uuid:
	*														byte0: characterisitic attribute
	*																	bit0		Broadcast
	*																	bit1		Read
	*																	bit2		Write without Response
	*																	bit3		Write
	*																	bit4		Notify
	*																	bit5		Indicate
	*																	bit6		Authentication Signed Write
	*																	bit7		Extended Properties
	*														byte1:			characterisitic uuid length(2 or 16)
	*														byte2-3(17):characterisitic uuid
	*														byte4(18):	write/read payload length(1--20;default:1)
	*														byte5(19)-x:write/read payload(default:00)
	*					service_Characteristic_payload_len: sizeof(ble_Characteristic_uuid)
  * @retval 0:failed
  *         other:Characteristic handle
  */
uint16_t BT_AddBleCharacteristic(uint8_t* ble_Characteristic_uuid, uint16_t service_Characteristic_payload_len);

/**
  * @brief  Read the received bluetooth data from receive buffer
  * @param  pbuf:the output buffer,255 bytes is enough
  *
  * @retval The data length of the readed,0 is fail
  */
int BT_ReadBTData(uint8_t* pbuf);


/**
  * @brief   get event opcode of bluetooth module
  *
  * @param  none
  *
  * @retval -1		:no data
  *         other	:event opcode
  */
int BT_GetEventOpcode(void);


/**
  * @brief   parse bluetooth module data packet according to yichip HCI protocol from ipc buf,you shoud call it at BT_UART IRQHandler function
  * @param  Unone
  *
  * @retval none
  */
void BT_ParseBTData(void);




/**
  * @brief   Handling user events about bluetooth module,This is not an interrupt function,A circular call is required to detect bluetooth data
  *
  * @param  none
  *
  * @retval none
  */
void BT_Progress(void);


/**
  * @brief  BT Init
  * @param  none
  * @retval TRUE:success
  *         FALSE:fail
  */
Boolean BT_Init(void);

/**
  * @brief  BT_WakeupInit (M0 Sleep,Bt lpm Sleep)
  * @param  none
  * @retval TRUE:success
  *         FALSE:fail
  */
Boolean BT_WakeupInit(void);

/**
  * @brief  DNS(domain name resolution)
  * @param  dns_data: pointer to  domain name(www.baidu.com).
            DataLen: data length(max 62 bytes)
  * @retval TRUE:success
  *         FALSE:fail
  */
Boolean BT_DnsReq( uint8_t *dns_data,int len);
/**
  * @brief  Connect Bnep
  * @param   *phone_mac_addr: pointer to phone_mac_addr.
	           Byte0~Byte5   The MAC address you want to connect to your phone
						 Byte6~Byte21  Link key
						 (phone MAC and Link key from NVRAM event,all zero is connect last pair phone)
             len: data length(must be 22 byte)
  * @retval TRUE:success
  *         FALSE:fail
  */
Boolean BT_ConnectBnep( uint8_t *phone_mac_addr,int len);

/**
  * @brief  disconnect Bnep
  * @param   none
  * @retval TRUE:success
  *         FALSE:fail
  */
Boolean BT_disconnectBnep(void);



/**
  * @brief  Connect Tcp
  * @param  tcp_ip_addr: pointer to  tcp_ip_addr.
	          Byte0  TCP connect handle(0x00 or 0x01)
						Byte1~Byte4  IP address(192.168.1.1 is c0 a8 01 01)
						Byte5~Byte6  port number(8888 is 22 b8)
            len: data length(must be 7 byte)
  * @retval TRUE:success
  *         FALSE:fail
  */
Boolean BT_ConnectTcp( uint8_t *tcp_ip_addr,int len);



/**
  * @brief  Send  TcpData
  * @param  tcpdata: pointer to  tcpdata.
	          Byte0  TCP handle
						Byte1~ByteN  The TCP data need to send(max 235 bytes)
            len: data length(max value 1+235)
  * @retval TRUE:success
  *         FALSE:fail
  */
Boolean BT_BnepSendTcpData( uint8_t *tcpdata,int len);


/**
  * @brief  Send  Tcp big packet Data
  * @param  tcpdata: pointer to  tcpdata.
	          Byte0  TCP handle
						Byte1~ByteN  The TCP data need to send(max 1460 bytes)
            len: data length(max value 1+1460)
  * @retval TRUE:success
  *         FALSE:fail
  */
Boolean BT_BnepSendTcpBigData( uint8_t *tcpdata,int len);


/**
  * @brief  Send  UDP Data
  * @param  tcpdata: pointer to  udpdata.
	          Byte0~Byte3  UDP remote IP(192.168.1.1 is c0 a8 01 01)
						Byte4~Byte5  UDP local port(8888 is 22 b8)
						Byte6~Byte7  UDP remote port(12345 is 30 39)
						Byte8~ByteN  The UDP data need to send(max 247 bytes)
            len: data length(max value 255)
  * @retval TRUE:success
  *         FALSE:fail
  */
Boolean BT_BnepSendUdpData( uint8_t *udpdata,int len);



/**
  * @brief  Disconnect  Tcp
  * @param  tcp_handle: the want to disconnect TCP handle(0x00 or 0x01)
  * @retval TRUE:success
  *         FALSE:fail
  */
Boolean BT_DisconnectTcp(uint8_t tcp_handle);







#endif

