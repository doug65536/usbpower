// clk.h

#include <stdint.h>

// Set clock to 16MHz / (1<<log2_divisor)
// 0=16MHz, 1=8MHz, 2=4MHz, ... 7=64kHz.
// Behaviour is undefined if 
void clk_divisor(uint8_t log2_divisor);
