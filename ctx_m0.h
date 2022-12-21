#pragma once
#include "ctx.h"
#include <string.h>

struct ctx_m0 : public ctx {
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
  uint32_t apsr;
  uint32_t pc;
};

struct ctx_m0_bootstrap {
  ctx_m0 ctx;
  uint32_t task_self_destruct;
};

static constexpr size_t const task_init_sz = sizeof(ctx_m0_bootstrap);

static ctx_init_fixup arch_fetch_fixup(ctx_init_fixup const *fixup)
{
	ctx_init_fixup result;
	memcpy(&result, fixup, sizeof(result));
	return result;
}

static inline void arch_irq_enable()
{
	__asm__ __volatile__ ("CPSIE i");
}

static inline void arch_irq_disable()
{
	__asm__ __volatile__ ("CPSID i");
}
