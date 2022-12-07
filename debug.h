// debug.h

#include <avr/pgmspace.h>

void debug_leds_init();
void debug_leds_toggle();
void debug_leds_toggle_led(uint8_t led_index);
void debug_leds_toggle_led_divisor(uint8_t led_index, 
  uint32_t increment, uint32_t wrap);

#define noreturn __attribute__((__noreturn__))
noreturn void debug_assert_failed(
  char const * PROGMEM file, int line, char const * PROGMEM expr);

#define unused __attribute__((__unused__))

#define assert(expr) ((expr) \
  ? true \
  : (debug_assert_failed(PSTR(__FILE__), __LINE__, PSTR(#expr)), false))

#define assume(expr) do { if (!(expr)) __builtin_unreachable(); } while (0)
#define MAX_DEBUG
#ifdef MAX_DEBUG
#define insist(expr) assert(expr)
#else
#define insist(expr) assume(expr)
#endif

// Disable optimization of a particular function
#define unoptimized __attribute__((__optimize__("-O0")))
