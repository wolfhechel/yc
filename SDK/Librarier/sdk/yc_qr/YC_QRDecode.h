#ifndef __YC_QRDECODE_H__
#define __YC_QRDECODE_H__
#include "yc3121.h"
#include "YC_QRDecode_TypeDef.h"

/*
 * @brief:YC_QRDecode
 * @param:QRDecode_Config_CB
 * @param:len:  message len
 * @param:buf:  message
 * @return:  yc_qr_return_type_ENUM
 */
yc_qr_return_type_ENUM YC_QRDecode(QRDecode_Config_CB *p_QRDecode_Config,uint32_t *len, uint8_t *buf);

/**
 * @brief  get yichip scan code lib version
 * @param  none
 * @return lib version:
 *					bit0~bit15:low version number
 *					bit16~bit31:high version number
 */
uint32_t SC_GetVersion(void);

#endif
