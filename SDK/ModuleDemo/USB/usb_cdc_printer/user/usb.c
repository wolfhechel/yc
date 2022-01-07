#include "yc_sysctrl.h"
#include "usb.h"
#include "misc.h"
#include "yc_timer.h"

void dmacopy_nowait(byte *dest, byte *src, int len);

typedef struct
{
    uint32_t bitrate;
    uint8_t  format;
    uint8_t  paritytype;
    uint8_t  datatype;
} LINE_CODING;

LINE_CODING linecoding =
{
    921600, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
};

int  usb_get_buffer(uint8_t *buf);
void usb_last_packet(void);
void enable_systick(int counter);
void TIMER_Configuration(void);

extern byte aes[];
#define VENDOR_ID       0x483
#define PRODUCT_ID      0x5720

byte const *bufptr;
byte remain, isstr;
usb_setup usb0_setup;
byte usb1_send_flag = 1;
byte mcmd, rxlen, clear_fifo;
byte usbbuf[65], uartbuf[100];
byte usbtxbuf[128];
byte pcnt[3000] __attribute__((aligned(256)));
int uwp, urp, tptr, next_urp, timeout, totallen, pcp, tc, chksum;
int utxwp, utxrp;

const byte devicedesc[] =
{
    18,                     //bLength
    DSC_DEVICE,         //bDescriptorType
    WORD(0x110),            //bcdUSB
    0x02,                   //bDeviceClass
    0x00,                   //bDeviceSubClass
    0x00,                   //bDeviceProtocol
    EP0_PACKET_SIZE,        //bMaxPacketSize0
    WORD(VENDOR_ID),        //idVendor
    WORD(PRODUCT_ID),       //idProduct
    WORD(0x0200),           //bcdDevice
    0x01,                   //iManufacturer
    0x02,                   //iProduct
    0x03,                   //iSerialNumber
    0x01                    //bNumConfigurations
};


const byte confdesc[] =
{
    /* Configuration 1 */
    0x09,
    0x02,
    0x43,
    0x00,
    0x02,
    0x01,
    0x00,
    0xc0,
    0x32,

#if 1
    /* Standard interface descriptor */
    0x09,                                          /* bLength */
    0x04,                              /* bDescriptorType */
    0x00,
    0x00,
    0x01,
    0x02,
    0x02,
    0x01,
    0x00,

    0x05,
    0x24,
    0x00,
    0x10,
    0x01,

    0x05,
    0x24,
    0x01,
    0x00,
    0x00,

    /*ACM Functional Descriptor*/
    0x04,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
    0x02,   /* bmCapabilities */

    /*Union Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x06,   /* bDescriptorSubtype: Union func desc */
    0x00,   /* bMasterInterface: Communication class interface */
    0x01,   /* bSlaveInterface0: Data Class Interface */

    /*Endpoint 2 Descriptor*/
    0x07,                           /* bLength: Endpoint Descriptor size */
    0x05,   /* bDescriptorType: Endpoint */
    0x82,                     /* bEndpointAddress */
    0x03,                           /* bmAttributes: Interrupt */
    8,     /* wMaxPacketSize: */
    0,
    0xFF,                           /* bInterval: */
#endif

    /*Data class interface descriptor*/
    0x09,   /* bLength: Endpoint Descriptor size */
    0x04,  /* bDescriptorType: */
    0x01,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x02,   /* bNumEndpoints: Two endpoints used */
    0x0A,   /* bInterfaceClass: CDC */
    0x00,   /* bInterfaceSubClass: */
    0x00,   /* bInterfaceProtocol: */
    0x00,   /* iInterface: */

    /*Endpoint OUT Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    0x05,      /* bDescriptorType: Endpoint */
    0x01,                        /* bEndpointAddress */
    0x02,                              /* bmAttributes: Bulk */
    0x40,  /* wMaxPacketSize: */
    0x00,
    0x00,                              /* bInterval: ignore for Bulk transfer */

    /*Endpoint IN Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    0x05,      /* bDescriptorType: Endpoint */
    0x81,                         /* bEndpointAddress */
    0x02,                              /* bmAttributes: Bulk */
    0x40,  /* wMaxPacketSize: */
    0x00,
    0x00                               /* bInterval: ignore for Bulk transfer */


};

char const string0[] = "\x4\x3\x9\x4";
char const string1[] = "Yichip Technologies";
char const string2[] = "YICHIP3121 Virtual ComPort";
char const string3[] = "b120300001";
char const id[] = "\x0\xaprinter;";
char const *const string[] =   {string0, string1, string2, string3};


