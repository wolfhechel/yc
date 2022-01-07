#include "yc_bt.h"
#include "yc_systick.h"
#include "yc_timer.h"
#include "yc_ipc.h"

#define BT_LOG  0
#define     BT_ERROR_LOG



static tick StartTick;
static uint32_t HCI_CMD_BIT_FLAG[2] = {0};
#define BTTIMEOUT 500
#define BT_BUFSIZE 2048
uint8_t rcvbuf[BT_BUFSIZE] = {0};
extern uint8_t ipcrecbuf[IPCREVSIZE];

static BT_BufTypeDef BluetoothBuf;
static uint8_t CmdRepData[5];
static int PacketNum = 0;

extern IPC_TypeDef *IpcTx;
extern IPC_TypeDef *IpcRx;
extern uint8_t bt_lpm_mode;

HCI_TypeDef hci_bt;

void BT_BIT_SET(uint32_t *a, int b)
{
    if (b < 32)
    {
        a[0] |= (1 << b);
    }
    else
    {
        a[1] |= (1 << (b - 32));
    }
}

void BT_BIT_CLEAR(uint32_t *a, int b)
{
    if (b < 32)
    {
        a[0] &= ~(1 << b);
    }
    else
    {
        a[1] &= ~(1 << (b - 32));
    }
}

Boolean BT_BIT_GET(uint32_t *a, int b)
{
    if (b < 32)
    {
        return (Boolean)((a[0] & (1 << b)) >> b);
    }
    else
    {
        return (Boolean)((a[1] & (1 << (b - 32))) >> (b - 32));
    }
}

static void SkipData(int Num)
{
    BluetoothBuf.ReadIndex += Num;
    if (BluetoothBuf.ReadIndex >= BluetoothBuf.BufSize)    BluetoothBuf.ReadIndex -= BluetoothBuf.BufSize;
    NVIC_DisableIRQ(BT_IRQn);
    BluetoothBuf.count -= Num;
    NVIC_EnableIRQ(BT_IRQn);
}

static void SaveData(uint8_t *data, int len)
{
    int i;
    if (BluetoothBuf.count + len > BluetoothBuf.BufSize)
    {
#ifdef BT_ERROR_LOG
        MyPrintf("BluetoothBuf full\r\n");
#endif
        return;
    }
    for (i = 0; i < len; i++)
    {
        BluetoothBuf.pBuf[BluetoothBuf.WriteIndex] = data[i];
        BluetoothBuf.count++;
        BluetoothBuf.WriteIndex++;
        if (BluetoothBuf.WriteIndex >= BluetoothBuf.BufSize)   BluetoothBuf.WriteIndex = 0;
    }
}

//#define PrintLog
#define DOMAIN_MAX_LEN  62
Boolean BNEP_DomainConvert(unsigned char *domainString, unsigned char domainStringLen, unsigned char *newDomainString, int *newDomainStringIndex)
{
#ifdef PrintLog
    MyPrintf("input domain:%s \n", domainString);
#endif
    unsigned char domainCount = 0;
    *newDomainStringIndex = 1;
    int newDomainDelimiterIndex = 0;
#ifdef PrintLog
    MyPrintf("domain hex:\n");
    for (int i = 0; i < domainStringLen; i++)
    {
        MyPrintf("%02X ", domainString[i]);
    }
    MyPrintf("\n");
#endif
    while (domainStringLen--)
    {
        domainCount++;
        if (*domainString == '.')
        {
            newDomainString[newDomainDelimiterIndex] = domainCount - 1;
            newDomainDelimiterIndex = *newDomainStringIndex;
            (*newDomainStringIndex)++;
            if ((*newDomainStringIndex) >= DOMAIN_MAX_LEN) return FALSE;
            domainCount = 0;
        }
        else
        {
            newDomainString[(*newDomainStringIndex)++] = *domainString;
            if ((*newDomainStringIndex) >= DOMAIN_MAX_LEN) return FALSE;
        }
        domainString++;
    }
    newDomainString[newDomainDelimiterIndex] = domainCount;
    newDomainString[(*newDomainStringIndex)++] = 00;
    if ((*newDomainStringIndex) >= DOMAIN_MAX_LEN) return FALSE;
#ifdef PrintLog
    MyPrintf("format Domain:\n");
    for (int  i = 0; i < *newDomainStringIndex; i++)
    {
        MyPrintf("%02X ", newDomainString[i]);
    }
    MyPrintf("\n");
#endif
    return TRUE;
}

