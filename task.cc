#include <string.h>

#include "ctx.h"
#include "task.h"
#include "debug.h"

extern "C" void *task_start_trampoline(void*);
extern "C" void task_yield(task_t forced_task);
extern "C"
noreturn_decl
void task_yield_noreturn(task_t forced_task);

struct task {
  ctx *sp = nullptr;
  void *stk_limit = nullptr;
  task_state state = task_state_free;
};

static constexpr size_t max_tasks = 4;

task tasks[max_tasks];
uint8_t task_index;
uint8_t tasks_ready;

void task_init()
{
  tasks[0].state = task_state_running;
}

noreturn_decl void *task_self_destruct(void *return_value)
{
  task *self = tasks + task_index;
  self->sp = (ctx*)return_value;
  self->state = task_state_exited;
  task_yield_noreturn();
}

ctx *task_cswitch(task_t forced_task, ctx *outgoing_ctx)
{
  // Look up running task
  task *outgoing_task = tasks + task_index;

  // Verify no stack overflow if not main task
  if (task_index) {
    assert((void*)outgoing_ctx >= outgoing_task->stk_limit);
  }

  // Save the stack pointer for resume later
  outgoing_task->sp = outgoing_ctx;

  // If still running, make it ready
  if (outgoing_task->state == task_state_running) {
    outgoing_task->state = task_state_ready;
    ++tasks_ready;
  }

  // Assume the forced task
  uint8_t i = forced_task;
  uint8_t skipped = 0;
  
  // If caller doesn't specify a forced task
  if (!i && tasks_ready) {
    // Find next ready task, skipping idle task 0
    for (i = task_index + 1; skipped < max_tasks; ++i, ++skipped) {
      // Wraparound and skip over task 0
      if (i >= max_tasks) {
        // Restart at 1 (by the time the for loop increment happens)
        i = 0;
        continue;
      }

      if (tasks[i].state == task_state_ready)
        break;
    }
  }

  if (skipped >= max_tasks || !tasks_ready)
    i = 0;
  
  task_index = i;
  task *incoming_task = tasks + i;

  assert(incoming_task->state == task_state_ready);
  incoming_task->state = task_state_running;
  --tasks_ready;

  return incoming_task->sp;
}

// Create a task with the specified stack, function, and argument
// Returns new task id
// Return 0 if out of resources
task_t task_create(void *stack, size_t stack_sz, 
  void *(*entry)(void *arg), void *arg, 
  task_state initial_state)
{
  char *bootstrap = arch_init_stack(stack, stack_sz);

  // Wire it up so it will return to task_self_destruct, save
  // the return value of the start function, and terminate the task,
  // if the task function returns.
  // Set up the callee saved registers to contain the start function
  // and argument, point it at task_start_trampoline. The trampoline
  // sets up the parameter registers and jumps to the task function
  
  ctx_init_fixup fixup;
  for (ctx_init_fixup const *fixup_ptr = task_init_fixups_arch;
      (fixup = arch_fetch_fixup(fixup_ptr)), 
      (fixup.tag != init_tag::end); ++fixup_ptr) {
    char *dest = bootstrap + fixup.value;
    void *(*fn)(void*);
    union {
#if USE_PTR32
      uint32_t n32;
#endif
#if USE_PTR16
      uint16_t n16;
#endif
#if USE_PTR8
      uint8_t n8;
#endif
    };
    switch (fixup.tag) {
#if USE_PTR32
    case init_tag::entry_31_0:
      fn = entry;
      static_assert(sizeof(fn) == sizeof(uint32_t));
storefn:
      memcpy(dest, &fn, sizeof(fn));
      break;
#endif
#if USE_PTR16
    case init_tag::entry_15_0:
      n16 = (uintptr_t)entry;
store16:
      memcpy(dest, &n16, sizeof(n16));
      break;
#endif
#if USE_PTR8
    case init_tag::entry_15_8:
      n8 = ((uintptr_t)entry) >> 8;
store8:
      memcpy(dest, &n8, sizeof(n8));
      break;
    case init_tag::entry_7_0:
      n8 = (uintptr_t)entry;
      goto store8;
#endif
#if USE_PTR32
    case init_tag::tramp_31_0:
      fn = task_start_trampoline;
      goto storefn;
#endif
#if USE_PTR16
    case init_tag::tramp_15_0:
      n16 = (uintptr_t)task_start_trampoline + PC_OFFSET;
      goto store16;
#endif
#if USE_PTR8
    case init_tag::tramp_15_8:
      n8 = ((uintptr_t)task_start_trampoline + PC_OFFSET) >> 8;
      goto store8;
    case init_tag::tramp_7_0:
      n8 = (uintptr_t)task_start_trampoline + PC_OFFSET;
      goto store8;
#endif
#if USE_PTR32
    case init_tag::arg_31_0:
      n32 = (uintptr_t)arg;
store32:
      memcpy(dest, &n32, sizeof(n32));
      break;
#endif
#if USE_PTR16
    case init_tag::arg_15_0:
      n16 = (uintptr_t)arg;
      goto store16;
#endif
#if USE_PTR8
    case init_tag::arg_15_8:
      n8 = (uintptr_t)arg >> 8;
      goto store8;
    case init_tag::arg_7_0:
      n8 = (uintptr_t)arg;
      goto store8;
#endif
#if USE_PTR32
    case init_tag::exit_31_0:
      fn = task_self_destruct;
      goto storefn;
#endif
#if USE_PTR16
    case init_tag::exit_15_0:
      n16 = (uintptr_t)task_self_destruct;
      goto store16;
#endif
#if USE_PTR8
    case init_tag::exit_15_8:
      n8 = ((uintptr_t)task_self_destruct) >> 8;
      goto store8;
    case init_tag::exit_7_0:
      n8 = (uintptr_t)task_self_destruct;
      goto store8;
#endif
    case init_tag::end:
      __builtin_unreachable();
    }
  }

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
  tp->sp = (ctx*)bootstrap;
  tp->state = initial_state;
  tp->stk_limit = stack;

  tasks_ready += (tp->state == task_state_ready);

  return task_id;
}

void task_resume(task_t task_id)
{
  assert(tasks[task_id].state == task_state_suspended);
  if (tasks[task_id].state == task_state_suspended) {
    tasks[task_id].state = task_state_ready;
    ++tasks_ready;
  }
}

void task_suspend_self()
{
  tasks[task_index].state = task_state_suspended;
  task_yield();
}

task_t task_current()
{
  return task_index;
}

void task_run_forever()
{
	while(1) {
    // Keep waiting for interrupts until a task is ready
    if (tasks_ready)
      task_yield();
    else
      arch_sleep();
	}
}
