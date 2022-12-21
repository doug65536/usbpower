
// ABI:
// callee saved: r4-r11. 
// r13 is sp, r14 is lr, r15 is pc
//  uint32_t r4;
//  uint32_t r5;
//  uint32_t r6;
//  uint32_t r7;
//  uint32_t r8;
//  uint32_t r9;
//  uint32_t r10;
//  uint32_t r11;
//  uint32_t apsr;
//  uint32_t pc;
// size_t task_yield(size_t force_task)
.global task_yield
.global task_yield_noreturn
task_yield:
task_yield_noreturn:
  // Return address and status register
  mov r2,lr
  mrs r1,PSR // _nzcvq
  push {r1-r2}

  // Save high registers
  mov r1,r9
  mov r2,r10
  mov r3,r11
  push {r1-r3}

  mov r1,r8
  push {r1}

  // Save low registers
  push {r3-r7}

  // Call scheduler, passing stack pointer in 2nd parameter (1st parameter
  // comes from the task_yield caller)
  mov r1,sp
  bl task_cswitch
  mov sp,r0

  pop {r0-r7}
  mov r4,r0
  mov r5,r1
  mov r6,r2
  mov r7,r3
  mov r8,r4
  mov r9,r5
  mov r10,r6
  mov r11,r7

  pop {r1-r2}
  msr PSR_nzcvq,r1
  bx r2

.global task_start_trampoline
task_start_trampoline:
  mov r0,r5
  bx r4
