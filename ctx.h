#pragma once

#include <stdint.h>
#include <stddef.h>

struct ctx {
};

enum struct init_tag : uint8_t {
#if USE_PTR32
	// 32 bit
	entry_31_0,
#endif

#if USE_PTR16
	// 16 bit
	entry_15_0,
#endif

	// hi8
	entry_15_8,

	// lo8
	entry_7_0,
	
#if USE_PTR32
	// 32 bit
	arg_31_0,
#endif

#if USE_PTR16
	// 16 bit
	arg_15_0,
#endif

	// hi8
	arg_15_8,

	// lo8
	arg_7_0,

#if USE_PTR32
	// exit32
	exit_31_0,
#endif

#if USE_PTR16
	// exit16
	exit_15_0,
#endif

	// hi8
	exit_15_8,

	// lo8
	exit_7_0,

#if USE_PTR32
	// trampoline32
	tramp_31_0,
#endif

#if USE_PTR16
	// trampoline16
	tramp_15_0,
#endif

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