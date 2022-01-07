
/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usb_regs.h
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : USB OTG IP hardware registers.
 *****************************************************************************/


#ifndef __USB_OTG_REGS_H__
#define __USB_OTG_REGS_H__

#include "yc3121.h"
//#define  USB_HID_DEMO_TEST  1
//#define  POWER_FROM_USB    1
//#define USB_HID_MS_DEMO  1


#define CORE_USB_CONFIG                         USB_BASEADDR
#define CORE_USB_INT_MASK(x)            (USB_BASEADDR + 1 + x)
#define CORE_USB_ADDR                           (USB_BASEADDR + 4)
#define CORE_USB_TRIG                               (USB_BASEADDR + 0x10)
#define CORE_USB_STALL                          (USB_BASEADDR + 0x11)
#define CORE_USB_CLEAR                          (USB_BASEADDR + 0x12)
#define CORE_USB_EP(x)                          (USB_BASEADDR + 0x18 + x)
#define CORE_USB_EP_LEN(x)                  (USB_BASEADDR + 0x20 + x)
#define CORE_USB_STALL_STATUS               (USB_BASEADDR + 0x25)
#define CORE_USB_STATUS                         (USB_BASEADDR + 0x26)
#define CORE_USB_FIFO_EMPTY             (USB_BASEADDR + 0x27)
#define CORE_USB_FIFO_FULL                  (USB_BASEADDR + 0x28)
#define USB_SOFCNT                              (USB_BASEADDR + 0x29)
#define USB_SOFCNTHI                            (USB_BASEADDR + 0x2a)




#define USB_OTG_FS_BASE_ADDR                 CORE_USB_CONFIG
#define NUM_EP_FIFO                          4
#define USB_OTG_EP_FIFO_REG_OFFSET           0x0001

#define USB_OTG_MAX_TX_FIFOS                 4

#define USB_OTG_HS_MAX_PACKET_SIZE           512
#define USB_OTG_FS_MAX_PACKET_SIZE           64
#define USB_OTG_MAX_EP0_SIZE                 64




typedef struct _USB_OTG_CTRLREGS
{
    volatile uint8_t USB_CTL;
    volatile uint8_t IRQ_MASK1;
    volatile uint8_t IRQ_MASK2;
    volatile uint8_t IRQ_MASK3;
    volatile uint8_t FADDR;

} USB_OTG_CTRLREGS;
typedef struct _USB_OTG_XFERCTL
{
    volatile uint8_t TRIG;
    volatile uint8_t STALL;
    volatile uint8_t TOGGLE;
} USB_OTG_XFERCTL;

typedef struct _USB_OTG_EPREGS
{
    volatile uint8_t EP0;
    volatile uint8_t EP1;
    volatile uint8_t EP2;
    volatile uint8_t EP3;

} USB_OTG_EPREGS;

typedef struct _USB_OTG_EP_LENREGS
{
    volatile uint8_t EP0_LEN;
    volatile uint8_t EP1_LEN;
    volatile uint8_t EP2_LEN;
    volatile uint8_t EP3_LEN;

} USB_OTG_EP_LENREGS;

typedef struct _USB_OTG_STATUSREGS
{
    volatile uint8_t STALL_STATUS;
    volatile uint8_t STATUS;
    volatile uint8_t EMPTY;
    volatile uint8_t FULL;
} USB_OTG_STATUSREGS;


typedef struct
{
    USB_OTG_CTRLREGS      *CTRLREGS;
    USB_OTG_XFERCTL       *xferctl;
    uint8_t               *ep[NUM_EP_FIFO];
    uint8_t               *LENREGS[NUM_EP_FIFO];
    USB_OTG_STATUSREGS    *STATUSEGS;

} USB_OTG_CORE_REGS, *PUSB_OTG_CORE_REGS;


typedef union _USB_OTG_CTL_TypeDef
{
    uint8_t d8;
    struct
    {
        uint8_t enable_iso_outep2           : 1;
        uint8_t enable_iso_inep2            : 1;
        uint8_t pad_bias_ctl                : 2;
        uint8_t enable_usb_function         : 1;
        uint8_t speed                       : 1;
        uint8_t resume                      : 1;
        uint8_t wakeup_enable               : 1;

    } b;


} USB_OTG_CTL_TypeDef;


typedef union _USB_OTG_IRQ_MASK1_TypeDef
{
    uint8_t d8;
    struct
    {
        uint8_t outep0_irq_mask             : 1;
        uint8_t outep1_irq_mask             : 1;
        uint8_t outep2_irq_mask             : 1;
        uint8_t outep3_irq_mask             : 1;
        uint8_t setup_mask                      : 1;
        uint8_t suspend_mask                    : 1;
        uint8_t nak_mask                        : 1;
        uint8_t stall_mask                      : 1;
    }        b;

}       USB_OTG_IRQ_MASK1_TypeDef;


