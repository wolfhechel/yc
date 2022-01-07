#include "ota_ble.h"
#include "yc_bt.h"
#include "yc_qspi.h"

// 49535343-FE7d-4AE5-8FA9-9FAFD205E455
uint8_t ble_service_uuid_lsps[] = {0x10,0x55,0xe4,0x05,0xd2,0xaf,0x9f,0xa9,0x8f,0xe5,0x4a,0x7d,0xfe,0x43,0x53,0x53,0x49};

//49535343-1E4D-4BD9-BA61-23C647249616
uint8_t ble_Characteristic_uuid_lsps_tx[] = {0x10,0x10,0x16,0x96,0x24,0x47,0xc6,0x23,0x61,0xba,0xd9,0x4b,0x4d,0x1e,0x43,0x53,0x53,0x49,0x01,0x00};

//49535343-8841-43F4-A8D4-ECBE34729BB3
uint8_t ble_Characteristic_uuid_lsps_rx[] = {0x0c,0x10,0xb3,0x9b,0x72,0x34,0xbe,0xec,0xd4,0xa8,0xf4,0x43,0x41,0x88,0x43,0x53,0x53,0x49,0x01,0x00};

//49535343-aca3-481c-91ec-d85e28a60318
uint8_t ble_Characteristic_uuid_flow_ctrl[] = {0x18,0x10,0x18,0x03,0xa6,0x28,0x5e,0xd8,0xec,0x91,0x1c,0x48,0xa3,0xac,0x43,0x53,0x53,0x49,0x01,0x00};

uint16_t ble_send_handle = 0x2a;
uint8_t bt_cod[3] = {0x24,0x04,0x04};




volatile uint32_t CurOta2FlashSize = 0;
volatile uint32_t CurBuck_length   = 0;
volatile uint16_t PacketCurIndex   = 0;       //��ǰ���Ͱ����
volatile uint16_t PacketLastIndex  = 0;       //�ж��Ƿ��ش�
static   uint8_t  BUCK[BUCK_LEN]   = {0};       //��ŵ�ǰ�������������

BleotadataStr BleotaDatacheck;

extern uint32_t APPWRITEADDR;

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

void BT_IRQHandler(void)
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

void BleInit(void)
{
    uint8_t bt_name[15] = "YC3121BT";
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
		MyPrintf("SetBleName_failed\n");

	if(BT_SetBleAddr(bt_addr) == TRUE)
		MyPrintf("SetBleAddr_suc\n");
	else
		MyPrintf("SetBleAddr_fail\n");

	if(BT_SetParingMode(0x03) == TRUE)
		MyPrintf("set confirmkey mode success\n");
	else
		MyPrintf("set confirmkey mode failed\n");

	if(BT_SetCOD(bt_cod) == TRUE)
		MyPrintf("set COD sucess\n");
	else
		MyPrintf("set COD failed\n");

	if(BT_DeleteService() == TRUE)
		MyPrintf("delete service sucess\n");
	else
		MyPrintf("delete service failed\n");

	temp_handle=BT_AddBleService(ble_service_uuid_lsps,sizeof(ble_service_uuid_lsps));
	if( temp_handle!= 0)  
		MyPrintf("add service sucess,handle=%04x\n",temp_handle);
	else
		MyPrintf("add service failed,return=%04x\n",temp_handle);

	ble_send_handle=BT_AddBleCharacteristic(ble_Characteristic_uuid_lsps_tx,sizeof(ble_Characteristic_uuid_lsps_tx));
	if( ble_send_handle!= 0)  
		MyPrintf("add Characteristic tx sucess,handle=%04x\n",ble_send_handle);
	else
		MyPrintf("add Characteristic tx failed,return=%04x\n",ble_send_handle);

	temp_handle=BT_AddBleCharacteristic(ble_Characteristic_uuid_lsps_rx,sizeof(ble_Characteristic_uuid_lsps_rx));
	if( temp_handle!= 0)
		MyPrintf("add Characteristic rx sucess;handle=%04x\n",temp_handle);
	else
		MyPrintf("add Characteristic rx failed,return=%04x\n",temp_handle);

	temp_handle=BT_AddBleCharacteristic(ble_Characteristic_uuid_flow_ctrl,sizeof(ble_Characteristic_uuid_flow_ctrl));
	if( temp_handle!= 0)
		MyPrintf("add Characteristic flow_ctrl sucess;handle=%04x\n",temp_handle);
	else
		MyPrintf("add Characteristic flow_ctrl failed,return=%04x\n",temp_handle);
#if 0
	if(BT_SetVisibility(TRUE,TRUE,TRUE) == TRUE)  
#else
    if(BT_SetVisibility(FALSE,FALSE,TRUE) == TRUE)  
#endif
        MyPrintf("SetVisibility sucess\n");
	else
		MyPrintf("SetVisibility failed\n");

    MyPrintf("bt version=%x\n",BT_GetVersion());
}

