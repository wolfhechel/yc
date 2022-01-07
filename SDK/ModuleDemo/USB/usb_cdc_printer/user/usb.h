
#define  BT_GBG_EN *(volatile uint8_t*)(0xc8904)

#define  HCI_CLASS_REQUEST_TYPE  0x20

                                       // SetReport HID Request
#define  HID_CLASS_REQUEST_TYPE  0x21 

// Standard Request Codes
#define  GET_STATUS              0x00  // Code for Get Status
#define  CLEAR_FEATURE           0x01  // Code for Clear Feature
#define  SET_FEATURE             0x03  // Code for Set Feature
#define  SET_ADDRESS             0x05  // Code for Set Address
#define  GET_DESCRIPTOR          0x06  // Code for Get Descriptor
#define  SET_DESCRIPTOR          0x07  // Code for Set Descriptor(not used)
#define  GET_CONFIGURATION       0x08  // Code for Get Configuration
#define  SET_CONFIGURATION       0x09  // Code for Set Configuration
#define  GET_INTERFACE           0x0A  // Code for Get Interface
#define  SET_INTERFACE           0x0B  // Code for Set Interface
#define  SYNCH_FRAME             0x0C  // Code for Synch Frame(not used)

// Standard Descriptor Types
#define  DSC_DEVICE              0x01  // Device Descriptor
#define  DSC_CONFIG              0x02  // Configuration Descriptor
#define  DSC_STRING              0x03  // String Descriptor
#define  DSC_INTERFACE           0x04  // Interface Descriptor
#define  DSC_ENDPOINT            0x05  // Endpoint Descriptor

// HID Descriptor Types
#define  DSC_HID			     0x21  // HID Class Descriptor
#define  DSC_HID_REPORT			 0x22  // HID Report Descriptor

// Define bmRequestType bitmaps
#define  IN_DEVICE               0x00  // Request made to device,
                                       // direction is IN
#define  OUT_DEVICE              0x80  // Request made to device,
                                       // direction is OUT
#define  IN_INTERFACE            0x01  // Request made to interface,
                                       // direction is IN
#define  OUT_INTERFACE           0x81  // Request made to interface,
                                       // direction is OUT
#define  IN_ENDPOINT             0x02  // Request made to endpoint,
                                       // direction is IN
#define  OUT_ENDPOINT            0x82  // Request made to endpoint,

#define USB_DEVICE_CLASS_AUDIO        	0x01
#define USB_DEVICE_CLASS_CDC  			0x02
#define USB_DEVICE_CLASS_HID                  	0x03
#define USB_DEVICE_CLASS_IMAGE 		0x06
#define USB_DEVICE_CLASS_PRINTER 		0x07
#define USB_DEVICE_CLASS_MASS 		0x08
#define USB_DEVICE_CLASS_HUB 			0x09
#define USB_DEVICE_CLASS_CDC_DATA	0x0a
#define USB_DEVICE_CLASS_SMARTCARD	0x0b
#define USB_DEVICE_CLASS_SECURITY		0x0d
#define USB_DEVICE_CLASS_VIDEO		0x0e
#define USB_DEVICE_CLASS_HEALTH		0x0f
#define USB_DEVICE_CLASS_AV			0x10
#define USB_DEVICE_CLASS_WIRELESS		0xe0

#define USB_ENDPOINT_TYPE_BUCK          		0x02
#define USB_ENDPOINT_TYPE_INT          		0x03
#define USB_ENDPOINT_TYPE_ADAP_ISO          	0x09
#define USB_ENDPOINT_TYPE_SYNC_ISO        	0x0d

#define GET_DEVICE_ID			0
#define SET_LINE_CODING			0x20
#define GET_LINE_CODING			0x21
#define SET_CONTROL_LINESTATE   0X22

// HID Request Codes
#define  GET_REPORT 		     0x01   // Code for Get Report
#define  GET_IDLE				 0x02   // Code for Get Idle
#define  GET_PROTOCOL			 0x03   // Code for Get Protocol
#define  SET_REPORT				 0x09   // Code for Set Report
#define  SET_IDLE				 0x0A   // Code for Set Idle
#define  SET_PROTOCOL			 0x0B   // Code for Set Protocol

#define  HID_RPT_ID           	0xf2
#define  HID_RPT_SIZE         	0x3f

#define  EP0_PACKET_SIZE         0x40 
#define  EP1_PACKET_SIZE         0x40
#define  HID_REPORT_DESCRIPTOR_SIZE    0x001B

#define  be16(x) ((x<<8)|(x>>8))                
#define LOBYTE(x)  ((byte)((x) & 0xff))
#define HIBYTE(x)  ((byte)((x) >> 8 & 0xff))
#define TRDBYTE(x)  ((byte)((x) >> 16 & 0xff))

#define WORD(x)				LOBYTE(x),HIBYTE(x)
#define TBYTE(x)				WORD(x),TRDBYTE(x)
#define DWORD(x)			WORD(x),WORD(x >> 16)

#define USB_STATUS_FIFO0_READY		1
#define USB_STATUS_FIFO1_READY		2
#define USB_STATUS_FIFO3_READY		8
#define USB_STATUS_SETUP			0x10
#define USB_STATUS_SUSPEND			0x20
#define USB_STATUS_NAK				0x40
#define USB_STATUS_RESET			0x80

typedef struct
{
	byte  bmRequestType;
	byte  bRequest;
	word wValue;
	word wIndex;
	word wLength;
} usb_setup;


void usb_main(void);
