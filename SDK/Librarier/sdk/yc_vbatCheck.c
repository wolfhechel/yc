#include "yc_vbatCheck.h"

//#define VBAT_DEBUG

uint8_t vbat_check(void)
{
    uint8_t ret = 0;
    uint32_t back_val_lpm_ctrl=0;
    uint32_t back_val_lpm_sensor = 0;
    uint32_t back_val_lpm_gpio_wkhi = 0;


    uint32_t val_lpm_ctrl=0;
    uint32_t val_lpm_sensor = 0;
    uint32_t val_lpm_gpio_wkhi = 0;
    uint32_t val_lpm_status = 0;

    val_lpm_ctrl = lpm_read(LPM_CTRL);           //f8400
    back_val_lpm_ctrl  = val_lpm_ctrl;
    val_lpm_ctrl |= (1<<5);
    lpm_write(LPM_CTRL,val_lpm_ctrl);
    #ifdef VBAT_DEBUG
    MyPrintf("f8400=%x\r\n",val_lpm_ctrl);
    #endif

    val_lpm_gpio_wkhi = lpm_read(LPM_GPIO_WKHI); //f8414
    back_val_lpm_gpio_wkhi = val_lpm_gpio_wkhi;
    val_lpm_gpio_wkhi &= (~(0x03<<21));
    lpm_write(LPM_GPIO_WKHI,val_lpm_gpio_wkhi);
    #ifdef VBAT_DEBUG
    MyPrintf("f8414=%x\r\n",val_lpm_gpio_wkhi);
    #endif

    val_lpm_sensor = lpm_read(LPM_SENSOR);       //f8404
    back_val_lpm_sensor = val_lpm_sensor;
    val_lpm_sensor |= (0x06);
    lpm_write(LPM_SENSOR,val_lpm_sensor);
    #ifdef VBAT_DEBUG
    MyPrintf("f8404=%x\r\n",val_lpm_sensor);
    #endif
    delay(10000);

    val_lpm_status = lpm_read(LPM_STATUS);
    #ifdef VBAT_DEBUG
    MyPrintf("f8478=%x\r\n",val_lpm_status);
    #endif
    ret = ((val_lpm_status>>16) & 0x1f);


    lpm_write(LPM_CTRL,     back_val_lpm_ctrl);
    lpm_write(LPM_GPIO_WKHI,back_val_lpm_gpio_wkhi);
    lpm_write(LPM_SENSOR,   back_val_lpm_sensor);

    return ret;
}