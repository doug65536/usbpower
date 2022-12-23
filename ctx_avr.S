; Do what gcc does
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__SREG__ = 0x3f
__tmp_reg__ = 0
__zero_reg__ = 1
	; uint8_t sreg;
	; uint8_t r1;
	; uint8_t r2;
	; uint8_t r3;
	; uint8_t r4;
	; uint8_t r5;
	; uint8_t r6;
	; uint8_t r7;
	; uint8_t r8;
	; uint8_t r9;
	; uint8_t r10;
	; uint8_t r11;
	; uint8_t r12;
	; uint8_t r13;
	; uint8_t r14;
	; uint8_t r15;
	; uint8_t r16;
	; uint8_t r17;
	; uint8_t r28;
	; uint8_t r29;
  ; void (*pc)();

.text
.Lnothing_is_ready:
  ; Return false
  ldi r24,0
  ret

; bool task_yield()
.global task_yield
.global task_yield_noreturn
task_yield:
task_yield_noreturn:
  in r0,__SREG__
  ; Return immediately if no tasks are ready
  lds r18,tasks_ready
  tst r18
  breq .Lnothing_is_ready
  ; S
  cli
  push r29
  push r28
  push r17
  push r16
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push r7
  push r6
  push r5
  push r4
  push r3
  push r2
  push r0
  in r22,__SP_L__
  in r23,__SP_H__
#if 0
  ldi r30,pm_lo8(task_cswitch)
  ldi r31,pm_hi8(task_cswitch)
  icall
#else
  rcall task_cswitch
#endif
  out __SP_H__,r25
  out __SP_L__,r24
  pop r0
  pop r2
  pop r3
  pop r4
  pop r5
  pop r6
  pop r7
  pop r8
  pop r9
  pop r10
  pop r11
  pop r12
  pop r13
  pop r14
  pop r15
  pop r16
  pop r17
  pop r28
  pop r29
  clr r1
  out __SREG__,r0
  ; Return true
  ldi r24,1
  ; Skip ret if the interrupt flag is set
  sbrs r0,7
  ret
  reti

.global task_start_trampoline
task_start_trampoline:
  ; Get entry point from callee saved context to ijmp source register Z
  movw r30, r2
  ; The stack pointer is always in r24w here
  movw r26,r24
  ; Get parameter from callee saved context to parameter register
  movw r24,r4
  ; Clear zero constant
  clr r1
  ijmp

#if 0
;  push r31
;  ldi r31,42
;  rjmp task_on_main_stack
; Caller loads handler number into r31
.global task_run_on_main_stack
task_run_on_main_stack:
  push r5
  push r4
  push r3
  push r2
  ; Save the stack pointer and status in callee saved registers
  in r2,__SP_L__
  in r3,__SP_H__
  in r4,__SREG__  
  ; Fetch main thread stack pointer
  lds r4,tasks
  lds r5,tasks+1
  ; Switch to main task's stack
  out __SP_H__,r5
  out __SP_L__,r4
  ; Push everything the compiler thinks it can clobber onto main stack
  push r0
  push r1
  push r18
  push r19
  push r20
  push r21
  push r22
  push r23
  push r24
  push r25
  push r26
  push r27
  ; the compiler preserves r28w
  push r30
  ; pushed r31 already, a while back
  movw r30,r24
  icall
  out __SP_H__,r3
  out __SP_L__,r2
  out __SREG__,r4
  pop r27
  pop r2
  pop r3
  pop r4
  pop r5
  pop r31
  reti
#endif
