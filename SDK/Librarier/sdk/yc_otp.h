#ifndef __YC_OTP_H__
#define __YC_OTP_H__

#include "yc3121.h"

/*
 * @brief:  OTP deint
 * @param:  none
 * @return: none
 */
void deinit_otp(void);

/*
 * @brief:  OTP init
 * @param:  none
 * @return: none
 */
void init_otp(void);

/*
 * @brief:read_chip_life
 * @param:none
 * @return: lifecycle
 */
uint32_t read_chip_life(void);

/*
 * @brief:read_chipid
 * @param:chip id
 * @return: none
 */
void read_chipid(uint8_t *id);

/*
 * @brief:OTP read
 * @param:addr
 * @param:*data
 * @param: len
 * @return: 1-succ  0-fail
 */
uint8_t read_otp(uint32_t addr, uint8_t *data, uint32_t len);

/*
 * @brief:OTP write
 * @param:addr
 * @param:*data
 * @param: len
 * @return: 1-succ  0-fail
 */
uint8_t write_otp(uint32_t addr, uint8_t *data, uint32_t len);

#endif
