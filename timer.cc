#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "task.h"
#include "debug.h"

// Fractions of a second (8 microsecond units)
// We get a tick every 16384ms, 16ms + 384us
// We need to carry the microseconds every 1000us
// gcd(384, 1000) = 8
// 1000 / 8 = 125
//  384 / 8 = 48
static uint8_t fractsec;
static uint32_t millisec;
static uint8_t days;

// millisec can only withstand increasing for 49.7 days
// To handle this, every day, all timeouts are moved
// back 1 day worth of milliseconds, and millisec is
// moved back the same amount

static constexpr const uint32_t day_of_ms = 1000L * 86400;

// Called every 16384 microseconds
static void timer_tick()
{
  millisec += 16;
  fractsec += 384 / 8;

  if (fractsec >= 1000 / 8) {
    fractsec -= 1000 / 8;
    ++millisec;
  }
}

struct timer {
  uint32_t millisec;
  uint8_t task_id;
};

static constexpr const uint8_t max_timers = 4;
static constexpr const uint8_t timers_used_full = (1U << max_timers) - 1;
static uint8_t timers_used;
static timer timers[max_timers];

static bool timer_is_used(uint8_t timer)
{
  return timers_used & (1U << timer);
}

static void timer_mark_used(uint8_t timer)
{
  timers_used |= (1U << timer);
}

static void timer_mark_unused(uint8_t timer)
{
  timers_used &= ~(1U << timer);
}

static bool timers_full()
{
  return timers_used == timers_used_full;
}

void timer_init()
{
  OCR0B = 0xFF;
  TIMSK0 = (1U << TOIE0);
  TIFR0 = (1U << TOV0);
  // div 1024
  TCCR0B = (1U << CS00) | (1U << CS02);
}

void timer_wait_for_ms(uint32_t wait_millisec)
{
  return timer_wait_until(millisec + wait_millisec);
}

void timer_wait_until(uint32_t wakeup_millisec)
{
  insist(wakeup_millisec < millisec + day_of_ms);

  // Resume immediately if overdue already
  if (wakeup_millisec <= millisec)
    return;
  
  // Wait for a timer to become available
  while (timers_full())
    task_yield();
  
  for (uint8_t i = 0; i < max_timers; ++i) {
    if (!(timer_is_used(i))) {
      // Found unused slot, fill it in
      timer_mark_used(i);
      timers[i].task_id = task_current();
      timers[i].millisec = wakeup_millisec;

      // Suspend
      task_suspend_self();
      
      // Woke up...
      // Free the timer
      timer_mark_unused(i);
      // timers[i].millisec = 0;
      // timers[i].task_id = 0;
      return;
    }
  }
}

static void timer_notify()
{
  // Move the time and timeouts back a day, every day
  bool fixup = (millisec >= day_of_ms);

  for (uint8_t i = 0; timers_used && i < max_timers; ++i) {
    if (!(timers_used & (1U << i)))
      continue;
    if (timers[i].millisec <= millisec) {
      // Timer is active and has expired
      task_resume(timers[i].task_id);
      timers_used &= ~(1U << i);
      //timers[i].millisec = 0;
      //timers[i].task_id = 0;
    } else if (fixup) {
      // It didn't get cleared by timing out, so move it back
      timers[i].millisec -= day_of_ms;
    }
  }

  if (fixup) {
    millisec -= day_of_ms;
    days += (days < 0xFF);
  }

  //debug_leds_toggle_led(0);
}

ISR(TIMER0_OVF_vect)
{
  debug_leds_toggle_led_divisor(0, 61);
  timer_tick();
  timer_notify();
}
