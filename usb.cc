#include "usb.h"
#include "debug.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

static const uint8_t usb_device_desc[18] PROGMEM = {
  // bLength
  sizeof(usb_device_desc),

  // bDescriptorType, 1 is device
  1,

  // USB protocol supported (0x200)
  0x00,
  0x02,

  // Device class
  0,

  // bDeviceSubclass
  0,

  // bDeviceProtocol
  0,

  // bMaxPacketSize0 (UECFG1X)
  32,

  // idVendor (0xB94B randomly chosen)
  0x4B,
  0xB9,

  // idProduct (0xB012)
  0x12,
  0xB0,

  0,

  // bcdDevice
  1,

  // iManufacturer
  0,

  // iProduct
  0,

  // iSerialNumber
  0,

  // bNumConfigurations
  1
};
//config_size = 34;
static uint8_t const usb_config_descriptor[25] PROGMEM = {
  //
  // Configuration descriptor (wrapper)

  // bLength
  9,

  // bDescriptorType - 2 is device
  2,

  // config size
  sizeof(usb_config_descriptor),

  // bNumInterfaces
  1,

  // bConfigurationValue
  1,
  
  // iConfiguration
  0,

  // bmAttributes
  0xC0,

  // bMaxPower 2mA units
  50,

  //
  // Interface descriptor

  // bLength
  9,

  // bDescriptorType 4 = interface
  4,

  // bInterfaceNumber
  0,

  // bAlternateSetting
  0,

  // bNumEndpoints
  1,

  // bInterfaceClass
  0x03,//fixme

  // bInterfaceSubclass
  0x01,//fixme

  // bInterfaceProtocol
  0x01,//fixme

  // iInterface
  0,

  //
  // Endpoint

  // bLength
  7,

  // bDescriptorType (endpoint descriptor)
  0x05,

  // endpoint number (0x80 = IN, endpoint 1)
  0x80 | 1,

  // bmAttributes (interrupt)
  0x03,

  // wMaxPacketSize
  12, 
  12 >> 8,

  // wInterval (poll every one ms)
  1,
  1 >> 8
};

static void usb_select_ep(uint8_t ep)
{
  UENUM = ep;
}

static void usb_endpoint_enable_irq_rx_setup()
{
  UEIENX = (1U << RXSTPE);
}
static void usb_endpoint_enable()
{
  UECONX = (1U << EPEN);
}

static void usb_endpoint_dir(uint8_t dir)
{
  UECFG0X = dir;
}

static void usb_endpoint_alloc(uint8_t size)
{
  UECFG1X |= size | 2;
}

static bool usb_endpoint_config_ok()
{
  return UESTA0X & (1U << CFGOK);
}

static void usb_endpoint_reset()
{
  UERST = 1;
  UERST = 0;
}

static void usb_disable()
{
  USBCON &= ~(1U << OTGPADE) & ~(1U << USBE);
}

static void usb_enable()
{
  USBCON |= (1U << USBE) | (1U << OTGPADE);
}

static void usb_reset()
{
  usb_disable();
  usb_enable();
}

unused
static void usb_freeze_clk()
{
  USBCON |= (1U << FRZCLK);
}

static void usb_unfreeze_clk()
{
  USBCON &= ~(1U << FRZCLK);
}

static void usb_mode_fullspeed()
{
  UDCON = 0;
}

static void usb_attach()
{
  UDCON &= ~(1U << DETACH);
}

unused
static void usb_detach()
{
  UDCON |= (1U << DETACH);
}

static void usb_enable_irq_end_of_reset()
{
  UDIEN |= (1U << EORSTE);
}

static void usb_enable_irq_start_of_frame()
{
  UDIEN |= (1U << SOFE);
}

void usb_init()
{
#ifdef __AVR_ATmega32U4__
  usb_reset();
  usb_unfreeze_clk();
  usb_mode_fullspeed();
  usb_attach();
  usb_enable_irq_end_of_reset();
  usb_enable_irq_start_of_frame();
#endif

  // PLLCSR = (((1U << PLLTM1) | (1U << PLLTM0)) | (1U << PLLE));
  // while(!(PLLCSR & (1U << PLOCK)));
}

static void usb_handle_end_of_reset()
{
  usb_select_ep(0);
  usb_endpoint_enable();
  usb_endpoint_dir(0);
  usb_endpoint_alloc(32);
  assert(usb_endpoint_config_ok());
  usb_endpoint_reset();
}

