#include "task.h"

static char test_task_stack[96];
void *stored_arg;
task_t task_tid;

void *task_test(void *arg)
{
  return (void*)0x87654321;
}

__attribute__((__constructor__))
void ctor_test()
{
  task_tid = 99;
}

int main()
{
  task_init();
  task_tid = task_create(test_task_stack, sizeof(test_task_stack), 
    task_test, (void*)0x12345678);
  task_run_forever();
}
