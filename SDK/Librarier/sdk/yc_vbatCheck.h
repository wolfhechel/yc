#ifndef __VBAT_CHECK_H__
#define __VBAT_CHECK_H__
#include "yc3121.h"
#include "yc_lpm.h"

/**
 * @brief:纽扣功耗检测函数
 *
 * @param:none
 *
 * @retval:返回值bit1、bit2同时为1表示纽扣功耗异常
 */
uint8_t vbat_check(void);

#endif
