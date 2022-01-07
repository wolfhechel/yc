#include "yc_lpm.h"

uint32_t lpm_read(volatile int *addr)
{
    return ((uint32_t(*)(volatile int *addr))FUNC_LPM_READ_ADDR)(addr);
}

void lpm_write(volatile int *addr, uint32_t value)
{
    ((void(*)(volatile int *addr, uint32_t value))FUNC_LPM_WRITE_ADDR)(addr, value);
}

void lpm_bt_write(uint8_t type, uint32_t val)
{
    //  ((void(*)(uint8_t type,uint32_t val))FUNC_LPM_BT_WRITE_ADDR)(type,val);
    enable_clock(CLKCLS_BT);
    *(volatile uint8_t *)0xc804c = (val & 0x000000ff) >> 0;
    delay_us(1);
    *(volatile uint8_t *)0xc804d = (val & 0x0000ff00) >> 8;
    delay_us(1);
    *(volatile uint8_t *)0xc804e = (val & 0x00ff0000) >> 16;
    delay_us(1);
    *(volatile uint8_t *)0xc804f = (val & 0xff000000) >> 24;
    delay_us(1);
    *(volatile uint8_t *)0xc8005 = 1 << type;
    delay_ms(1);
}

uint32_t lpm_bt_read(uint8_t type)
{
    if (type == LPMBT_WKUPLOW || type == LPMBT_WKUPHIGH)
    {
        return (((uint32_t(*)(uint8_t type))FUNC_LPM_BT_READ_ADDR)(type)) >> 2;
    }
    else
    {
        return (((uint32_t(*)(uint8_t type))FUNC_LPM_BT_READ_ADDR)(type));
    }
}

void lpm_sleep(void)
{
    ((void(*)())FUNC_LPM_SLEEP_ADDR)();
}

void setlpmval(volatile int *addr, uint8_t startbit, uint8_t bitwidth, uint32_t val)
{
    ((void(*)(volatile int *addr, uint8_t startbit, uint8_t bitwidth, uint32_t val))FUNC_SETLPMVAL_ADDR)(addr, startbit, bitwidth, val);
}

uint32_t readlpmval(volatile int *addr, uint8_t startbit, uint8_t bitwidth)
{
    uint32_t tmp = 0;
    if (32 == bitwidth)
    {
        tmp = lpm_read(addr);
        return ((tmp >> startbit) & ((1 << bitwidth) - 1));
    }
    else
    {
        return lpm_read(addr);
    }
}

void GPIO_Unused_Pd(void)
{
    for (int i = 0; i < 48; i++)
    {
        if (GPIO_CONFIG(i) == 0 || GPIO_CONFIG(i) == 4 || GPIO_CONFIG(i) == 5)
        {
            GPIO_CONFIG(i) |= GPCFG_PD;
        }
    }
}

static uint32_t value_rtc_1s = 0;

static uint32_t get_otp(void)
{
    uint32_t time;

    if (value_rtc_1s == 0)
    {
        read_otp(0x13e, (uint8_t *)&time, 2);
        time = (time == 0) ? 0x8000 : time;
        time &= (~((uint32_t)0x1 << 31));
    }
    else
    {
        time = value_rtc_1s;
        time |= ((uint32_t)0x1 << 31);
    }

    return time;
}

void BT_Hibernate(void)
{
#if (VERSIONS == EXIST_BT)
    if(TRUE == ipc_inited)
    {
        ipc_inited = FALSE;

        read_otp(0x13e, (uint8_t *)&value_rtc_1s, 2);
        value_rtc_1s = (value_rtc_1s == 0) ? 0x8000 : value_rtc_1s;

        BT_CONFIG &= ~(1 << BT_INIT_FLAG);

        NVIC_DisableIRQ(BT_IRQn);
        SYSCTRL_ROM_SWITCH = 0x90;

        HCI_TypeDef IpcRxData;
        IpcRxData.type = 1;
        IpcRxData.opcode = 0x27;
        IpcRxData.DataLen = 0;
        IPC_TxData(&IpcRxData);
        //delay(10000);
        delay_ms(60);
    }
#elif (VERSIONS == NO_BT || VERSIONS == NO_XTAL)
    if(TRUE == ipc_inited)
    {
        ipc_inited = FALSE;
        BT_CONFIG &= ~(1 << BT_INIT_FLAG);
        NVIC_DisableIRQ(BT_IRQn);
        SYSCTRL_ROM_SWITCH = 0x90;
        lpm_bt_write(2,0);
        delay_us(1000);
        lpm_bt_write(3,0);
        delay_us(1000);
        lpm_bt_write(5,0);
        delay_us(1000);
        lpm_bt_write(1,0xd80000);
        delay_us(1000);
        lpm_bt_write(4,0x50000);
        delay_us(1000);
    }
#endif
}

