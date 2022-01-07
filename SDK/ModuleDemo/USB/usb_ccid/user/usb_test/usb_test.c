#include "yc3121.h"
#include "usb_test.h"
#ifndef USB_CCID_DEMO

extern void dmacopy_nowait(byte* dest, byte* src, int len);
//extern void savelist(byte *dat);
extern void test_display(int);
extern byte aes[];
#define VENDOR_ID     0x1937
#define PRODUCT_ID    0x1135

const device_descriptor devicedesc =
{
  18,           //bLength
  0x01,         //bDescriptorType
  0x0002,         //bcdUSB
  0x00,         //bDeviceClass
  0x00,         //bDeviceSubClass
  0x00,         //bDeviceProtocol
  EP0_PACKET_SIZE,    //bMaxPacketSize0
  VENDOR_ID,        //idVendor
  PRODUCT_ID,     //idProduct
  0x0001,         //bcdDevice
  0x01,         //iManufacturer
  0x02,         //iProduct
  0x00,         //iSerialNumber
  0x01          //bNumConfigurations
};


const byte hidreportdesc[] =
{
  /*
  0x06, 0x00, 0xff,   // USAGE_PAGE (Vendor Defined Page 1)
  0x09, 0x01,       // USAGE (Vendor Usage 1)
  0xa1, 0x01,     // COLLECTION (Application)

  0x85, HID_RPT_ID, // Report ID
  0x95, HID_RPT_SIZE, // REPORT_COUNT ()
  0x75,0x08,      //REPORT_SIZE(8)
  0x26,0x00,0x01,   //LOGICAL_MAXIMUM(255)
  0x15,0x00,      //LOGICAL_MINIMUM(0)
  0x09,0x01,      //USAGE(VendorUsage1)
  0x81,0x02,      //INPUT(Data,Var,Abs)
  0x09,0x01,      //USAGE(VendorUsage1)
  0x91,0x02,      //OUTPUT(Data,Var,Abs)

  0xC0        // END Application Collection
  */
  /*
  //====================??====75====================================
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (5)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (3)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x38,                    //     USAGE (Wheel)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x03,                    //     REPORT_COUNT (1)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xc0,                          //   END_COLLECTION
    0xc0,                           // END_COLLECTION
    */
  0x05, 0x01, // Global Generic Desktop
  0x09, 0x02, // Local Mouse
  0xa1, 0x01, // Main app collection
  0x09, 0x01, // Local Pointer
  0xa1, 0x00, // Main phy collection
  0x95, 0x03, // Global ReportCount
  0x75, 0x01, // Global ReportSize
  0x05, 0x09, // Global Button
  0x19, 0x01, // Local Usage Min
  0x29, 0x03, // Local Usage Max
  0x15, 0x00, // Global Logical Min
  0x25, 0x01, // Global Logical Max
  0x81, 0x02, // Main Input(Data,Var,Abs)
  0x95, 0x01, // Global ReportCount
  0x75, 0x05, // Global ReportSize
  0x81, 0x03, // Main Input(Cnsr,Var,Abs)
  0x95, 0x03, 0x75, 0x08, // Global ReportCount Global ReportSize
  0x05, 0x01, // Global Generic Desktip
  0x09, 0x30, // Local X
  0x09, 0x31, // Local Y
  0x09, 0x38, // Locak Wheel
  0x15, 0x81, // Global Logical Min
  0x25, 0x7f, // Global Logical Max
  0x81, 0x06, // Main Input(Data,Var,Rel)
  0xc0, // Main End collection
  0xc0 // Main End collection
};

