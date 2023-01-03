#include "ctx_avr.h"
#include <avr/pgmspace.h>
#include "timer.h"

ctx_init_fixup const task_init_fixups_arch[] PROGMEM = {
  { init_tag::entry_15_8, offsetof(ctx_avr, r3) },
  { init_tag::entry_7_0, offsetof(ctx_avr, r2) },

  { init_tag::tramp_15_8, offsetof(ctx_avr, pc_hi) },
  { init_tag::tramp_7_0, offsetof(ctx_avr, pc_lo) },

  { init_tag::arg_15_8, offsetof(ctx_avr, r5) },
  { init_tag::arg_7_0, offsetof(ctx_avr, r4) },

  { init_tag::exit_15_8, offsetof(ctx_avr_bootstrap, task_self_destruct_hi) },
  { init_tag::exit_7_0, offsetof(ctx_avr_bootstrap, task_self_destruct_lo) },

  { init_tag::end, 0 }
};

char *arch_init_stack(void *stack, size_t stack_sz)
{
  return (char*)(((uintptr_t)stack + stack_sz - 
    task_init_sz) & -task_init_align);
}

ctx_init_fixup arch_fetch_fixup(ctx_init_fixup const *fixup)
{
	ctx_init_fixup result;
	memcpy_P(&result, fixup, sizeof(result));
	return result;
}

static void arch_enable_irq_then_sleep_then_disable_irq()
{
  __asm__ __volatile__ (
	  // It is guaranteed to execute the sleep before taking an interrupt
    "sei\n"
    "sleep\n"
    "cli\n"
  );
}

void arch_sleep()
{
	sleep_enable();
	arch_enable_irq_then_sleep_then_disable_irq();
	sleep_disable();
}

void timer_init()
{
  OCR1A = 0xFF;
  TIMSK1 = (1U << OCIE1A);
  TIFR1 = (1U << TOV1);
  // div 1024, clear on timer compare match
  TCCR1B = (1U << CS10) | (1U << CS12) | (1U << WGM12);
}

ISR(TIMER1_COMPA_vect)
{
  //debug_leds_toggle_led_divisor(0, 16384L/8, 1000000L/8);
  timer_tick();
}