Boolean BT_Init()
{
    BluetoothBuf.BufSize =  BT_BUFSIZE;
    BluetoothBuf.pBuf = rcvbuf;
    BluetoothBuf.count = 0;
    BluetoothBuf.ReadIndex = 0;
    BluetoothBuf.WriteIndex = 0;
    HCI_CMD_BIT_FLAG[0] = 0; //全局变量问题优化
    HCI_CMD_BIT_FLAG[1] = 0;
    StartTick = 0;
    PacketNum = 0;
    int i;
    for (i = 0; i < 5; i++)    CmdRepData[i] = 0;
    for (i = 0; i < BT_BUFSIZE; i++)   rcvbuf[i] = 0;
    hci_bt.p_data =  ipcrecbuf;
    IpcInit();
#ifdef BT_ERROR_LOG
    MyPrintf("BT_Init end\n");
#endif
    return TRUE;
}

Boolean BT_WakeupInit()
{
    BluetoothBuf.BufSize =  BT_BUFSIZE;
    BluetoothBuf.pBuf = rcvbuf;
    BluetoothBuf.count = 0;
    BluetoothBuf.ReadIndex = 0;
    BluetoothBuf.WriteIndex = 0;
    hci_bt.p_data =  ipcrecbuf;
    enable_intr(INTR_BT);
    bt_lpm_mode = 1;
    BT_SetLpmMode(0x00);
#ifdef BT_ERROR_LOG
    MyPrintf("BT_Wakeup Init end\n");
#endif
    return TRUE;
}

static uint8_t GetData(Boolean UpdataAnalyzeIndex)
{
    uint8_t TempData;
    TempData = BluetoothBuf.pBuf[BluetoothBuf.ReadIndex];
    if (UpdataAnalyzeIndex)
    {
        NVIC_DisableIRQ(BT_IRQn);
        BluetoothBuf.count--;
        NVIC_EnableIRQ(BT_IRQn);
        BluetoothBuf.ReadIndex++;
        if (BluetoothBuf.ReadIndex >= BluetoothBuf.BufSize)    BluetoothBuf.ReadIndex = 0;
    }
    return TempData;
}


static Boolean  SendCMD(HCI_TypeDef *msg)
{
    if ((msg->DataLen + 3) > IPC_get_available_size())
    {
#ifdef BT_ERROR_LOG
        MyPrintf("IPC BT Tx FULL\n");
#endif
        return FALSE;
    }
    msg->type = HCI_CMD;
#if BT_LOG
    MyPrintf("Print Send Data Pack:\n");
    PrintHCIPack(msg, "Send");//打印发送包
#endif
    IPC_TxData(msg);
    return TRUE;
}
Boolean BT_SetBtAddr(uint8_t *bt_addr)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_SET_BT_ADDR;
    msg.DataLen = 6;
    msg.p_data = bt_addr;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_SET_BT_ADDR);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_SET_BT_ADDR))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT)); //ms
    return FALSE;
}


Boolean BT_SetBleAddr(uint8_t *bt_addr)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_SET_BLE_ADDR;
    msg.DataLen = 6;
    msg.p_data = bt_addr;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_SET_BLE_ADDR);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_SET_BLE_ADDR))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT)); //ms
    return FALSE;
}

Boolean BT_SetVisibility(Boolean bt_discoverable, Boolean bt_connectability, Boolean ble_discoverable)
{
    HCI_TypeDef msg;
    uint8_t visibility_data = 0;
    if (bt_discoverable == 1)
        visibility_data = visibility_data | 0x01;
    if (bt_connectability == 1)
        visibility_data = visibility_data | 0x02;
    if (ble_discoverable == 1)
        visibility_data = visibility_data | 0x04;

    msg.opcode = HCI_CMD_SET_VISIBILITY;
    msg.DataLen = 0x01;
    msg.p_data = &visibility_data;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_SET_VISIBILITY);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_SET_VISIBILITY))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}

