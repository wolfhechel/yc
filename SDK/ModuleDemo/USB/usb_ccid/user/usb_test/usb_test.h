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
#define  EP1_PACKET_SIZE         6
#define  HID_REPORT_DESCRIPTOR_SIZE    0x001B

#define  be16(x) ((x<<8)|(x>>8))                

#define USB_STATUS_FIFO0_READY		1
#define USB_STATUS_FIFO1_READY		2
#define USB_STATUS_FIFO3_READY		8
#define USB_STATUS_SETUP			0x10
#define USB_STATUS_SUSPEND			0x20
#define USB_STATUS_NAK				0x40
#define USB_STATUS_STALL			0x80

typedef struct
{
	byte  bmRequestType;
	byte  bRequest;
	word wValue;
	word wIndex;
	word wLength;
} usb_setup;

//------------------------------------------
// Standard Device Descriptor Type Defintion
//------------------------------------------
typedef /*code*/ struct
{
   byte  bLength;              // Size of this Descriptor in Bytes
   byte  bDescriptorType;      // Descriptor Type (=1)
   word bcdUSB;               // USB Spec Release Number in BCD
   byte  bDeviceClass;         // Device Class Code
   byte  bDeviceSubClass;      // Device Subclass Code
   byte  bDeviceProtocol;      // Device Protocol Code
   byte  bMaxPacketSize0;      // Maximum Packet Size for EP0
   word idVendor;             // Vendor ID
   word idProduct;            // Product ID
   word bcdDevice;            // Device Release Number in BCD
   byte  iManufacturer;        // Index of String Desc for Manufacturer
   byte  iProduct;             // Index of String Desc for Product
   byte  iSerialNumber;        // Index of String Desc for SerNo
   byte  bNumConfigurations;   // Number of possible Configurations
} device_descriptor;              // End of Device Descriptor Type

//--------------------------------------------------
// Standard Configuration Descriptor Type Definition
//--------------------------------------------------
typedef /*code*/ struct
{
   byte  bLength;              // Size of this Descriptor in Bytes
   byte  bDescriptorType;      // Descriptor Type (=2)
   word wTotalLength;         // Total Length of Data for this Conf
   byte  bNumInterfaces;       // No of Interfaces supported by this
                                  // Conf
   byte  bConfigurationValue;  // Designator Value for *this*
                                  // Configuration
   byte  iConfiguration;       // Index of String Desc for this Conf
   byte  bmAttributes;         // Configuration Characteristics (see below)
   byte  bMaxPower;            // Max. Power Consumption in this
                                  // Conf (*2mA)
} configuration_descriptor;       // End of Configuration Descriptor Type

//----------------------------------------------
// Standard Interface Descriptor Type Definition
//----------------------------------------------
typedef /*code*/ struct
{
   byte  bLength;              // Size of this Descriptor in Bytes
   byte  bDescriptorType;      // Descriptor Type (=4)
   byte  bInterfaceNumber;     // Number of *this* Interface (0..)
   byte  bAlternateSetting;    // Alternative for this Interface (if any)
   byte  bNumEndpoints;        // No of EPs used by this IF (excl. EP0)
   byte  bInterfaceClass;      // Interface Class Code
   byte  bInterfaceSubClass;   // Interface Subclass Code
   byte  bInterfaceProtocol;   // Interface Protocol Code
   byte  iInterface;           // Index of String Desc for this Interface
} interface_descriptor;           // End of Interface Descriptor Type

//------------------------------------------
// Standard Class Descriptor Type Definition
//------------------------------------------
typedef /*code */struct
{
   byte  bLength;              // Size of this Descriptor in Bytes (=9)
   byte  bDescriptorType;      // Descriptor Type (HID=0x21)
   word bcdHID;    			  // HID Class Specification
                                  // release number (=1.01)
   byte  bCountryCode;         // Localized country code
   byte  bNumDescriptors;	  // Number of class descriptors to follow
   byte  bReportDescriptorType;// Report descriptor type (HID=0x22)
   unsigned int wItemLength;	  // Total length of report descriptor table
} class_descriptor;               // End of Class Descriptor Type

//---------------------------------------------
// Standard Endpoint Descriptor Type Definition
//---------------------------------------------
typedef /*code*/ struct
{
   byte  bLength;              // Size of this Descriptor in Bytes
   byte  bDescriptorType;      // Descriptor Type (=5)
   byte  bEndpointAddress;     // Endpoint Address (Number + Direction)
   byte  bmAttributes;         // Endpoint Attributes (Transfer Type)
   word wMaxPacketSize;	      // Max. Endpoint Packet Size
   byte  bInterval;            // Polling Interval (Interrupt) ms
} endpoint_descriptor;            // End of Endpoint Descriptor Type

//---------------------------------------------
// HID Configuration Descriptor Type Definition
//---------------------------------------------
// From "USB Device Class Definition for Human Interface Devices (HID)".
// Section 7.1:
// "When a Get_Descriptor(Configuration) request is issued,
// it returns the Configuration descriptor, all Interface descriptors,
// all Endpoint descriptors, and the HID descriptor for each interface."
typedef /*code*/ struct {
	configuration_descriptor 	hid_configuration_descriptor;
	interface_descriptor 		hid_interface_descriptor;
	class_descriptor 			hid_descriptor;
	endpoint_descriptor 		hid_endpoint_in_descriptor;
	endpoint_descriptor 		hid_endpoint_out_descriptor;
} hid_configuration_descriptor;