const byte confdesc[] =
{
  /*
    // configuration
    0x09,       // Length
    0x02,       // Type (configuration)
    41,         // Totallength (= 9+9+9+7+7)
    0x00,
    0x01,       // NumInterfaces
    0x01,       // bConfigurationValue
    0x00,       // iConfiguration
    0xa0,       // bmAttributes
    0x31,         // MaxPower (in 2mA units)

  // interface
    0x09,         // Length
    0x04,         // Type (interface)
    0x00,         // interface #0
    0x00,         // alternate interface #0
    0x02,         // number of endpoints
    0x03,         // Interface class (3 = HID)
    0x00,         // Interface subclass
    0x02,         // Interface protocol
    0x00,         // no string confdesc

  // hid
    0x09,         // Length
    0x21,         // Type (HID)
    0x01,         // bcdhid
    0x01,
    0x00,         // country code
    0x01,         // number of descriptors
    0x22,         // descriptor type
    0x1b,         // total length of report descriptor
    0x00,

  // IN endpoint
    0x07,         // Length
    0x05,         // Type (Endpoint)
    0x83,         // bit7:Direction is IN, bit3-0:endpoint number 1
    0x03,         // bit1-0:transfer type is interrupt
    HID_RPT_SIZE,     // maximum packet size
    0x00,
    0x01,         // polling interval is 1 frame

  // OUT endpoint
    0x07,         // Length
    0x05,         // Type (Endpoint)
    0x03,         // bit7:Direction is IN, bit3-0:endpoint number 1
    0x03,         // bit1-0:transfer type is interrupt
    HID_RPT_SIZE,     // maximum packet size
    0x00,
    0x01        // polling interval is 1 frame
  */



  //config
  0x09,//0xLength
  0x02,//0xType
  41,         // Totallength (= 9+9+9+7+7)
  0x00,
  0x01,//0xNumInterfaces
  0x01,//0xbConfigurationValue
  0x00,//0xstringindexConfiguration
  0x80,//0xbmAttributes
  0x32,//0xMaxPower0x(in0x2mA0xunits)

  //0xinterface_descriptor0xhid_interface_descriptor--mouse
  0x09,//0xbLength
  0x04,//0xbDescriptorType
  0x00,//0xbInterfaceNumber
  0x00,//0xbAlternateSetting
  0x01,//0xbNumEndpoints
  0x03,//0xbInterfaceClass0x(30x=0xHID)
  0x01,//0xbInterfaceSubClass
  0x02,//0xbInterfaceProcotol0x0x01 kb; 0x0x2=mouse
  0x00,//0xiInterface


  //class_descriptor hid_descriptor--mouse
  0x09,    //bLength
  0x21,    //bDescriptorType
  0x10, 0x01, //bcdHID
  0x21,    //bCountryCode
  0x01,    //bNumDescriptors
  0x22,    //bDescriptorType
  sizeof(hidreportdesc) & 0xff,  //wItemLength (tot. len. of report confdesc)
  (sizeof(hidreportdesc) >> 8) & 0xff,

  //endpoint_descriptor hid_endpoint_in_descriptor--mouse

  0x07,    //bLength
  0x05,    //bDescriptorType
  0x81,    //bEndpointAddress
  0x03,    // bmAttributes
  EP1_PACKET_SIZE, 0x00, //MaxPacketSize (LITTLE  ENDIAN)
  0x0A,    //bInterval
};





char const string0[] = "\x4\x3\x9\x4";
char const string1[] = "Yichip Technologies";
char const string2[] = "yc3121 usb mouse";
char const string3[] = "b120300001";

char const* const string[] =   {string0, string1, string2, string3};


const byte strlength[] = {sizeof(string0), sizeof(string1), sizeof(string2), sizeof(string3)};

byte const* bufptr;
byte remain, isstr;
usb_setup usb0_setup;
int usblen;
byte mcmd;
byte usbbuf[256];

static void usb0_force_stall(void)
{
  USB_STALL =  1;
}

