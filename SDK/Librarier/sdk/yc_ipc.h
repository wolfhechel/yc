#ifndef __YC_IPC_H__
#define __YC_IPC_H__

#include "yc3121.h"
#include "yc_timer.h"
#include "bt_code.h"
#include "misc.h"
#include "yc_lpm.h"

#define IPCREVSIZE                 256
#define HW_REG_8BIT(reg, value)    (*((volatile uint8_t *)((uint32_t)reg)) = value)
#define HR_REG_8BIT(reg)           (*(volatile uint8_t *)((uint32_t)reg))
#define HR_REG_16BIT(reg)          ((uint16_t)(HR_REG_8BIT(reg) |((HR_REG_8BIT(reg+1)<<8)&0xff00)))
#define HW_REG_16BIT(reg,value)    do{HW_REG_8BIT(reg,value&0x00ff);HW_REG_8BIT(reg + 1,value>>8);}while(0)
#define HR_IPC_REG_8BIT(reg)       HR_REG_8BIT(reg|M0_BASE_ADDR)
#define HW_IPC_REG_8BIT(reg,value) HW_REG_8BIT(reg|M0_BASE_ADDR,value)

#define RB_UPDATE_PTR(p,s,e)       ((p) == (e))?((p)=(s)):((p)++)
#define M0_BASE_ADDR               0xC0000

#define MEM_SPP_FLOWCTRL_FLAG_ADDR    (0xc453d)
#define MEM_SPP_FLOWCTRL_FLAG_VALUE   (0x01)	//0x00:disable flowctrl  0x01:enable flowctrl


#define MEM_RFCOMM_LMP_DIS_FLAG_ADDR	(0xc4acf)
#define MEM_RFCOMM_LMP_DIS_FLAG_VALUE	(0x00)	//0x00:收到 rfcomm disconnected 不断开lmp 层连接  0x01:收到rfcomm disconnected断开lmp


#pragma pack(1)
typedef struct
{
    uint16_t IpcStartAddr;
    uint16_t IpcEndAddr;
    uint16_t ReadPtr;
    uint16_t WrtiePtr;
} IPC_TypeDef;
#pragma pack()

/**
  * @brief  yichip HCI CMD Structure definition
  */
#pragma pack(1)
typedef struct
{
    uint8_t type;
    uint8_t opcode;
    uint8_t DataLen;
    uint8_t *p_data;
} HCI_TypeDef;
#pragma pack()

/**
  * @brief   Ipc send data
	*
  * @param   IpcData: HCI_TypeDef
  *
  * @retval	 SUCCESS or ERROR
  */
void IPC_TxData(HCI_TypeDef *IpcData);

/**
  * @brief   Ipc Put Bt Data
	*
  * @param   *buf: Conform to format HCI_TypeDefs
  *
	* @param   size: *buf data len(bte),max value is 0xff+3=258
  *
  * @retval	 TRUE or FALSE
  */
Boolean IPC_PutBtData(const void *buf, uint32_t size);

/**
  * @brief   IpcReadBtData(get A complete package)
	*
  * @param   IpcData: HCI_TypeDef
  *
  * @retval	 TRUE or FALSE
  */
Boolean IPC_ReadBtData(HCI_TypeDef *IpcData);


/**
  * @brief   Ipc Get Bt Data
	*
  * @param   *buf: out data buf
  *
	* @param   *size:input--*buf size(byte);output--valid data len(byte)
  *
  * @retval	 TRUE or FALSE
  */
Boolean IPC_GetBtData(void *buf, uint32_t *size);

/**
  * @brief   get ipc buf available size
	*
  * @param   none
  *
  * @retval	 available size
  */
uint16_t IPC_get_available_size(void);

/**
  * @brief   get ipc buf available size
	*
  * @param   none
  *
  * @retval	 TRUE:have data  FALSE:not data
  */
Boolean IPC_have_data(void);

void IpcInit(void);

void PrintHCIPack(HCI_TypeDef *msg, const char *str);

#endif


