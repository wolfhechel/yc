在使用SCI接口时，需要初始化SCI各IO口引脚，根据实际使用引脚和卡到位检测电平进行初始化。
注意一：卡到位电平设置为未插卡时IO引脚电平状态。 1为高电平、低为0低电平.

下面函数在外设初始化时调用。
void ISO7816_IO_Config(void)
{
    sci_pin.clk.gpio_group = GPIOC;
    sci_pin.clk.gpio_pin = GPIO_Pin_13;

    sci_pin.io.gpio_group = GPIOC;
    sci_pin.io.gpio_pin = GPIO_Pin_14;

    sci_pin.reset.gpio_group = GPIOC;
    sci_pin.reset.gpio_pin = GPIO_Pin_15;

    sci_pin.detect.gpio_group = DET_PORT;
    sci_pin.detect.gpio_pin = DET_PIN;
    GPIO_Config(sci_pin.detect.gpio_group,sci_pin.detect.gpio_pin,INPUT|PULL_UP);
    sci_pin.detect_state = 1;
	
    period_7816 = (CPU_MHZ / 4000000 / 2) - 1;
    contact_reset_l_cnt = 42000 * (CPU_MHZ / 1000000) / 4;
    contact_atr_rfb_time_cnt = 47000 * (CPU_MHZ / 1000000) / 4;
    contact_atr_rab_time_cnt = (20164 * (CPU_MHZ / 1000000) / 4) * 372;

    atr_time_start();
}

注意二：
yc_7816.h内宏定义
量产时应如下：
//#define DEBUG_7816_EMV          //定义时开启EMV测试时打印交互状态
//#define DEBUG_7816              //定义时开启打印交互数据
//#define EMV_L1_TEST             //定义时进行EMV测试，量产代码屏蔽此定义

过检时如下：
#define DEBUG_7816_EMV          //定义时开启EMV测试时打印交互状态
//#define DEBUG_7816              //定义时开启打印交互数据
#define EMV_L1_TEST             //定义时进行EMV测试，量产代码屏蔽此定义

注意三：
SCI模块用到两个定时器。定时器0用于计时、定时器3用于产生clk。应用代码要避免使用这两个定时器。

注意四：
量产时开启超时退出功能，时间设置默认为3S.应用固件如开启看门狗建议时间大于等于6S.

