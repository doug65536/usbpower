#pragma once

// Can you believe that they screwed up the underscored names?
#define noreturn_decl __attribute__((noreturn))
#define unused_decl __attribute__((__unused__))

// No need for progmem
#define PROGMEM
#define PSTR
// Only use 32 bit values in context init
#define USE_PTR32 1
#define USE_PTR16 0
#define USE_PTR8 0

#define TIMER_GCD 8
#define TIMER_MS 16
#define TIMER_US 384

extern int volatile dummy_led_port_ddr;
extern int volatile dummy_led_port;
#ifndef LED_PORT
#define LED_DDR  dummy_led_port_ddr
#endif
#ifndef LED_PORT
#define LED_PORT  dummy_led_port
#endif
#ifndef LED_BIT
#define LED_BIT 0
#endif

