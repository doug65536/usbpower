#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "ctx.h"
#include "task.h"
#include "debug.h"

extern "C" void *task_start_trampoline();
extern "C" void task_yield(uint16_t forced_task);
extern "C" __attribute__((__noreturn__)) 
void task_yield_noreturn(uint16_t forced_task);

struct task {
  avrctx *sp = nullptr;
  void *stk_limit = nullptr;
  task_state state = task_state_free;
};

static constexpr size_t max_tasks = 4;

static task tasks[max_tasks];
static uint8_t task_index;

void task_init()
{
  tasks[0].state = task_state_running;
}

 __attribute__((__noreturn__))
void task_self_destruct(void *return_value)
{
  task *self = tasks + task_index;
  self->sp = (avrctx*)return_value;
  self->state = task_state_exited;
  task_yield_noreturn();
}

avrctx *task_cswitch(uint16_t forced_task, avrctx *outgoing_ctx)
{
  task *incoming_task;
  task *outgoing_task = tasks + task_index;

  if (task_index) {
    insist((void*)outgoing_ctx >= outgoing_task->stk_limit);
  }

  if (outgoing_task->state == task_state_running)
    outgoing_task->state = task_state_suspended;

  outgoing_task->sp = outgoing_ctx;

  uint8_t i;
  uint8_t skipped = 0;
  if (!forced_task) {
    for (i = task_index + 1; skipped < max_tasks; ++i, ++skipped) {
      // Wraparound and skip over task 0
      if (i >= max_tasks) {
        i = 0;
        continue;
      }
      incoming_task = tasks + i;
      if (incoming_task->state == task_state_ready)
        break;
    }
  } else {
    i = forced_task;
  }

  task_index = i;

  if (skipped >= max_tasks) {
    incoming_task = tasks;
    task_index = 0;
  }

  incoming_task->state = task_state_running;

  return incoming_task->sp;
}

// Create a task with the specified stack, function, and argument
// Returns new task id
// Return 0 if out of resources
uint8_t task_create(void *stack, size_t stack_sz, 
  void *(*entry)(void *arg), void *arg, 
  task_state initial_state)
{
  // Fill in the context save area to resume into the new task.
  // Wire it up so it will return to task_self_destruct, save
  // the return value of the start function, and terminate the task,
  // if the task function returns.
  // Set up the callee saved registers to contain the start function
  // and argument, point it at task_start_trampoline. The trampoline
  // sets up the parameter registers and jumps to the task function
  avrctx_bootstrap *bootstrap = reinterpret_cast<avrctx_bootstrap *>(
    ((uintptr_t)stack + stack_sz - sizeof(avrctx_bootstrap)) & -sizeof(int));
  
  uintptr_t addr = reinterpret_cast<uintptr_t>(task_self_destruct);
  bootstrap->task_self_destruct_hi = addr >> 8;
  bootstrap->task_self_destruct_lo = addr;
  
  addr = reinterpret_cast<uintptr_t>(task_start_trampoline);
  bootstrap->ctx.pc_hi = addr >> 8;
  bootstrap->ctx.pc_lo = addr;
  bootstrap->ctx.sreg = 0;  // interrupts disabled!

  addr = (uintptr_t)entry;
  bootstrap->ctx.r3 = (uint8_t)(addr >> 8);
  bootstrap->ctx.r2 = (uint8_t)(addr);

  addr = (uintptr_t)arg;
  bootstrap->ctx.r5 = (uint8_t)(addr >> 8);
  bootstrap->ctx.r4 = (uint8_t)addr;

  // Find a free task slot
  uint8_t task_id;
  // 0 slot is reserved for main thread
  for (task_id = 1; task_id < max_tasks; ++task_id) {
    if (tasks[task_id].state == task_state_free)
      break;
  }
  
  // Return 0 if out of resources
  if (task_id >= max_tasks)
    return 0;

  // Fill in task
  task *tp = tasks + task_id;
  tp->sp = &bootstrap->ctx;
  tp->state = initial_state;
  tp->stk_limit = stack;

  return task_id;
}

void task_resume(uint8_t task_id)
{
  tasks[task_id].state = task_state_ready;
}

void task_suspend_self()
{
  tasks[task_index].state = task_state_suspended;
  task_yield();
}

uint8_t task_current()
{
  return task_index;
}

void task_run_forever()
{
	sei();
	sleep_enable();

	while(1) {
		task_yield();
		sleep_mode();
	}
}