static void usb_trig(int ep)
{
  int i;
  USB_TRG =  1 << ep;
  return;
  USB_STATUS = USB_STATUS_NAK;

  for (i = 0; i < 10000; i++)
    if (USB_STATUS & USB_STATUS_NAK)
    {
      break;
    }

  USB_FIFO_EMPTY = 1 << ep;

  for (i = 0; i < 10000; i++)
  {
    if (USB_FIFO_EMPTY & 1 << ep)
    {
      break;
    }
  }
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

      if (isstr)
      {
        USB_EP(0) = 0;
      }
    }

    //    savelist((byte*)bufptr);
    bufptr += len;
    remain -= len;
    usb_trig(0);
    GPIO_CONFIG(30) = GPCFG_OUTPUT_HIGH;
    delay(10);
    GPIO_CONFIG(30) = GPCFG_OUTPUT_LOW;
    USB_INT_MASK(1) =  0xfe;
  }
  else
  {
    USB_INT_MASK(1) =  0xff;
  }
}

static void usb0_respond(byte const* buff, byte size)
{
  if (isstr)
  {
    USB_EP(0) = (size + 1) * 2;
    USB_EP(0) = 3;
  }

  if (size > usb0_setup.wLength)
  {
    size = usb0_setup.wLength;
  }

  bufptr = buff;
  remain = size;
  usb0_tx();
}


static void usb0_request_set_address(void)
{
  MyPrintf("\n usb0_request_set_address:");

  if (usb0_setup.bmRequestType == IN_DEVICE && usb0_setup.wIndex == 0
      && usb0_setup.wLength == 0)
  {
    USB_TRG = 0x10;
    USB_ADDR = usb0_setup.wValue;
    MyPrintf("%02x", usb0_setup.wValue);
  }
  else
  {
    usb0_force_stall();
  }

  MyPrintf("\n");
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
  MyPrintf("\n usb0_request_set_configuration:");

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
  MyPrintf("\n usb0_request_set_interface:");

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
  MyPrintf("\n usb0_request_get_descriptor:");

  byte index;
  isstr = 0;

  switch (usb0_setup.wValue >> 8)
  {
    case DSC_DEVICE:
      MyPrintf("DEVICE:%d\n", sizeof(devicedesc));
      usb0_respond((byte const*)&devicedesc, sizeof(devicedesc));
      break;

    case DSC_CONFIG:
      MyPrintf("CONFIG:%d\n", sizeof(confdesc));
      usb0_respond(confdesc, sizeof(confdesc));
      break;

    case DSC_STRING:
      MyPrintf("STRING:\n");
      index = usb0_setup.wValue  & 3;
      isstr = index > 0;
      usb0_respond((byte const*)string[index], strlength[index] - 1);
      break;

    case DSC_HID:
      MyPrintf("DSC_HID mou\n");
      usb0_respond(confdesc + 18, confdesc[18]);
      break;

    case DSC_HID_REPORT:
      MyPrintf("DSC_HID_REPORT mou\n");
      usb0_respond(hidreportdesc, sizeof(hidreportdesc));
      break;

    /*
      case DSC_INTERFACE:
        usb0_respond((byte*)&desc_interface,sizeof(desc_interface));
        break;
      case DSC_ENDPOINT:
        if (usb0_setup.wValue&0xFF == IN_EP1)
        {
          usb0_respond((byte*)&desc_ep_in,sizeof(desc_ep_in));
        }
        else if (usb0_setup.wValue&0xFF == OUT_EP1)
        {
          usb0_respond((byte*)&desc_ep_out,sizeof(desc_ep_out));
        }
        else
        {
          usb0_force_stall();
        }
        break;
    */
    default:
      usb0_force_stall();
      break;
  }
}

