#pragma once
#ifndef __ASSEMBLER__
#include <stdint.h>

extern "C" void disp_tx_ST7735R(uint8_t value, bool dcx, uint8_t base);
extern "C" uint64_t disp_rx_ST7735R(uint8_t base);
#endif

#define SPI_D      DDRB
#define SPI_P      PORTB
#define SPI_B_CSX  3
#define SPI_B_SCL  2
#define SPI_B_SDA  1
