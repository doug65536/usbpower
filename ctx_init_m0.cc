#include "ctx_m0.h"
#include "task.h"

ctx_init_fixup const task_init_fixups_arch[] = {
  { init_tag::entry_31_0, offsetof(ctx_m0, r4) },
  { init_tag::arg_31_0, offsetof(ctx_m0, r5) },
  { init_tag::exit_31_0, offsetof(ctx_m0_bootstrap, task_self_destruct) },
  { init_tag::tramp_31_0, offsetof(ctx_m0, pc) },
  { init_tag::end, 0 }
};

char *arch_init_stack(void *stack, size_t stack_sz)
{
  return (char*)(((uintptr_t)stack + stack_sz - 
    task_init_sz) & -task_init_align);
}

static inline void arch_irq_enable()
{
	__asm__ __volatile__ ("CPSIE i");
}

static inline void arch_irq_disable()
{
	__asm__ __volatile__ ("CPSID i");
}

void arch_sleep()
{
  
}

ctx_init_fixup arch_fetch_fixup(ctx_init_fixup const *fixup)
{
	ctx_init_fixup result;
	memcpy(&result, fixup, sizeof(result));
	return result;
}

// fixme
int volatile dummy_led_port_ddr;
int volatile dummy_led_port;

extern "C" void abort()
{
  while(1);
}

extern "C" int _getpid()
{  
  return task_index + 1;
}

extern "C" void exit(int)
{
  abort();
}
