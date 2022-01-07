#ifndef __YC_CRC16_H__
#define __YC_CRC16_H__

#include "..\core\yc3121.h"
#include "..\core\system.h"

uint16_t crc16_calc(uint16_t iv, uint32_t mask, uint8_t *data, uint16_t data_len);

#define CRC16_TEST

#endif