Boolean BT_SetBtName(uint8_t *bt_name, uint16_t name_len)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_SET_BT_NAME;
    msg.DataLen = name_len;
    msg.p_data  = bt_name;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_SET_BT_NAME);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_SET_BT_NAME))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}


Boolean BT_SetBleName(uint8_t *ble_name, uint16_t name_len)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_SET_BLE_NAME;
    msg.DataLen = name_len;
    msg.p_data  = ble_name;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_SET_BLE_NAME);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_SET_BLE_NAME))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}


Boolean BT_SendSppData(uint8_t *spp_data, uint16_t DataLen)
{
    if (DataLen == 0 || DataLen > MAX_BLUETOOTH_DATA_LEN)  return FALSE;
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_SEND_SPP_DATA;
    msg.DataLen = DataLen;
    msg.p_data = spp_data;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_SEND_SPP_DATA);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_SEND_SPP_DATA))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}

Boolean BT_SendBleData(uint8_t *ble_data, uint16_t DataLen)
{
    if (DataLen < 2 || DataLen > MAX_BLUETOOTH_DATA_LEN)   return FALSE;
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_SEND_BLE_DATA;
    msg.DataLen = DataLen;

    msg.p_data = ble_data;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_SEND_BLE_DATA);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_SEND_BLE_DATA))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}


uint8_t Bt_GetBtStatus(void)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_STATUS_REQUEST;
    msg.DataLen = 0;
    msg.p_data = NULL;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_STATUS_REQUEST);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_STATUS_REQUEST))
        {
            return CmdRepData[0];
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT)); //ms
    return 0x80;
}

Boolean BT_SetParingMode(uint8_t mode)
{
    uint8_t tmode = mode ;
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_SET_PAIRING_MODE;
    msg.DataLen = 1;
    msg.p_data = &tmode;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_SET_PAIRING_MODE);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_SET_PAIRING_MODE))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}

Boolean BT_SetPincode(uint8_t *Pincode, uint8_t DataLen)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_SET_PINCODE;
    msg.DataLen = DataLen;
    msg.p_data = Pincode;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_SET_PINCODE);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_SET_PINCODE))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}



Boolean BT_BtDisconnect(void)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_BT_DISCONNECT;
    msg.DataLen = 0;
    msg.p_data = NULL;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_BT_DISCONNECT);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_BT_DISCONNECT))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}

uint32_t BT_GetVersion(void)
{
    HCI_TypeDef msg;
    uint32_t TempVersion = 0;
    msg.opcode = HCI_CMD_VERSION_REQUEST;
    msg.DataLen = 0;
    msg.p_data = NULL;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_GET_VERSION);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_GET_VERSION))
        {
            TempVersion = (CmdRepData[0] << 8) | (CmdRepData[1]);
            return TempVersion;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return 0;
}


Boolean BT_BleDisconnect(void)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_BLE_DISCONNECT;
    msg.DataLen = 0;
    msg.p_data = NULL;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_BLE_DISCONNECT);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_BLE_DISCONNECT))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}


Boolean BT_SetNVRAM(uint8_t *NvData, int len)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_SET_NVRAM;
    msg.DataLen = len;
    msg.p_data = NvData;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_SET_NVRAM);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_SET_NVRAM))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}


Boolean BT_EnterSleepMode(void)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_ENTER_SLEEP_MODE;
    msg.DataLen = 0;
    msg.p_data = NULL;
    SendCMD(&msg);
    return TRUE;
}

Boolean BT_SetLpmMode(uint8_t mode)
{
    HCI_TypeDef msg;
    uint8_t temp_mode = mode;
    msg.opcode = HCI_CMD_SET_LPM_MODE;
    msg.DataLen = 1;
    msg.p_data = &temp_mode;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_SET_LPM_MODE);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_SET_LPM_MODE))
        {
            bt_lpm_mode = mode;
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}


Boolean BT_ConfirmGkey(uint8_t isMatching)
{
    HCI_TypeDef msg;
    uint8_t tmismatching = isMatching;
    msg.opcode = HCI_CMD_CONFIRM_GKEY;
    msg.DataLen = 1;
    msg.p_data = &tmismatching;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_CONFIRM_GKEY);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_CONFIRM_GKEY))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}

