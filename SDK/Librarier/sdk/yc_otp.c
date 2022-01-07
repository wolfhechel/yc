#include "yc_otp.h"

/*
 * @brief:  OTP deint
 * @param:  none
 * @return: none
 */
void deinit_otp(void)
{
    ((void(*)())FUNC_DEINIT_OTP_ADDR)();
}

/*
 * @brief:  OTP init
 * @param:  none
 * @return: none
 */
void init_otp(void)
{
    ((void(*)())FUNC_INIT_OTP_ADDR)();
}

uint32_t read_chip_life(void)
{
    uint32_t life = 0;
    init_otp();
    life = ((uint32_t(*)(void))FUNC_READ_CHIPLF_ADDR)();
    deinit_otp();
    return life;
}

void read_chipid(uint8_t *id)
{
    init_otp();
    ((void(*)(uint8_t *id))FUNC_READ_CHIPID_ADDR)(id);
    deinit_otp();
}

/*
 * @brief:  OTP read
 * @param:  addr
 * @param:  *data
 * @param:  len
 * @return: none
 */
uint8_t read_otp(uint32_t addr, uint8_t *data, uint32_t len)
{
    init_otp();
    ((uint8_t(*)(uint32_t addr, uint8_t *data, uint32_t len))FUNC_READ_OTP_ADDR)(addr, data, len);
    deinit_otp();
    return SUCCESS;
}

/*
 * @brief:  OTP write
 * @param:  addr
 * @param:  *data
 * @param:  len
 * @return: SUCCESS or ERROR
 */
uint8_t write_otp(uint32_t addr, uint8_t *data, uint32_t len)
{
    uint8_t rtn = 0;

    //step2:
    rtn = ((uint8_t(*)(uint32_t addr, uint8_t *data, uint32_t len))FUNC_WRITE_OTP_ADDR)(addr, data, len);

    return rtn;
}