void Chip_Speedstep(void)
{
    SYSCTRL_HCLKConfig(SYSCTRL_HCLK_Div8);
}

void CM0_Sleep(uint32_t time, uint32_t GPIO_Pin0_Pin31, uint16_t GPIO_Pin32_Pin47, uint8_t islow_wakeup, uint8_t is_powerdownbt)
{
    uint32_t temp;

    SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_INTR | SYSCTRL_AHBPeriph_SHA | \
                            SYSCTRL_AHBPeriph_CRC | SYSCTRL_AHBPeriph_PWM | \
                            SYSCTRL_AHBPeriph_WDT | SYSCTRL_AHBPeriph_USB | \
                            SYSCTRL_AHBPeriph_SPI | SYSCTRL_AHBPeriph_DES | \
                            SYSCTRL_AHBPeriph_RSA | SYSCTRL_AHBPeriph_ASE | \
                            SYSCTRL_AHBPeriph_7816 | SYSCTRL_AHBPeriph_SM4 | \
                            SYSCTRL_AHBPeriph_7811 | SYSCTRL_AHBPeriph_ADC7811 | \
                            SYSCTRL_AHBPeriph_CP, DISABLE);

    //DISABLE RTC WAKEUP
    setlpmval(LPM_GPIO_WKHI, 17, 1, 0);

    //set gpio wakeup level
    setlpmval(LPM_GPIO_WKHI, 18, 1, islow_wakeup ? 1 : 0);

    //set gpio num
    if(GPIO_Pin0_Pin31!=0 || GPIO_Pin32_Pin47!=0)
    {
        for(int i = 0;i < 32;i++)
        {
            if((GPIO_Pin0_Pin31 & (1<<i)) != 0)
            {
                if (islow_wakeup)
                    GPIO_CONFIG(i) = PULL_UP;
                else
                    GPIO_CONFIG(i) = PULL_DOWN;
            }
        }
        for(int i = 0;i<16;i++)
        {
            if((GPIO_Pin32_Pin47 & (1<<i)) != 0)
            {
                if (islow_wakeup)
                    GPIO_CONFIG(i+32) = PULL_UP;
                else
                    GPIO_CONFIG(i+32) = PULL_DOWN;
            }
        }
        setlpmval(LPM_GPIO_WKUP, 0, 32, GPIO_Pin0_Pin31);
        setlpmval(LPM_GPIO_WKHI, 0, 16, GPIO_Pin32_Pin47);
    }
    else
    {
        setlpmval(LPM_GPIO_WKUP, 0, 32, 0);
        setlpmval(LPM_GPIO_WKHI, 0, 16, 0);
    }

    temp = get_otp();

    if (time)
    {
        //SET SECMAX
        setlpmval(LPM_SECMAX, 0, 16, temp);
        //SET SLEEP CNT
        setlpmval(LPM_WKUP_TIMER, 0, 32, time);
        //clr RTC CNT
        setlpmval(LPM_RTC_CNT, 0, 32, 0);
        // ENABLE RTC WAKEUP
        setlpmval(LPM_GPIO_WKHI, 17, 1, 1);
    }

    if (is_powerdownbt == 1 && (temp >> 31) == 0)
    {
        BT_Hibernate();
    }

    lpm_sleep();
}

void enable_gpio_32(void)
{
    uint32_t value = lpm_bt_read(LPMBT_CONTROL1);
    value &= ~(1<<17);
    lpm_bt_write(LPMBT_CONTROL1, value);
}
