#include "ctx_avr.h"
#include <avr/pgmspace.h>

ctx_init_fixup const task_init_fixups_arch[] PROGMEM = {
  { init_tag::entry_15_0, offsetof(ctx_avr, r2) },
  { init_tag::tramp_15_8, offsetof(ctx_avr, pc_hi) },
  { init_tag::tramp_7_0, offsetof(ctx_avr, pc_lo) },
  { init_tag::arg_15_0, offsetof(ctx_avr, r4) },
  { init_tag::exit_15_8, offsetof(ctx_avr_bootstrap, task_self_destruct_hi) },
  { init_tag::exit_7_0, offsetof(ctx_avr_bootstrap, task_self_destruct_lo) },
  { init_tag::end, 0 }
};
