
#ifndef _OTA_BLE_H_
#define _OTA_BLE_H_

#include "yc_encflash_bulk.h"
#include "type.h"

#define PORTOCOL_VERSION_REQUEST    0x10       
#define BUCK_SIZE_REQUEST           0x11       
#define START_REQUEST               0x15       
#define DATA_WRITE_CMD              0x16       
#define DATA_WRITE_REQUEST          0x17       
#define END_REQUEST                 0x18       

#define RESULTTRUE                  0x00
#define RESULTERROR                 0x01
#define EVTRESPOND                  0xAB
#define RECDATAPACKCMD              0xBA

#define NORMALOTA                   0x00

#define PACKET_MAXLEN               0x85  //MAX = 193
#define BUCK_LEN                    0x1000

typedef struct
{
    uint8_t notifydata_1;
    uint8_t notifydata_2;
    uint8_t Evt;
    uint8_t ResultCode;
    uint8_t RcvFlag;
    uint8_t Payload[32];
}BleDataPackStr;

typedef struct
{
    uint16_t Buck_length;       
    int Sum_length;             
    int CheckSum;               
    uint8_t  Back2AppFlag;      
}BleotadataStr;

void BleInit(void);
void BT_Progress(void);
void Ble_Cmd_check(void);
void BT_IRQHandler(void);
uint8_t BleOtaUpdateStatus(void);

#endif
