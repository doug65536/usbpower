#pragma once

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#define noreturn_decl __attribute__((__noreturn__))
#define unused_decl __attribute__((__unused__))

// Only use 8 bit values in context init
#define USE_PTR32 0
#define USE_PTR16 0
#define USE_PTR8 1

#define TIMER_GCD 8
#define TIMER_MS 16
#define TIMER_US 384

#ifndef LED_PORT
#define LED_DDR  DDRB
#endif
#ifndef LED_PORT
#define LED_PORT  OCR0A
#endif
#ifndef LED_BIT
#define LED_BIT 4
#endif
