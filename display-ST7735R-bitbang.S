#include <avr/io.h>

#define SPI_D      DDRB
#define SPI_P      PORTB
#define SPI_B_CSX  3
#define SPI_B_SCL  2
#define SPI_B_SDA  1

; data direction must be configured before calling
; void disp_tx_ST7735R(uint8_t value (r24), bool dcx (r22), uint8_t base (r20))
; 88 cycles, not including call, 5.5 microseconds per byte, 181818 bytes/sec
.global disp_tx_ST7735R
disp_tx_ST7735R:
  ; New value
  mov r27,r20
  ; Read bit 0 of dcx
  bst r22,0
  ; Write SDA
  bld r26,SPI_B_SDA
  ; Update pins (setup SDA)
  out SPI_P,r26
  ; Raise SCL (data sampled clock rising)
  sbr r26,SPI_B_SCL
  ; Prepare loop
  ldi r27,8
  ; Update pins
  out SPI_P,r26

  ; 7 cycles so far

.Lbit_loop:
  ; Lower SCL
  cbr r26,SPI_B_SCL
  ; Read MSB (data bit)
  bst r24,7
  ; Write SDA
  bld r26,SPI_B_SDA
  ; Update pins (setup SDA)
  out SPI_P,r26
  ; Get ready to get next bit
  lsl r24
  ; Raise SCL
  sbr r26,SPI_B_SCL
  ; Update bit countdown
  subi r27,1
  ; Raise SCL (data sampled clock rising)
  out SPI_P,r26
  ; Loop if not done
  brne .Lbit_loop
  ; 7 + 79 = 84 cycles so far
  ret
  ; 84 + 4 = 88

; data direction must be configured before calling
; uint64_t disp_rx_ST7735R(uint8_t base) 
; 93 cycles per byte, 5.81us/byte, 172000 bytes/s
.global disp_rx_ST7735R
disp_rx_ST7735R:
  ldi r26,8
  mov r22,r24
  ; 
  ; Lower SCL
.Lrecv_bit_loop:
  cbr r22,SPI_B_SCL
  out SPI_P,r22
  ; Read SDA into carry
  in r0,SPI_P
  clc
  sbrc r0,SPI_B_SDA
  sec
  ; Raise SCL
  sbr r22,SPI_B_SCL
  out SPI_P,r22
  ; Rotate bit into result
  rol r24
  subi r26,1
  brne .Lrecv_bit_loop
  ret
  ; 112 cycles

  ; 132x162


  ; +----------------|---------+
  ; |                |         |
  ; |                |    Vset |
  ; |                | 12.000V |
  ; |              V |         |
  ; |                |      CV |
  ; +----------------+---------+
  ; |                |         |
  ; |                |    Iset |
  ; |                |    45mA |
  ; |              A |         |
  ; |                |      CC |
  ; +----------------+---------+
  ; |                |         |
  ; |                |    Wset |
  ; |                |     15W |
  ; |              W |         |
  ; |                |      CW |
  ; +---------+------+---------+
  ; | history | 54mW |         |
  ; +---------+------+---------+
  ; |         | 4mWh |         |
  ; +---------+------+---------+
  ; +---------+------+---------+
  ; | config  | monitor | 
  ; +---------+------+---------+