static bool usb_ep_bank_writeable()
{
  return UEINTX & (1U << RWAL);
}

unused 
static void usb_send_progmem(void const * PROGMEM p, size_t sz)
{
  while (sz--) {
    UEDATX = *(char const *)pgm_read_byte(p);
    p = (char const *)p + 1;
  }
}

unused 
static void usb_send_u8(uint8_t value)
{
  UEDATX = value;
}

unused 
static void usb_send_u16(uint16_t value)
{
  UEDATX = (uint8_t)value;
  UEDATX = (uint8_t)(value >> 8);
}

unused 
static void usb_send_u24(uint32_t value)
{
  UEDATX = (uint8_t)value;
  UEDATX = (uint8_t)(value >> 8);
  UEDATX = (uint8_t)(value >> 16);
}

unused 
static void usb_send_u32(uint32_t value)
{
  UEDATX = (uint8_t)value;
  UEDATX = (uint8_t)(value >> 8);
  UEDATX = (uint8_t)(value >> 16);
  UEDATX = (uint8_t)(value >> 24);
}

static void usb_endpoint_irq_mask_set(uint8_t mask)
{
  UEINTX |= mask;
}

static void usb_endpoint_irq_mask_clr(uint8_t mask)
{
  UEINTX &= ~mask;
}

static void usb_handle_start_of_frame()
{
  usb_select_ep(1);
  if (!usb_ep_bank_writeable())
    return;
  // vinsense
  usb_send_u16(0);
  // ampval
  usb_send_u16(0);
  // vpreregsensehi
  usb_send_u16(0);
  // vpostregout
  usb_send_u16(0);
  // Digital inputs (PWRIN, HWB# button)
  usb_send_u16(0);
  // Digital output current state (PREREGEN#, TESTLOAD, LED)
  usb_send_u16(0);
  usb_endpoint_irq_mask_set((1U << STALLEDI) | (1U << RXOUTI) |
    (1<< RXSTPI) | (1U << NAKOUTI) | (1U << STALLEDI));
}

static void usb_handle_rx_out()
{

}

ISR(USB_GEN_vect)
{
  hang();
  uint8_t udint = UDINT;

  if (udint & (1U << EORSTI))
    usb_handle_end_of_reset();
  if (udint & (1U << RXOUTI))
    usb_handle_rx_out();
  if (udint & (1U << SOFI))
    usb_handle_start_of_frame();
}

static uint8_t usb_recv_u8()
{
  uint8_t value = UEDATX;
  return value;
}

static uint16_t usb_recv_u16()
{
  uint16_t value = UEDATX;
  value |= (uint16_t)UEDATX << 8;
  return value;
}

static uint32_t usb_recv_u24()
{
  uint32_t value = UEDATX;
  value |= (uint32_t)UEDATX << 8;
  value |= (uint32_t)UEDATX << 16;
  return value;
}

static void usb_endpoint_wait_bank()
{
  while (!(UEINTX & TXINI));
}

#define USB_REQ_SET_FEATURE 0x03
#define USB_REQ_SET_ADDRESS 0x05
#define USB_REQ_GET_DESCRIPTOR 0x06
#define USB_REQ_GET_CONFIGURATION 0x08

static void usb_handle_rx_setup_packet()
{
  // Receive the packet and return 
  uint8_t bmRequestType = usb_recv_u8();
  uint8_t bRequest = usb_recv_u8();
  uint16_t wValue = usb_recv_u16();
  uint16_t wIndex = usb_recv_u16();
  uint16_t wLength = usb_recv_u16();
  usb_endpoint_irq_mask_clr((1U << RXSTPI) | (1U << RXOUTI) | (1U << TXINI));

  if (bRequest == USB_REQ_GET_DESCRIPTOR) {
    uint8_t const *descriptor = nullptr;
    size_t descriptor_sz = 0;
    if (wValue == 0x100) {
      descriptor = usb_device_desc;
      descriptor_sz = sizeof(usb_device_desc);
    } else if (wValue == 0x200) {
      descriptor = usb_config_descriptor;
      descriptor_sz = sizeof(usb_config_descriptor);
    }
    usb_endpoint_wait_bank();
    usb_send_progmem(descriptor, descriptor_sz);
    usb_endpoint_irq_mask_clr((1U << TXINI));
  }
}

ISR(USB_COM_vect)
{
  hang();
  if (UEINTX & (1U << RXSTPI))
    usb_handle_rx_setup_packet();
}