typedef union _USB_OTG_IRQ_MASK2_TypeDef
{
    uint8_t d8;
    struct
    {
        uint8_t inep0_empty_irq_mask            : 1;
        uint8_t inep1_empty_irq_mask            : 1;
        uint8_t inep2_empty_irq_mask            : 1;
        uint8_t inep3_empty_irq_mask            : 1;
        uint8_t outep0_empty_irq_mask           : 1;
        uint8_t outep1_empty_irq_mask           : 1;
        uint8_t outep2_empty_irq_mask           : 1;
        uint8_t outep3_empty_irq_mask           : 1;
    } b;

} USB_OTG_IRQ_MASK2_TypeDef;



typedef union _USB_OTG_IRQ_MASK3_TypeDef
{
    uint8_t d8;
    struct
    {
        uint8_t inep0_full_irq_mask         : 1;
        uint8_t inep1_full_irq_mask         : 1;
        uint8_t inep2_full_irq_mask         : 1;
        uint8_t inep3_full_irq_mask         : 1;
        uint8_t outep0_full_irq_mask            : 1;
        uint8_t outep1_full_irq_mask            : 1;
        uint8_t outep2_full_irq_mask            : 1;
        uint8_t outep3_full_irq_mask            : 1;
    } b;

} USB_OTG_IRQ_MASK3_TypeDef;





typedef union _USB_OTG_trig_TypeDef
{
    uint8_t d8;
    struct
    {
        uint8_t send_ep0_data                                       : 1;
        uint8_t send_ep1_data                                       : 1;
        uint8_t send_ep2_data                                       : 1;
        uint8_t send_ep3_data                                       : 1;
        uint8_t reply_endpiont0_zero_packet         : 1;
        uint8_t reply_endpiont1_zero_packet         : 1;
        uint8_t reply_endpiont2_zero_packet         : 1;
        uint8_t reply_endpiont3_zero_packet         : 1;
    } b;

} USB_OTG_trig_TypeDef;


typedef union _USB_OTG_stall_TypeDef
{
    uint8_t d8;
    struct
    {
        uint8_t ep0_stall               : 1;
        uint8_t inep1_stall             : 1;
        uint8_t outep1_stall                : 1;
        uint8_t inep2_stall             : 1;
        uint8_t outep2_stall                : 1;
        uint8_t inep3_stall             : 1;
        uint8_t outep3_stall                : 1;
        uint8_t reserved                : 1;
    } b;

} USB_OTG_stall_TypeDef;

typedef union _USB_OTG_toggle_TypeDef
{
    uint8_t d8;
    struct
    {
        uint8_t clear_inep1_to_data0        : 1;
        uint8_t clear_outep1_to_data0   : 1;
        uint8_t clear_inep2_to_data0        : 1;
        uint8_t clear_outep2_to_data0   : 1;
        uint8_t clear_inep3_to_data0        : 1;
        uint8_t clear_outep3_to_data0   : 1;
        uint8_t usb_reset1                      : 1;
        uint8_t usb_reset2                      : 1;
    } b;

} USB_OTG_toggle_TypeDef;

typedef union _USB_OTG_stall_status_TypeDef
{
    uint8_t d8;
    struct
    {
        uint8_t ep0_is_stall                : 1;
        uint8_t inep1_is_stall          : 1;
        uint8_t outep1_is_stall         : 1;
        uint8_t inep2_is_stall          : 1;
        uint8_t outep2_is_stall         : 1;
        uint8_t inep3_is_stall          : 1;
        uint8_t outep3_is_stall         : 1;
        uint8_t reserved                : 1;
    } b;

} USB_OTG_stall_status_TypeDef;

typedef union _USB_OTG_IRQ1_TypeDef
{
    uint8_t d8;
    struct
    {
        uint8_t outep0_irq              : 1;
        uint8_t outep1_irq              : 1;
        uint8_t outep2_irq              : 1;
        uint8_t outep3_irq              : 1;
        uint8_t setup                   : 1;
        uint8_t suspend             : 1;
        uint8_t nak                 : 1;
        uint8_t reset                   : 1;
    } b;

} USB_OTG_IRQ1_TypeDef;

typedef union _USB_OTG_IRQ2_TypeDef
{
    uint8_t d8;
    struct
    {
        uint8_t inep0_empty_irq         : 1;
        uint8_t inep1_empty_irq         : 1;
        uint8_t inep2_empty_irq         : 1;
        uint8_t inep3_empty_irq         : 1;
        uint8_t outep0_empty_irq        : 1;
        uint8_t outep1_empty_irq        : 1;
        uint8_t outep2_empty_irq        : 1;
        uint8_t outep3_empty_irq        : 1;
    } b;

} USB_OTG_IRQ2_TypeDef;



typedef union _USB_OTG_IRQ3_TypeDef
{
    uint8_t d8;
    struct
    {
        uint8_t inep0_full_irq          : 1;
        uint8_t inep1_full_irq          : 1;
        uint8_t inep2_full_irq          : 1;
        uint8_t inep3_full_irq          : 1;
        uint8_t outep0_full_irq         : 1;
        uint8_t outep1_full_irq         : 1;
        uint8_t outep2_full_irq         : 1;
        uint8_t outep3_full_irq         : 1;
    } b;

} USB_OTG_IRQ3_TypeDef;


#endif  /* __USB_OTG_REGS_H__ */