Boolean BT_SetSppFlowcontrol(uint8_t packetNum)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_GET_CREADIT_GIVEN;
    msg.DataLen = 1;
    uint8_t temp_packetNum = packetNum;
    msg.p_data = &temp_packetNum;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_GET_CREADIT_GIVEN);
    SendCMD(&msg);
    return TRUE;//no event responce
}


Boolean BT_PasskeyEntry(uint8_t *key_data)
{
#define PKLEN 0x04
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_PASSKEY_ENTRY;
    msg.DataLen = PKLEN;
    msg.p_data = key_data;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_PASSKEY_ENTRY);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_PASSKEY_ENTRY))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}

Boolean BT_SetLEParing(uint8_t mode)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_LE_SET_PAIRING;
    msg.DataLen = 1;
    uint8_t temp_mode = mode;
    msg.p_data = &temp_mode;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_LE_SET_PARING);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_LE_SET_PARING))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}

Boolean BT_SetLEAdvData(uint8_t *adv_data, int DataLen)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_LE_SET_ADV_DATA;
    msg.DataLen = DataLen;
    msg.p_data = adv_data;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_LE_SET_ADV_DATA);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_LE_SET_ADV_DATA))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}

Boolean BT_SetLEScanData(uint8_t *scan_data, int DataLen)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_LE_SET_SCAN_DATA;
    msg.DataLen = DataLen;
    msg.p_data = scan_data;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_LE_SET_SCAN_DATA);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_LE_SET_SCAN_DATA))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}

Boolean BT_SetLESendConnUpdate(uint8_t *data, int len)
{
    if (0x08 != len)
        return FALSE;
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_LE_SEND_CONN_UPDATE_REQ;
    msg.DataLen = len;
    msg.p_data = data;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_LE_SEND_CONN_UPDATE);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_LE_SEND_CONN_UPDATE))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}

Boolean BT_SetLEAdvInterval(uint16_t ble_adv_interval)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_LE_SET_ADV_PARM;
    msg.DataLen = 2;
    uint8_t ble_adv_interval_buf[2];
    ble_adv_interval_buf[0] = (ble_adv_interval & 0xff);
    ble_adv_interval_buf[1] = ((ble_adv_interval >> 8) & 0xff);
    msg.p_data = ble_adv_interval_buf;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_LE_SET_ADV_PARM);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_LE_SET_ADV_PARM))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}



Boolean BT_RejectJustWork(uint8_t justwork)
{
    uint8_t td = justwork;
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_REJECT_JUSTWORK;
    msg.DataLen = 0x01;
    msg.p_data = &td;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_REJECT_JUSTWORK);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_REJECT_JUSTWORK))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}

Boolean BT_SetTxPower(uint8_t power)
{
    uint8_t tpower = power;
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_SET_TX_POWER;
    msg.DataLen = 0x01;
    msg.p_data = &tpower;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_SET_TXPOWER);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_SET_TXPOWER))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}

Boolean BT_Set_FixedPasskey(uint8_t *key)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_LE_SET_FIXED_PASSKEY;
    msg.DataLen = 0x05;
    msg.p_data = key;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_LE_SET_FIXED_PASSKEY);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_LE_SET_FIXED_PASSKEY))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}

Boolean BT_SetCOD(uint8_t *bt_cod)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_SET_COD;
    msg.DataLen = 3;
    msg.p_data = bt_cod;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_SET_COD);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_SET_COD))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT)); //ms
    return FALSE;
}


Boolean BT_DeleteService(void)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_DELETE_CUSTOMIZE_SERVICE;
    msg.DataLen = 0;
    msg.p_data = NULL;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_DELETE_SERVICE);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_DELETE_SERVICE))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return FALSE;
}

uint16_t BT_AddBleService(uint8_t *ble_service_uuid, uint16_t service_uuid_len)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_ADD_SERVICE_UUID;
    msg.DataLen = service_uuid_len;
    msg.p_data  = ble_service_uuid;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_ADD_UUID);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_ADD_UUID))
        {
            return (uint16_t)(CmdRepData[0] | (CmdRepData[1] << 8));
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return 0x00;
}




