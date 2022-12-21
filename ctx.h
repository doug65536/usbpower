#pragma once

#include <stdint.h>
#include <stddef.h>

#if __PTRDIFF_WIDTH__ >= 4
#define USE_PTR32 1
#else
#define USE_PTR32 1
#endif

struct ctx {
};

enum struct init_tag : uint8_t {
#if USE_PTR32
	// 32 bit
	entry_31_0,
#endif

	// 16 bit
	entry_15_0,

	// hi8
	entry_15_8,

	// lo8
	entry_7_0,
	
#if USE_PTR32
	// 32 bit
	arg_31_0,
#endif

	// 16 bit
	arg_15_0,

	// hi8
	arg_15_8,

	// lo8
	arg_7_0,

#if USE_PTR32
	// exit32
	exit_31_0,
#endif

	// exit16
	exit_15_0,

	// hi8
	exit_15_8,

	// lo8
	exit_7_0,

#if USE_PTR32
	// trampoline32
	tramp_31_0,
#endif

	// trampoline16
	tramp_15_0,

	// hi8
	tramp_15_8,

	// lo8
	tramp_7_0,

	end
};

struct ctx_init_fixup {
	init_tag tag;
	uint8_t value;
};

extern ctx_init_fixup const task_init_fixups_arch[];