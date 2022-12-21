#include "ctx_m0.h"

ctx_init_fixup const task_init_fixups_arch[] = {
  { init_tag::entry_31_0, offsetof(ctx_m0, r4) },
  { init_tag::arg_31_0, offsetof(ctx_m0, r5) },
  { init_tag::exit_31_0, offsetof(ctx_m0_bootstrap, task_self_destruct) },
  { init_tag::tramp_31_0, offsetof(ctx_m0, pc) },
  { init_tag::end, 0 }
};