const byte strlength[] = {sizeof(string0), sizeof(string1), sizeof(string2), sizeof(string3)};


static void usb0_force_stall(void)
{
    USB_STALL =  1;
}

static void usb_trig(int ep)
{
    volatile int i;
    for (i = 0; i < 90; i++);
    USB_TRG =  1 << ep;
    return;
}

static void usb0_tx()
{
    byte len, i;

    if (remain)
    {
        len = remain > EP0_PACKET_SIZE ? EP0_PACKET_SIZE : remain;
        for (i = 0; i < len; i++)
        {
            USB_EP(0) = bufptr[i];
            if (isstr) USB_EP(0) = 0;
        }
        bufptr += len;
        remain -= len;
        usb_trig(0);
        USB_INT_MASK(1) =  0xfe;
    }
    else
    {
        USB_INT_MASK(1) =  0xff;
    }
}


static void usb0_respond(byte const *buff, byte size)
{
    if (isstr)
    {
        USB_EP(0) = (size + 1) * 2;
        USB_EP(0) = 3;
    }
    if (size > usb0_setup.wLength)
        size = usb0_setup.wLength;
    bufptr = buff;
    remain = size;
    usb0_tx();
}


static void usb0_request_set_address(void)
{
    if (usb0_setup.bmRequestType == IN_DEVICE && usb0_setup.wIndex == 0
            && usb0_setup.wLength == 0)
    {
        USB_TRG = 0x10;
        USB_ADDR = usb0_setup.wValue;
    }
    else
    {
        usb0_force_stall();
    }
}
/*
static byte strlength(char const *str)
{
    byte i;
    for(i = 0;str[i];i++);
    return i;
}
*/
static void usb0_request_set_configuration(void)
{
    if (usb0_setup.bmRequestType == IN_DEVICE
            && usb0_setup.wIndex == 0 && usb0_setup.wLength == 0
            && usb0_setup.wValue <= 1)
    {
        USB_TRG = 0x10;
    }
    else
    {
        usb0_force_stall();
    }
}

static void usb0_request_set_interface(void)
{
    if (usb0_setup.bmRequestType == IN_INTERFACE
            && usb0_setup.wValue == 0
            && usb0_setup.wLength == 0)
    {
        USB_TRG = 0x10;
    }
    else
    {
        usb0_force_stall();
    }
}

static void usb0_request_get_descriptor(void)
{
    byte index;
    isstr = 0;
    switch (usb0_setup.wValue >> 8)
    {
    case DSC_DEVICE:
        usb0_respond((byte const *)&devicedesc, sizeof(devicedesc));
        if (USB_ADDR > 0) USB_ADDR |= 0x80;
        break;
    case DSC_CONFIG:
        usb0_respond(confdesc, sizeof(confdesc));
        break;
    case DSC_STRING:
        index = usb0_setup.wValue  & 3;
        isstr = index > 0;
        usb0_respond((byte const *)string[index], strlength[index] - 1);
        break;

    default:
        usb0_force_stall();
        break;
    }
}

#define INCPTR(x, inc)      (x + inc >= sizeof(usbbuf) ? x + inc - sizeof(usbbuf) : x + inc)

#define INTXCPTR(x, inc)      (x + inc >= sizeof(usbtxbuf) ? x + inc - sizeof(usbtxbuf) : x + inc)
int buflen()
{
    return uwp < urp ? uwp - urp + sizeof(usbbuf) : uwp - urp;
}

int txbuflen()
{
    return utxwp < utxrp ? utxwp - utxrp + sizeof(usbtxbuf) : utxwp - utxrp;
}

int txbufreeelen()
{
    return sizeof(usbtxbuf) - (utxwp < utxrp ? utxwp - utxrp + sizeof(usbtxbuf) : utxwp - utxrp);
}
void usb2buf(int len)
{
    int i;
    for (i = 0; i < len; i++, uwp = INCPTR(uwp, 1))
        usbbuf[uwp] = USB_EP(1);
}

void usb2txbuf(int txlen)
{
    byte len, i;
    if (txlen)
    {
        len = txlen > EP1_PACKET_SIZE ? EP1_PACKET_SIZE : txlen;
        for (i = 0; i < len; i++, utxrp = INTXCPTR(utxrp, 1))
            USB_EP(1)  = usbtxbuf[utxrp];
        usb_trig(1);
        usb1_send_flag = 0;
        USB_INT_MASK(1) =  0xfd;
    }
    else
    {
        USB_INT_MASK(1) =  0xff;
    }
}