uint16_t BT_AddBleCharacteristic(uint8_t *ble_Characteristic_uuid, uint16_t service_Characteristic_payload_len)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_ADD_CHARACTERISTIC_UUID;
    msg.DataLen = service_Characteristic_payload_len;
    msg.p_data  = ble_Characteristic_uuid;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_ADD_UUID);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_ADD_UUID))
        {
            return (uint16_t)(CmdRepData[0] | (CmdRepData[1] << 8));
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT));
    return 0x00;
}

Boolean BT_DnsReq(uint8_t *dns_data, int len)
{
    uint8_t format_dns[DOMAIN_MAX_LEN];
    int format_dns_len;
    if (TRUE != BNEP_DomainConvert(dns_data, len, format_dns, &format_dns_len))
    {
        return FALSE;
    }
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_DNS_REQ;
    msg.DataLen = format_dns_len;
    msg.p_data = format_dns;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_DNS_REQ);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_DNS_REQ))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT)); //ms
    return FALSE;
}

Boolean BT_ConnectBnep(uint8_t *phone_mac_addr, int len)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_CONNECT_BNEP;
    msg.DataLen = len;
    msg.p_data = phone_mac_addr;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_CONNECT_BNEP);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_CONNECT_BNEP))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT)); //ms
    return FALSE;
}

Boolean BT_disconnectBnep()
{
    return BT_BtDisconnect();
}

Boolean BT_ConnectTcp(uint8_t *tcp_ip_addr, int len)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_CONNECT_TCP;
    msg.DataLen = len;
    msg.p_data = tcp_ip_addr;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_CONNECT_TCP);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_CONNECT_TCP))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT)); //ms
    return FALSE;
}



Boolean BT_BnepSendTcpData(uint8_t *tcpdata, int len)
{
    if ((len - 1) > MAX_TCP_DATA_LEN)    return FALSE;
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_BNEP_SEND_TCP_DATA;
    msg.DataLen = len;
    msg.p_data = tcpdata;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_TCP_DATA);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_TCP_DATA))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT)); //ms
    return FALSE;
}

#define BIG_DATA_HEAD_LEN   5
Boolean BT_BnepSendTcpBigData(uint8_t *tcpdata, int len)
{
    if ((len - 1) > MAX_BIG_DATA_LEN)    return FALSE;
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_SEND_BIG_DATA;
    int sendedLen = 0;
    int packetLen;
    uint8_t firstPacketData[MAX_BLUETOOTH_DATA_LEN];
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_TCP_BIG_DATA_FINISH);
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_TCP_BIG_DATA_STATUS);
    while (sendedLen < (len - 1))
    {
        if (sendedLen == 0)
        {
            packetLen = (len - 1) > (MAX_BLUETOOTH_DATA_LEN - BIG_DATA_HEAD_LEN) ? (MAX_BLUETOOTH_DATA_LEN - BIG_DATA_HEAD_LEN) : (len - 1);
            msg.DataLen = packetLen + BIG_DATA_HEAD_LEN;
            firstPacketData[0] = tcpdata[0]; //handle

            firstPacketData[1] = ((len - 1) & 0xff);
            firstPacketData[2] = (((len - 1) >> 8) & 0xff); //big packet len

            firstPacketData[3] = 0;
            firstPacketData[4] = 0; //reserved
            memcpy((uint8_t *)&firstPacketData[5], (uint8_t *)&tcpdata[1], packetLen);
            msg.p_data = firstPacketData;
        }
        else
        {
            packetLen = ((len - 1) - sendedLen) > MAX_BLUETOOTH_DATA_LEN ? MAX_BLUETOOTH_DATA_LEN : ((len - 1) - sendedLen);
            msg.DataLen = packetLen;
            msg.p_data = (uint8_t *)&tcpdata[sendedLen + 1];
        }

        BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_TCP_BIG_DATA);
        SendCMD(&msg);
        StartTick = SysTick_GetTick();
        while (!BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_TCP_BIG_DATA))
        {
            if (SysTick_IsTimeOut(StartTick, BTTIMEOUT))
            {
                return FALSE;
            }
        }
        sendedLen += packetLen;
    }
    //wait big data finish
    StartTick = SysTick_GetTick();
    while (!BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_TCP_BIG_DATA_FINISH))
    {
        if (SysTick_IsTimeOut(StartTick, BTTIMEOUT))
        {
            return FALSE;
        }
    }
    //wait big data status
    StartTick = SysTick_GetTick();
    while (!BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_TCP_BIG_DATA_STATUS))
    {
        if (SysTick_IsTimeOut(StartTick, BTTIMEOUT))
        {
            return FALSE;
        }
    }
    return TRUE;
}


