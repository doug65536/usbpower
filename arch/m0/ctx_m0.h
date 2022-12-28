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

static constexpr size_t const task_init_sz = sizeof(ctx_m0);
static constexpr size_t const task_init_align = alignof(ctx_m0);
