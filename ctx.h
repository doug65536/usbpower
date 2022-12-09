#pragma once
#include <stdint.h>

// context
// R0, R1,         R18–R27,          R30, R31 are clobbered
//         R2-R17,          R28, R29 are callee saved
struct avrctx {
  uint8_t unused; // weird postincrement push artifact
	uint8_t sreg;
	uint8_t r2;
	uint8_t r3;
	uint8_t r4;
	uint8_t r5;
	uint8_t r6;
	uint8_t r7;
	uint8_t r8;
	uint8_t r9;
	uint8_t r10;
	uint8_t r11;
	uint8_t r12;
	uint8_t r13;
	uint8_t r14;
	uint8_t r15;
	uint8_t r16;
	uint8_t r17;
	uint8_t r28;
	uint8_t r29;
  uint8_t pc_hi;
  uint8_t pc_lo;
};

static_assert(sizeof(avrctx) == 22, "unexpected context size");

struct avrctx_bootstrap {
  avrctx ctx;
  uint8_t task_self_destruct_hi;
  uint8_t task_self_destruct_lo;
};

static_assert(sizeof(avrctx_bootstrap) == 24, "unexpected boostrap size");
