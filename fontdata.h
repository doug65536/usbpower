#pragma once
#include <stdint.h>
#include <avr/pgmspace.h>

struct font_symbol {
  uint16_t id;
  uint16_t width;
  uint16_t height;
  uint16_t xofs;
  uint16_t yofs;
  uint16_t xadvance;
};

struct font_kern {
  uint16_t first;
  uint16_t second;
  int16_t advance;
};

struct font_info {
  uint8_t height;
  uint8_t glyph_count;
  font_symbol const * symbols;
  uint8_t const * runs;
  uint8_t kern_count;
  font_kern const * kerns;
  uint16_t const * offsets;
};

#define FONTDATA __attribute__((__section__(".progmem.data.fontdata")))
