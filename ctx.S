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

; void task_yield()
.global task_yield
.global task_yield_noreturn
task_yield:
task_yield_noreturn:
  in r0,__SREG__
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
  rcall task_cswitch
  out __SP_L__,r24
  out __SP_H__,r25
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
  eor r1,r1
  out __SREG__,r0
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
  eor r1,r1
  ijmp