void USB_IRQHandler()
{
  int i, j, len;
  byte status, empty;
  byte* buf = (byte*)&usb0_setup;
  status = USB_STATUS;
  empty = USB_FIFO_EMPTY;
  USB_FIFO_EMPTY =  empty;
  USB_STATUS =  status;

  if (empty & 1)
  {
    usb0_tx();
  }

  if (status & USB_STATUS_FIFO0_READY)
  {
    len = USB_EP_LEN(0);

    for (i = 0; i < len; i++)
    {
      buf[i] = USB_EP(0);
    }

    //    savelist(buf);
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
        if (usb0_setup.bRequest == SET_IDLE)
        {
          USB_TRG = 0x10;
        }
        else
        {
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

          /*
                  case GET_STATUS:
                    usb0_request_get_status();
                    break;
                      case CLEAR_FEATURE:
                    usb0_request_clear_feature();
                    break;
                  case SET_FEATURE:
                    usb0_request_set_feature();
                    break;
                  case GET_CONFIGURATION:
                    usb0_request_get_configuration();
                    break;
                  case GET_INTERFACE:
                    usb0_request_get_interface();
                    break;
          */
          default:
            usb0_force_stall();
            break;
        }
      }

    }

  }

  if (status & USB_STATUS_FIFO1_READY)
  {
    USB_STATUS =  USB_STATUS_FIFO1_READY;
    len = USB_EP_LEN(1);
    usblen = len;

    for (i = 0; i < len; i++)
    {
      usbbuf[i] = USB_EP(1);
    }

    USB_TRG = 0x20;
    return;

    for (i = 0; i < 64; i++)
    {
      USB_EP(1) = aes[i];
    }

    usb_trig(1);
  }

  if (status & USB_STATUS_FIFO3_READY)
  {
    USB_STATUS =  USB_STATUS_FIFO3_READY;
    len = USB_EP_LEN(3) | (int)USB_EP_LEN(4) << 8;
    usblen = len;

    for (i = 0; i < len; i++)
    {
      usbbuf[i] = USB_EP(3);
    }

    for (i = 0; i < len; i++)
      //      for(j = 0;j < 4;j++)
    {
      USB_EP(3) = usbbuf[i];
    }

    usb_trig(3);
  }

}

void test_usb()
{
  int flag = 0;
  remain = 0;
  enable_clock(CLKCLS_BT);
  enable_clock(CLKCLS_USB);
  BT_CLKPLL_EN = 0xff;
  SYSCTRL_HCLK_CON |= 1 << 11;
  USB_CONFIG =  0x30;
  USB_INT_MASK(0) =  0xf0;
  USB_INT_MASK(1) =  0xff;
  USB_INT_MASK(2) =  0xff;
  enable_intr(INTR_USB);


  while (1)
  {
    USB_EP(1) = 0x00;//report id
    USB_EP(1) = (flag == 0) ? 0x02 : (flag == 1) ? 0x01 : 0x00; //key
    USB_EP(1) = (flag == 0) ? 0x50 : (flag == 1) ? 0x00 : (flag == 2) ? 0xE2 : 0x00; //xlow
    USB_EP(1) = (flag == 0) ? 0x00 : (flag == 1) ? 0x00 : (flag == 2) ? 0xFF : 0x00; //xhigh
    USB_EP(1) = (flag == 0) ? 0x00 : (flag == 1) ? 0x30 : (flag == 2) ? 0x00 : 0xE2; //ylow
    USB_EP(1) = (flag == 0) ? 0x00 : (flag == 1) ? 0x00 : (flag == 2) ? 0x00 : 0xFF; //yhigh
    //    USB_EP(1) = 0x01;//wheel
    //    USB_EP(1) = 0x00;//twheel
    usb_trig(1);
    delay_ms(1000);

    if (flag > 2)
    {
      flag = 0;
    }
    else
    {
      flag++;
    }
  }


  while (1)
  {
    mcmd += 2;
    mcmd *= 2;
  };

  while (1)
  {
    if (mcmd)
    {
      USB_EP(1) = 0;
      USB_EP(1) = mcmd;
      USB_EP(1) = 0;
      USB_EP(1) = 0;
      USB_EP(1) = 0;
      USB_EP(1) = 0;
      USB_TRG =  2;
      mcmd = 0;
    }
  }
}

#endif
