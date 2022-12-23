#pragma once
#include <stdint.h>
#include <stdlib.h>
#include "ctx.h"

using task_t = uint16_t;

enum task_state : uint8_t {
  task_state_free,
  task_state_suspended,
  task_state_ready,
  task_state_running,
  task_state_exited
};

void task_init();

// Returns task ID
task_t task_create(void *stack, size_t stack_sz, 
  void *(*entry)(void *arg), void *arg = nullptr, 
  task_state initial_state = task_state_ready);
 
// Schedules a specific suspended task to ready
void task_resume(task_t task_id);
// Suspend this task and switch to another task
void task_suspend_self();
// Return caller's task id
task_t task_current();
// Run the dispatcher/IRQ handler mainloop
noreturn_decl void task_run_forever();

// Assembly
extern "C" ctx *task_cswitch(task_t forced_task, ctx *outgoing_ctx);
extern "C" bool task_yield(task_t forced_task = 0);
extern "C" noreturn_decl bool task_yield_noreturn(task_t forced_task = 0);

extern uint8_t tasks_ready;
extern uint8_t task_index;

char *arch_init_stack(void *stack, size_t stack_sz);
ctx_init_fixup arch_fetch_fixup(ctx_init_fixup const *fixup);
void arch_sleep();