Boolean BT_BnepSendUdpData(uint8_t *udpdata, int len)
{
    if (len < 8 || len > MAX_BLUETOOTH_DATA_LEN)   return FALSE;
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_SEND_UDP_DATA;
    msg.DataLen = len;
    msg.p_data = udpdata;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_UDP_DATA);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_UDP_DATA))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT)); //ms
    return FALSE;
}


Boolean BT_DisconnectTcp(uint8_t tcp_handle)
{
    HCI_TypeDef msg;
    msg.opcode = HCI_CMD_DISCONNECT_TCP;
    msg.DataLen = 1;
    uint8_t temp_tcp_handle = tcp_handle;
    msg.p_data = &temp_tcp_handle;
    BT_BIT_CLEAR(HCI_CMD_BIT_FLAG, BIT_DISCONNECT_TCP);
    SendCMD(&msg);
    StartTick = SysTick_GetTick();
    do
    {
        if (BT_BIT_GET(HCI_CMD_BIT_FLAG, BIT_DISCONNECT_TCP))
        {
            return TRUE;
        }
    }
    while (!SysTick_IsTimeOut(StartTick, BTTIMEOUT)); //ms
    return FALSE;
}

void BT_ParseBTData()
{
    //此判断用于防止接收包buf满
    if (BluetoothBuf.BufSize - BluetoothBuf.count < 258)
    {
#if BT_LOG
        MyPrintf("BluetoothBuf full2");
        MyPrintf("BluetoothBuf.BufSize: %d,BluetoothBuf.count: %d\n", BluetoothBuf.BufSize, BluetoothBuf.count);
#endif
        return;
    }

    if (TRUE == IPC_ReadBtData(&hci_bt)) //The packet header has been receive
    {
#ifdef SDK_DEBUG
        //  PrintHCIPack(&hci_bt, "Receive");
#endif
        if (hci_bt.type == HCI_EVENT)
        {

            StartTick = SysTick_GetTick(); //处理tx被rx阻塞时的timeout
            switch (hci_bt.opcode)
            {
            case HCI_EVENT_SPP_DATA_RECEIVED:
//                  SaveData((uint8_t *)&hci_bt,hci_bt.DataLen+3);
                SaveData((uint8_t *)&hci_bt, 3);
                SaveData(hci_bt.p_data, hci_bt.DataLen);
                PacketNum++;
                break;

            case HCI_EVENT_BLE_DATA_RECEIVED:
//                  SaveData((uint8_t *)&hci_bt,hci_bt.DataLen+3);
                SaveData((uint8_t *)&hci_bt, 3);
                SaveData(hci_bt.p_data, hci_bt.DataLen);
                PacketNum++;
                break;

            case HCI_EVENT_BNEP_RECEIVE_TCP_DATA:
//                  SaveData((uint8_t *)&hci_bt,hci_bt.DataLen+3);
                SaveData((uint8_t *)&hci_bt, 3);
                SaveData(hci_bt.p_data, hci_bt.DataLen);
                PacketNum++;
                break;

            case HCI_EVENT_BNEP_RECEIVE_UDP_DATA:
//                  SaveData((uint8_t *)&hci_bt,hci_bt.DataLen+3);
                SaveData((uint8_t *)&hci_bt, 3);
                SaveData(hci_bt.p_data, hci_bt.DataLen);
                PacketNum++;
                break;

            case HCI_EVENT_BT_CONNECTED:
            case HCI_EVENT_BLE_CONNECTED:
            case HCI_EVENT_GKEY:
            case HCI_EVENT_BT_DISCONNECTED:
            case HCI_EVENT_BLE_DISCONNECTED:
            case HCI_EVENT_NVRAM_CHANGED:
            case HCI_EVENT_GET_PASSKEY:
            case HCI_EVENT_PAIRING_COMPLETED:
            case HCI_EVENT_LE_PAIRING_STATE:
            case HCI_EVENT_BNEP_CONNECT:
            case HCI_EVENT_BNEP_DISCONNECT:
            case HCI_EVENT_BNEP_CONNECT_FAIL:
            case HCI_EVENT_TCP_CONNECT:
            case HCI_EVENT_TCP_DISCONNECT:
            case HCI_EVNET_TCP_FAIL:
            case HCI_EVENT_TCP_CONNECT_LOG:
            case HCI_EVENT_BNEP_CHIP_ERROR:
            case HCI_EVENT_DNS_RSP:
            case HCI_EVENT_BB_DISCONNECT:
            case HCI_EVENT_BT_NAME:
            case HCI_EVENT_UDP_INFO:


                //      SaveData((uint8_t *)&hci_bt,hci_bt.DataLen+3);
                SaveData((uint8_t *)&hci_bt, 3);
                SaveData(hci_bt.p_data, hci_bt.DataLen);
                PacketNum++;
                break;

            case HCI_EVENT_CMD_COMPLETE:

                switch (hci_bt.p_data[0])
                {
                case HCI_CMD_SET_BT_ADDR:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_SET_BT_ADDR);
                    }
                    break;

                case HCI_CMD_SET_BLE_ADDR:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_SET_BLE_ADDR);
                    }
                    break;

                case HCI_CMD_SET_VISIBILITY:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_SET_VISIBILITY);
                    }
                    break;

                case HCI_CMD_SET_BT_NAME:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_SET_BT_NAME);
                    }
                    break;
                case HCI_CMD_SET_BLE_NAME:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_SET_BLE_NAME);
                    }
                    break;

                case HCI_CMD_SEND_SPP_DATA:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_SEND_SPP_DATA);
                    }
                    break;

                case HCI_CMD_SEND_BLE_DATA:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_SEND_BLE_DATA);
                    }
                    break;

                case HCI_CMD_SET_PAIRING_MODE:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_SET_PAIRING_MODE);
                    }
                    break;

                case HCI_CMD_SET_PINCODE:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_SET_PINCODE);
                    }
                    break;

                case HCI_CMD_VERSION_REQUEST:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_GET_VERSION);
                        CmdRepData[0] = hci_bt.p_data[2];
                        CmdRepData[1] = hci_bt.p_data[3];
                    }
                    break;

                case HCI_CMD_LE_SET_PAIRING:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_LE_SET_PARING);
                    }
                    break;

                case HCI_CMD_BT_DISCONNECT:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_BT_DISCONNECT);
                    }
                    break;

                case HCI_CMD_BLE_DISCONNECT:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_BLE_DISCONNECT);
                    }
                    break;
                case HCI_CMD_SET_NVRAM:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_SET_NVRAM);
                    }
                    break;

                case HCI_CMD_CONFIRM_GKEY:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_CONFIRM_GKEY);
                    }
                    break;

                case HCI_CMD_SET_LPM_MODE:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_SET_LPM_MODE);
                    }
                    break;

                case HCI_CMD_PASSKEY_ENTRY:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_PASSKEY_ENTRY);
                    }
                    break;

                case HCI_CMD_LE_SET_ADV_DATA:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_LE_SET_ADV_DATA);
                    }
                    break;

                case HCI_CMD_LE_SET_SCAN_DATA:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_LE_SET_SCAN_DATA);
                    }
                    break;

                case HCI_CMD_LE_SEND_CONN_UPDATE_REQ:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_LE_SEND_CONN_UPDATE);
                    }
                    break;
                case HCI_CMD_LE_SET_ADV_PARM:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_LE_SET_ADV_PARM);
                    }
                    break;

                case HCI_CMD_SET_TX_POWER:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_SET_TXPOWER);
                    }
                    break;

                case HCI_CMD_REJECT_JUSTWORK:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_REJECT_JUSTWORK);
                    }
                    break;

                case HCI_CMD_SET_COD:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_SET_COD);
                    }
                    break;

                case HCI_CMD_DELETE_CUSTOMIZE_SERVICE:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_DELETE_SERVICE);
                    }
                    break;

                case HCI_CMD_LE_SET_FIXED_PASSKEY:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_LE_SET_FIXED_PASSKEY);
                    }
                    break;

                case HCI_CMD_CONNECT_BNEP:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_CONNECT_BNEP);
                    }
                    break;

                case HCI_CMD_CONNECT_TCP:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_CONNECT_TCP);
                    }
                    break;

                case HCI_CMD_DISCONNECT_TCP:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_DISCONNECT_TCP);
                    }
                    break;

                case HCI_CMD_BNEP_SEND_TCP_DATA:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_TCP_DATA);
                    }
                    break;

                case HCI_CMD_SEND_UDP_DATA:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_UDP_DATA);
                    }
                    break;

                case HCI_CMD_SEND_BIG_DATA:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_TCP_BIG_DATA);
                    }
                    break;

                case HCI_CMD_DNS_REQ:
                    if (!hci_bt.p_data[1])
                    {
                        BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_DNS_REQ);
                    }
                    break;

                default:
