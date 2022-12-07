#pragma once
#include <stdint.h>
#include <stdlib.h>
#include "ctx.h"

enum task_state : uint8_t {
  task_state_free,
  task_state_suspended,
  task_state_ready,
  task_state_running,
  task_state_exited
};

void task_init();

// Returns task ID
uint8_t task_create(void *stack, size_t stack_sz, 
  void *(*entry)(void *arg), void *arg = nullptr, 
  task_state initial_state = task_state_ready);
 
// Schedules a specific suspended task to ready
void task_resume(uint8_t task_id);
// Suspend this task and switch to another task
void task_suspend_self();
// Return caller's task id
uint8_t task_current();
// Run the dispatcher/IRQ handler mainloop
__attribute__((__noreturn__))
void task_run_forever();

// Assembly
extern "C" avrctx *task_cswitch(uint16_t forced_task, avrctx *outgoing_ctx);
extern "C" void task_yield(uint16_t forced_task = 0);
extern "C" __attribute__((__noreturn__)) 
void task_yield_noreturn(uint16_t forced_task = 0);
