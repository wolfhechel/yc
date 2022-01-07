#include "otausb.h"
#include "halUSB.h"
#include "misc.h"
#include "yc_qspi.h"

extern void start_app(void);
extern USB_OTG_CORE_HANDLE  USB_OTG_dev;
extern uint32_t APPWRITEADDR;

uint8_t  rev_data[64];
uint8_t  rev_data[64], tx_buffer[64];
uint8_t  tx_buffer_flash[8192];
uint8_t  rx_buffer_flash[8192];
uint8_t  send_data_flag;
uint8_t  rx_buffer[100] = {0};
uint8_t  tx_buffer_boot[32] = {0};
uint8_t  len, uart_flag;
uint8_t  tx_buffer[64] = {0};
uint8_t  q;
uint32_t earse_len, check_sum = 0, address = 0, now_packet_num, total_packet_num, last_packet_len, pc_check_sum;
uint32_t block_len = 0, total_len, rem_len, total_len_send;
_Bool  bBacktoAppFlag = 0;

void USB_OTA_START(void)
{
    /******************CMD_OTA_START***************/
    total_packet_num = rev_data[2] + (rev_data[3] << 8);
    total_len = rev_data[4] + (rev_data[5] << 8) + (rev_data[6] << 16);

    tx_buffer[0] = 0xAB;
    tx_buffer[1] = 0xC0;
    tx_buffer[2] = 0x3A;
    tx_buffer[3] = 0x00;
    DCD_EP_Tx(&USB_OTG_dev, HID_IN_EP, tx_buffer, 64);

}

void USB_OTA_DATA(void)
{
    /******************CMD_OTA_DATA***************/
    now_packet_num =  rev_data[2] + (rev_data[3] << 8);
    if (now_packet_num < total_packet_num)
    {
        for (int i = 6; i  <  64 ; i++)
        {
            tx_buffer_flash [block_len] = rev_data [i];
            block_len++;
            if (block_len == 8192)
            {
                enc_write_flash_bulk(APPWRITEADDR + address * 0x2000, tx_buffer_flash, 8192, 0);
                block_len = 0;
                address++;
                for (int i = 0; i < 8192; i++)
                {
                    tx_buffer_flash[i] = 0;
                }
            }
        }
        tx_buffer[0] = 0xAB;
        tx_buffer[1] = 0xC1;
        tx_buffer[2] = rev_data[2];
        tx_buffer[3] = rev_data[3];
        for (int i = 6; i < 64; i++)
        {
            check_sum = check_sum + rev_data[i];
        }
        DCD_EP_Tx(&USB_OTG_dev, HID_IN_EP, tx_buffer, 64);
    }
    else
    {
        last_packet_len = rev_data[4] + (rev_data[5] << 8);
        for (int i = 6; i  <  (last_packet_len + 6) ; i++)
        {
            tx_buffer_flash [block_len] = rev_data [i];
            block_len++;
            if (block_len == 8192)
            {
                enc_write_flash_bulk(APPWRITEADDR + address * 0x2000, tx_buffer_flash, 8192, 0);
                block_len = 0;
                address++;
                for (int i = 0; i < 8192; i++)
                {
                    tx_buffer_flash[i] = 0;
                }
            }
        }

        block_len = block_len + (32 - block_len % 32);

        enc_write_flash_bulk(APPWRITEADDR + address * 0x2000, tx_buffer_flash, block_len, 1);

        tx_buffer[0] = 0xAB;
        tx_buffer[1] = 0xC1;
        tx_buffer[2] = rev_data[2];
        tx_buffer[3] = rev_data[3];
        for (int i = 6; i < last_packet_len + 6; i++)
        {
            check_sum = check_sum + rev_data[i];
        }
        DCD_EP_Tx(&USB_OTG_dev, HID_IN_EP, tx_buffer, 64);
    }

}

void USB_OTA_END(void)
{
    /******************CMD_OTA_END***************/
    pc_check_sum =  rev_data[4] + (rev_data[5] << 8) + (rev_data[6] << 16);
    tx_buffer[0] = 0xAB;
    tx_buffer[1] = 0xC2;
    tx_buffer[2] = rev_data[2];
    tx_buffer[3] = rev_data[3];
    if (pc_check_sum == check_sum)
    {
        tx_buffer[4] = 0x55;
        bBacktoAppFlag = 1;
        MyPrintf("pc_check_sum sucess\n\n");
    }
    else
    {
        tx_buffer[4] = 0xAA;
        soft_reset();
    }
    DCD_EP_Tx(&USB_OTG_dev, HID_IN_EP, tx_buffer, 64);
}

void Cmd_check(void)
{
    if (halUsbDataAck() == HAVE_DATA)
    {
        len = halUsbReceive(rev_data, 64);
        switch ((rev_data[0] | rev_data[1] << 8))
        {
        case 0xC0BA:
            USB_OTA_START();
            break;
        case 0xC1BA:
            USB_OTA_DATA();
            break;
        case 0xC2BA:
            USB_OTA_END();
            break;
        }
    }
}

void BackToApp(unsigned char status)
{
    if (status)
    {
        delay_ms(10);
        MyPrintf("BackToApp\n\n");
        enc_erase_flash_32byte(0x1000120);
        start_app();
    }
}
