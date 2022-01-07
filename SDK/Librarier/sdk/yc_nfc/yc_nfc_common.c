#include "yc_nfc_common.h"

void Nfc_SysTick_Delay_Us(uint32_t us)
{
    //((void(*)(int us))0x42bc+1)(us);
    delay_us(us);
}

void Nfc_SysTick_Delay_Ms(uint32_t ms)
{
    //((void(*)(int ms))0x42e0+1)(ms);
    delay_ms(ms);
}

/************************** Communication Interface ***************** -Start- */

/* Hardware reset */
void YC_EMV_HwReset(void)
{
    RST_Disable()
    Nfc_SysTick_Delay_Ms(5);
    RST_Enable()
    Nfc_SysTick_Delay_Ms(20);
}

void WRITEREG(uint8_t addr, uint8_t data)
{
	uint8_t flag = 0, ucbuf[3] = {0};
	
	#ifdef NFC_CS_PIN
	NFC_CS_LOW();
	#endif	
    if (addr & 0x80)	//page1
    {
        flag = 1;
        ucbuf[0] = ((0x00 << 1) & 0xFE);
        ucbuf[1] = 0x01;
        SPI_SendBuff(NFC_SPI, ucbuf, 2);
    }
    ucbuf[0] = ((addr << 1) & 0xFE);
    ucbuf[1] = data;
    SPI_SendBuff(NFC_SPI, ucbuf, 2);
    if (flag)
    {
        ucbuf[0] = ((0x00 << 1) & 0xFE);
        ucbuf[1] = 0x00;
        SPI_SendBuff(NFC_SPI, ucbuf, 2);
    }
	#ifdef NFC_CS_PIN
	NFC_CS_HIGH();
	#endif

}
uint8_t READREG(uint8_t addr)
{
    uint8_t reg = 0xFF, ucbuf[3] = {0};
    uint8_t flag = 0;
	#ifdef NFC_CS_PIN
	NFC_CS_LOW();
	#endif	
    if (addr & 0x80)	//page1
    {
        flag = 1;
        ucbuf[0] = ((0x00 << 1) & 0xFE);
        ucbuf[1] = 0x01;
        SPI_SendBuff(NFC_SPI, ucbuf, 2);
    }
    ucbuf[0] = (((addr << 1) & 0xFE) | 0x01);
    SPI_SendAndReceiveData(NFC_SPI, ucbuf, 1, &reg, 1);
    if (flag)
    {
        ucbuf[0] = ((0x00 << 1) & 0xFE);
        ucbuf[1] = 0x00;
        SPI_SendBuff(NFC_SPI, ucbuf, 2);
    }
	#ifdef NFC_CS_PIN
	NFC_CS_HIGH();
	#endif	
    return reg;
}
