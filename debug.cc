#include "debug.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdlib.h>

#ifndef LED_PORT
#define LED_DDR  DDRD
#endif
#ifndef LED_PORT
#define LED_PORT  PORTD
#endif
#ifndef LED_BIT
#define LED_BIT 0
#endif

void debug_leds_init()
{
	LED_DDR = ~0b11;
	LED_PORT = 0b00;
	// while (1) {
	// 	_delay_ms(1000);
	// }
}

void debug_leds_toggle()
{		
  LED_PORT ^= 0b11;
}

void debug_leds_toggle_led(uint8_t led_index)
{
  LED_PORT ^= 1U << (LED_BIT + led_index);
}

static uint32_t divisor_counters[2];

void debug_leds_toggle_led_divisor(uint8_t led_index, 
  uint32_t increment, uint32_t divisor)
{
  if ((divisor_counters[led_index] += increment) >= divisor) {
    divisor_counters[led_index] -= divisor;
    debug_leds_toggle_led(led_index);
  }
}

void debug_assert_failed(char const *file unused,
  int line unused, char const *expr unused)
{
  abort();
}