#ifdef BT_ERROR_LOG
                    MyPrintf("bt 06 cmd err\n");
#endif
                    break;
                }
                break;

            case HCI_EVENT_I_AM_READY:
                BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_MODULE_READY);
                break;

            case HCI_EVENT_STAUS_RESPONSE:
                BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_STATUS_REQUEST);
                CmdRepData[0] = hci_bt.p_data[0];
                break;

            case HCI_EVENT_UUID_HANDLE:
                BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_ADD_UUID);
                CmdRepData[0] = hci_bt.p_data[0];
                CmdRepData[1] = hci_bt.p_data[1];
                break;

            case HCI_EVENT_BIG_DATA_FINISH:
                if (hci_bt.p_data[0] == 0x00)
                {
                    BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_TCP_BIG_DATA_FINISH);
                }
                break;

            case HCI_EVENT_BIG_DATA_STATUS:
                if (hci_bt.p_data[0] == 0x03 || hci_bt.p_data[0] == 0x04)
                {
                    BT_BIT_SET(HCI_CMD_BIT_FLAG, BIT_BNEP_SEND_TCP_BIG_DATA_STATUS);
                }
                break;

            default:
#ifdef BT_ERROR_LOG
                MyPrintf("bt event err");
#endif
                break;
            }
            IPC_HOLD_BT = 0;
            WAKEUP_BT &= ~(1 << WAKEUP_BT_FLAG);
        }
        else
        {
#ifdef BT_ERROR_LOG
            MyPrintf("bt 02 err");
#endif
        }
    }
    else
    {
#ifdef BT_ERROR_LOG
        MyPrintf("receive bt data err");
#endif
    }
}

