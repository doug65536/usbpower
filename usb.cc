#include "usb.h"
#include <avr/io.h>

void usb_init()
{
  USBCON &= ~(1U << USBE);
  USBCON |= (1U << USBE);
  USBCON &= ~(0x01 << FRZCLK);
  UHWCON |= (0x01 << UVREGE);
  USBCON |= (0x01 << VBUSTE);
  UDCON &= ~(0x01 << DETACH);
  // PLLCSR = (((0x01 << PLLTM1) | (0x01 << PLLTM0)) | (0x01 << PLLE));
  // while(!(PLLCSR & (0x01 << PLOCK)));
}