void USB_IRQHandler(void)
{
    int i, len;
    byte status, empty, full;
    byte *buf = (byte *)&usb0_setup;
    status = USB_STATUS;
    empty = USB_FIFO_EMPTY;
    full = USB_FIFO_FULL;
    USB_FIFO_EMPTY =  empty;
    USB_STATUS =  status;
    USB_FIFO_FULL = full;
    if (status & USB_STATUS_RESET)
    {
        USB_ADDR = 0x00;
    }
    if (empty & 1)
    {
        usb0_tx();
    }
    if (empty & 2)
    {
        usb1_send_flag = 1;
        USB_INT_MASK(1) =  0xff;
    }
    if (status & USB_STATUS_FIFO0_READY)
    {
        len = USB_EP_LEN(0);
        for (i = 0; i < len; i++)
            buf[i] = USB_EP(0);
        if (status & USB_STATUS_SETUP)
        {
            usb0_setup.wLength = usb0_setup.wLength;
            usb0_setup.wValue = usb0_setup.wValue;
            if ((usb0_setup.bmRequestType & ~0x80) == HCI_CLASS_REQUEST_TYPE)
            {
                USB_TRG = 0x10;
            }
            else if ((usb0_setup.bmRequestType & ~0x80) == HID_CLASS_REQUEST_TYPE)
            {
                uint8_t Buf[7] = {0};
                switch (usb0_setup.bRequest)
                {
                case SET_IDLE:
                    USB_TRG = 0x10;
                    break;
                case GET_DEVICE_ID:
                    isstr = 0;
                    usb0_respond((uint8_t *)id, sizeof(id) - 1);
                    break;
                case SET_LINE_CODING:
                    break;
                case GET_LINE_CODING:
                    isstr = 0;
                    Buf[0] = (uint8_t)(linecoding.bitrate);
                    Buf[1] = (uint8_t)(linecoding.bitrate >> 8);
                    Buf[2] = (uint8_t)(linecoding.bitrate >> 16);
                    Buf[3] = (uint8_t)(linecoding.bitrate >> 24);
                    Buf[4] = linecoding.format;
                    Buf[5] = linecoding.paritytype;
                    Buf[6] = linecoding.datatype;
                    usb0_respond(Buf, 7);
                    break;
                case SET_CONTROL_LINESTATE:
                    USB_TRG = 0x10;
                    break;
                }
            }
            else
            {
                switch (usb0_setup.bRequest)
                {
                case SET_ADDRESS:
                    usb0_request_set_address();
                    break;
                case GET_DESCRIPTOR:
                    usb0_request_get_descriptor();
                    break;
                case SET_CONFIGURATION:
                    usb0_request_set_configuration();
                    break;
                case SET_INTERFACE:
                    usb0_request_set_interface();
                    break;

                default:
                    usb0_force_stall();
                    break;
                }
            }

        }
        else
        {
            linecoding.bitrate = (uint32_t)(buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
            linecoding.format = buf[4];
            linecoding.paritytype = buf[5];
            linecoding.datatype = buf[6];
            USB_TRG = 0x10;
        }

    }
    if (status & USB_STATUS_FIFO1_READY)
    {
        USB_STATUS =  USB_STATUS_FIFO1_READY;
        len = USB_EP_LEN(1);

        timeout = 0;
        if (status & USB_STATUS_NAK)
        {
            if(rxlen == 0)
            {
                rxlen = len;
            }                
            if (buflen() + rxlen < sizeof(usbbuf))
            {
                usb2buf(rxlen);
                for (i = 0; i < len - rxlen; i++) USB_EP(1);
                rxlen = 0;
            }
        }
        else if (len > 0)
        {
            if (buflen() + len < sizeof(usbbuf))
            {
                usb2buf(len);
                rxlen = 0;
            }
            else
            {
                rxlen = len;
            }
        }
    }
}



byte history[256] __attribute__((aligned(256)));
byte *hisptr = history;



void enable_systick(int counter)
{
    SYST_RVR = SYST_CVR = counter;
    SYST_CSR = 7;
}

int i, len, dataLen, usbTotallen;
uint32_t increse = 0;
void usb_main()
{

    remain = mcmd = rxlen = clear_fifo = tptr = uwp = urp = timeout = totallen = pcp = tc = chksum = utxwp = utxrp = 0;
    next_urp = -1;
    
    for (int i = 0; i < sizeof(pcnt) / sizeof(pcnt[0]); i++) pcnt[i] = 0;
    enable_clock(CLKCLS_BT);
    enable_clock(CLKCLS_UART);
    enable_clock(CLKCLS_USB);
    USB_CONFIG =  0x30;
    USB_INT_MASK(0) =  0x30;
    USB_INT_MASK(1) =  0xff;
    USB_INT_MASK(2) =  0xff;

    enable_intr(INTR_USB);
    NVIC_EnableIRQ(TIM0_IRQn);
    
    NVIC_SetPriority(USB_IRQn, 0);
    NVIC_SetPriority(TIM0_IRQn, 1);

    TIMER_Configuration();

    uint8_t uartretval;
    uint8_t recvlen;
    uint32_t recvtotallen = 0;
    uint8_t buf[65] = {0};
    uint8_t big_buffer[1024] = {0};
    uint8_t big_buffer_have_space = 1;
    
    while (1)
    {

        recvlen = buflen();

        if (recvlen > 0)
        {
            if (big_buffer_have_space)
            {
                recvlen = usb_get_buffer(buf);
                for (int i = 0; i < recvlen; i++)
                {
                    big_buffer[i] = buf[i];
                }
                recvtotallen += recvlen;
                MyPrintf("recvlen = %d recvtotallen = %d\r\n", recvlen, recvtotallen);
            }
            else
            {
                /*big_buffer_have_no_space*/
            }

        }
 
        if (txbuflen() > 0)
        {
            if (usb1_send_flag == 1)
            {
                usb2txbuf(txbuflen());
            }
        }


        /*usb_last_packet in TIMER0_IRQHandler*/

        //usb_last_packet();
        
        if (UART_IsRXFIFONotEmpty(UART0))
        {
            uartretval = UART_ReceiveData(UART0);
            switch (uartretval)
            {
            case '1':
                /*prapare usb tx send data*/
                for (int i = 0; i < 100; i++, utxwp = INTXCPTR(utxwp, 1))
                    usbtxbuf[utxwp]  = i;
                break;
            case '2':
                MyPrintf("big_buffer_have_space '1' !\n\n");
                big_buffer_have_space = 1;
                break;
            case '3':
                MyPrintf("big_buffer_have_space '0' !\n\n");
                big_buffer_have_space = 0;
                break;
            case 'c':
                MyPrintf("totallen = %d chksum = %d\r\n", totallen, chksum);
                recvtotallen  = 0;
                totallen = 0;
                chksum = 0;
                break;
            case 'n':
                MyPrintf("case 'n' !\n\n");
                USB_CONFIG =  0x00;
                delay_ms(1); 
                USB_CONFIG =  0x30;
                break; 
  
            default:
                break;
            }
            uartretval = 0;
        }
    };
}



int usb_get_buffer(uint8_t *buf)
{
    int len;

    len = buflen();
    if (len > 0)
    {

        for (int i = 0; i < len; i++, urp = INCPTR(urp, 1))
        {
            buf[i] = usbbuf[urp];
            chksum += buf[i];
        }
        totallen += len;

    }
    return len;
}

void usb_last_packet(void)
{
    /*please put usb_last_packet(); into Timer interrupt*/
    if (rxlen && buflen() + rxlen < sizeof(usbbuf))
    {
        /*The expected value of the timeout is 100ms;
          timeout = 100ms/each into Timer interrupt time*/
        if (timeout++ > 100)
        {
            usb2buf(rxlen);
            for (i = 0; i < 64 - rxlen; i++) USB_EP(1);
            rxlen = 0;
            tc++;
            timeout = 0;
        }
    }
}

/**
  * @brief  TIM configuration function.
  * @param  None
  * @retval None
  */
void TIMER_Configuration(void)
{
    TIM_InitTypeDef TIM_InitStruct;

    TIM_InitStruct.period = 96000;

    TIM_InitStruct.TIMx = TIM0;
    TIM_Init(&TIM_InitStruct);

    /* Configure timer for counting mode */
    TIM_ModeConfig(TIM0, TIM_Mode_TIMER);

    /* The last step must be enabled */
    TIM_Cmd(TIM0, ENABLE);
}

/**
  * @brief  TIM0~TIM8 Interrupt service function.
  * @param  None
  * @retval None
  */
void TIMER0_IRQHandler(void)
{
    usb_last_packet();
}
