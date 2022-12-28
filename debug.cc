#include "debug.h"
#include "arch.h"
#include <stdint.h>
#include <stdlib.h>

void debug_leds_init()
{
	LED_DDR = ~0b11;
	LED_PORT = 0b00;
	// while (1) {
	// 	_delay_ms(1000);
	// }
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

noreturn_decl void debug_assert_failed(char const *file unused_decl,
  int line unused_decl, char const *expr unused_decl)
{
  abort();
}

extern "C" noreturn_decl
void hang()
{
  while (1) {
    LED_PORT ^= 0b11;
    for (uint32_t volatile i = 0; i < 200000; ++i);
  }
}
