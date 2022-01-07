#include "yc_trng.h"

/**
* @brief  init TRNG
*
* @param  none
*
* @retval none
*/
void TRNG_Init(void)
{
    disable_intr(INTR_TRNG);
    SYSCTRL_HWCTRL(11)  = 0xff;
    SYSCTRL_HWCTRL(10) |= 0xc0;

    *(volatile int *)(0xf8524) = 0x87878787;
    SYSCTRL_RNG_CTRL = 0;
    SYSCTRL_RNG_CTRL = 0x667f;                                          //rng generation enable
    while ((SYSCTRL_RNG_CTRL & (1 << 28)) == 0)
    {
    }
//  enable_intr(INTR_TRNG);
}

/**
* @brief  get 128bit TRNG data
*
* @param  rand[4]:128bit TRNG data out of buf
*
* @retval 0:succeed  1:error
*/
void TRNG_Get(volatile unsigned long *rand)
{
    rand[0] = SYSCTRL_RNG_DATA(0);
    rand[1] = SYSCTRL_RNG_DATA(1);
    rand[2] = SYSCTRL_RNG_DATA(2);
    rand[3] = SYSCTRL_RNG_DATA(3);
}

uint8_t GetTRNGData_8bit(void)
{
    return  GetTRNGData();
}

int GetTRNGData(void)
{
    static uint32_t old_rng_data  = 0;
    uint32_t tmp = 0;
    while (1)
    {
        tmp =  SYSCTRL_RNG_DATA(0);
        if ((tmp != old_rng_data) && (tmp != 0) && (tmp != 0xffffffff))
        {
            old_rng_data = tmp;
            break;
        }
    }
    return  tmp;
}

void Disable_Trng(void)
{
    SYSCTRL_HWCTRL(11)  = 0;
    SYSCTRL_HWCTRL(10) &= 1;
}