#define IFLASH_NVRAM_ADDR 0x1000000+((512-4)*1024)
#define NVRAM_LEN 170//Nvram ����170bytes,ÿ��Nvram����Ϊ34bytes,���ɴ洢5�������Ϣ

static uint8_t bleData[MAX_BLUETOOTH_DATA_LEN]={0};
volatile static int32_t BleDataLen = 0;

uint8_t ReserveNV(uint8_t* nvram)
{
	MyPrintf("new nvram data,updata to flash\r\n");
	qspi_flash_sectorerase(IFLASH_NVRAM_ADDR);
	return qspi_flash_write(IFLASH_NVRAM_ADDR,nvram,NVRAM_LEN);
}

void BT_Progress(void)
{
	int eventCmd= BT_GetEventOpcode();
	switch(eventCmd)
	{
		case BT_DATA:
			{
				uint8_t btData[MAX_BLUETOOTH_DATA_LEN];
				int BtDataLen=0;
				BtDataLen=BT_ReadBTData(btData);
				if(BtDataLen>0)
				{
					MyPrintf("bt received %d bytes bt data:\n",BtDataLen);
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
            BleDataLen=BT_ReadBTData(bleData+2); //Ble�յ������浽buff��
            if(BleDataLen>0)
            {
                //add handle
                bleData[0]=(ble_send_handle&0xff);
                bleData[1]=((ble_send_handle>>8)&0xff);
                BleDataLen += 2;
            }
            else	MyPrintf("read ble data error\r\n");
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
					ReserveNV(NvramData);//�����µ�5���豸��Ϣ��NVRAM�� д��flash ,
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

				if(BT_ConfirmGkey(0) == TRUE)//BT_ConfirmGkey����Ϊ0�������ԣ�1��ȡ����ԡ�
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

//#define debug
void OtaData2FlashStart(void)
{
    enc_write_flash_bulk(APPWRITEADDR + CurOta2FlashSize, BUCK,BleotaDatacheck.Buck_length, 0);
    CurOta2FlashSize += BleotaDatacheck.Buck_length;
    CurBuck_length = BleotaDatacheck.Buck_length;
}

void OtaData2FlashEnd(void)
{
    qspi_flash_sectorerase(APPWRITEADDR + CurOta2FlashSize-CurBuck_length);
    enc_write_flash_bulk(APPWRITEADDR + CurOta2FlashSize-CurBuck_length, BUCK,CurBuck_length, 1);
}

void Ble_Cmd_check(void)
{
    uint16_t version = 0;
    uint16_t packet_maxlen = PACKET_MAXLEN;
    uint16_t buck_len = BUCK_LEN;
    uint8_t  blerecdatalen;
    uint8_t  updateflashmode;       //����normal �̼�
    uint32_t SumLength = 0;
    uint32_t CheckSum = 0;
    BleDataPackStr Dataretpack;     //���巵�����ݰ�
    uint8_t *data = bleData+4;
    if(bleData[0] == 0x2a && bleData[1] == 0x00)
    {
        Dataretpack.notifydata_1 = (ble_send_handle&0xff);
        Dataretpack.notifydata_2 = ((ble_send_handle>>8)&0xff);
        Dataretpack.Evt          = EVTRESPOND;
        Dataretpack.ResultCode   = RESULTTRUE;
        Dataretpack.RcvFlag      = data[1];
        if(data[0] == RECDATAPACKCMD)
        {
            switch (data[1])
            {
            case PORTOCOL_VERSION_REQUEST:
                version = BT_GetVersion();
                Dataretpack.Payload[0] = version&0xff;
                Dataretpack.Payload[1] = ((version>>8)&0xff);
                BT_SendBleData((uint8_t *)&Dataretpack,7);
                break;
            case BUCK_SIZE_REQUEST:
                memcpy(Dataretpack.Payload, &buck_len, 2);
                memcpy(Dataretpack.Payload + 2, &packet_maxlen, 2);
                BT_SendBleData((uint8_t *)&Dataretpack, 9);
                break;
            case START_REQUEST:
                updateflashmode = NORMALOTA;
                Dataretpack.Payload[0] = updateflashmode;
                BT_SendBleData((uint8_t *)&Dataretpack,6);
                break;
            case DATA_WRITE_CMD:
                PacketCurIndex = ((uint16_t)data[3]<<8|(uint16_t)data[2]);
                blerecdatalen = data[4];
                #ifdef debug
                MyPrintf("PacketCurIndex = 0x%04x\n",PacketCurIndex);
                MyPrintf("blerecdatalen = 0x%02x\n",blerecdatalen);
                #endif
                PacketLastIndex = PacketCurIndex-PacketLastIndex;
                if(PacketLastIndex != 0x00 && PacketLastIndex != 0x01)
                {
                    BleotaDatacheck.Buck_length = 0x00;
                    BleotaDatacheck.CheckSum    = 0x00;
                    BleotaDatacheck.Sum_length  = 0x00;
                }
                PacketLastIndex = PacketCurIndex;
                memcpy(BUCK + BleotaDatacheck.Buck_length, data + 5, blerecdatalen);
                BleotaDatacheck.Buck_length += blerecdatalen;
                for(uint8_t i = 0; i < blerecdatalen; i++)    //�ۼ����
                {
                    BleotaDatacheck.CheckSum += BUCK[BleotaDatacheck.Buck_length - blerecdatalen + i];
                }
                break;
            case DATA_WRITE_REQUEST:
                PacketCurIndex = ((uint16_t)data[3]<<8|(uint16_t)data[2]);
                blerecdatalen = data[4];
                #ifdef debug
                MyPrintf("PacketCurIndex = 0x%04x\n",PacketCurIndex);
                MyPrintf("blerecdatalen = 0x%02x\n",blerecdatalen);
                #endif
                PacketLastIndex = PacketCurIndex-PacketLastIndex;
                if(PacketLastIndex != 0x00 && PacketLastIndex != 0x01)
                {
                    BleotaDatacheck.Buck_length = 0x00;
                    BleotaDatacheck.CheckSum    = 0x00;
                    BleotaDatacheck.Sum_length  = 0x00;
                }
                PacketLastIndex = PacketCurIndex;
                memcpy(BUCK + BleotaDatacheck.Buck_length, data + 5, blerecdatalen);
                BleotaDatacheck.Buck_length += blerecdatalen;
                BleotaDatacheck.Sum_length += BleotaDatacheck.Buck_length;
                for(uint8_t i = 0; i < blerecdatalen; i++)    //�ۼ����
                {
                    BleotaDatacheck.CheckSum += BUCK[BleotaDatacheck.Buck_length - blerecdatalen + i];
                }
                /*���յ���BUCK���浽flash��*/
                OtaData2FlashStart();
                BleotaDatacheck.Buck_length = 0;
                BT_SendBleData((uint8_t *)&Dataretpack, 5);
                break;
            case END_REQUEST:
                /*���һ��BUCK����д��*/
                OtaData2FlashEnd();
                Dataretpack.Payload[0] = NORMALOTA;
                SumLength = ((uint32_t)data[3]|(uint32_t)data[4]<<8|(uint32_t)data[5]<<16|(uint32_t)data[6]<<24);
                CheckSum =  ((uint32_t)data[7]|(uint32_t)data[8]<<8|(uint32_t)data[9]<<16|(uint32_t)data[10]<<24);
                if(SumLength == BleotaDatacheck.Sum_length && CheckSum == BleotaDatacheck.CheckSum )
                {
                    #ifdef debug
                    MyPrintf("BLE OTA OK\n");
                    MyPrintf("SumLength = 0x%08x\n",SumLength);
                    MyPrintf("CheckSum = 0x%08x\n",CheckSum);
                    MyPrintf("BleotaDatacheck.Sum_length = 0x%08x\n",BleotaDatacheck.Sum_length);
                    MyPrintf("BleotaDatacheck.CheckSum = 0x%08x\n",BleotaDatacheck.CheckSum);
                    #endif
                    BleotaDatacheck.Back2AppFlag = 1;
                }
                memcpy(Dataretpack.Payload + 1, &BleotaDatacheck.Sum_length, 4);
                memcpy(Dataretpack.Payload + 5, &BleotaDatacheck.CheckSum, 4);
                BT_SendBleData((uint8_t *)&Dataretpack, 14);
                break;
            default:
                break;
            }
        }
        /*�������buf*/
        memset(bleData,0,255);
    }
}

uint8_t BleOtaUpdateStatus(void)
{
    if(BleotaDatacheck.Back2AppFlag == 1)
    {
        BleotaDatacheck.Back2AppFlag = 0;
        return TRUE;
    }
    else
        return FALSE;
}