int BT_GetEventOpcode()
{
    if (PacketNum <= 0)    return -1;
    uint8_t TempOpcode;
    int TempIndex = BluetoothBuf.ReadIndex + 1;
    if (TempIndex == BluetoothBuf.BufSize) TempIndex = 0;
    TempOpcode = BluetoothBuf.pBuf[TempIndex];

    /*这几个事件用户不用取数据，所以跳过数据包，维护读指针，
    其他需要用户获取数据的事件则在BT_ReadBTData中维护指针*/
    if (TempOpcode == BT_CONNECTED || \
            TempOpcode == BLE_CONNECTED || \
            TempOpcode == BT_DISCONNECTED || \
            TempOpcode == BLE_DISCONNECTED || \
            TempOpcode == BNEP_CONNECT || \
            TempOpcode == BNEP_DISCONNECT || \
            TempOpcode == BNEP_CHIP_ERROR || \
            TempOpcode == HCI_EVENT_BB_DISCONNECT || \
            TempOpcode == PASSKEY)
    {
        SkipData(3);
    }
    NVIC_DisableIRQ(BT_IRQn);
    PacketNum--;
    NVIC_EnableIRQ(BT_IRQn);
    return TempOpcode;
}



int BT_ReadBTData(uint8_t *pbuf)
{
    uint8_t OpCode;
    int len = 0;
    int i = 0;
    if (BluetoothBuf.count < 3)    return 0;
    SkipData(1);
    OpCode = GetData(TRUE);
    len = GetData(TRUE);
    if (OpCode == BLE_DATA)
    {
//      len-=2;//ble handle 处理
//      SkipData(2);
    }
    for (i = 0; i < len; i++)
    {
        pbuf[i] = GetData(TRUE);
    }
    return len;
}


