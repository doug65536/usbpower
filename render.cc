#include "render.h"
#include "fontdata.h"
#include "display-ST7735R-bitbang.h"

// The display is enabled when CSX is low
// When CSX goes high, 


extern font_info liberation_sans_narrow_bold_48_font_info;
extern font_info liberation_sans_narrow_regular_16_font_info;

using coord = uint8_t;

static void st7735_caset(coord sx, coord ex, uint8_t base)
{
  disp_tx_ST7735R(0x2A, false, base);
  disp_tx_ST7735R(sx >> 8, true, base);
  disp_tx_ST7735R(sx, true, base);
  disp_tx_ST7735R(ex >> 8, true, base);
  disp_tx_ST7735R(ex, true, base);
}

static void st7735_raset(coord sy, coord ey, uint8_t base)
{
  disp_tx_ST7735R(0x2B, false, base);
  disp_tx_ST7735R(sy >> 8, true, base);
  disp_tx_ST7735R(sy, true, base);
  disp_tx_ST7735R(ey >> 8, true, base);
  disp_tx_ST7735R(ey, true, base);
}

static void st7735_write(uint8_t base)
{
  disp_tx_ST7735R(0x2C, false, base);
}

static uint8_t st7735_lookup_char(font_info const *font, uint16_t codepoint)
{
  size_t st = 0;
  size_t en = font->glyph_count;
  while (st < en) {
    size_t md = ((en - st) >> 1) + st;
    if (font->symbols[md].id < codepoint)
      st = md + 1;
    else if (font->symbols[md].id > codepoint)
      en = md;
    else
      return md;
  }
  return 0xFF;
}

static uint8_t st8835_lookup_kern(font_info const *font, 
  uint16_t first, uint16_t second)
{
  size_t st = 0;
  size_t en = font->kern_count;
  while (st < en) {
    size_t md = ((en - st) >> 1) + st;
    if (font->kerns[md].first < first)
      st = md + 1;
    else if (font->kerns[md].first > first)
      en = md;
    else if (font->kerns[md].second < second)
      st = md + 1;
    else if (font->kerns[md].second > second)
      en = md;
    else 
      return md;
  }
  return 0xff;
}

static uint16_t utf8_decode(char const *&p)
{
  uint16_t result;

  if ((p[0] & 0x80) == 0)
    result = *p++;
  else if ((p[0] & 0xE0) == 0xC0 && 
      (p[1] & 0xC0) == 0x80)
    result = (p[1] & 0x3F) | ((p[0] & 0x1F) << 6);
  else if ((p[0] & 0xF0) == 0xE0 && 
      (p[1] & 0xC0) == 0x80 && 
      (p[2] & 0xC0) == 0x80)
    result = (p[2] & 0x3F) | ((p[1] & 0xF) << 6) | ((p[0] & 0x0F) << 12);
  else
    result = '?';
  
  return result;
}

static coord st7735_width(font_info const *font,
  char const *text, size_t sz)
{
  char const *input = text;
  char const *end = text + sz;
  size_t total = 0;  
  uint16_t this_char = (input < end) ? utf8_decode(input) : 0;
  uint16_t next_char = (input < end) ? utf8_decode(input) : 0;
  while (this_char) {
    uint8_t char_index = st7735_lookup_char(font, this_char);
    total += font->symbols[char_index].xadvance;

    if (next_char) {
      // Lookup kern
      uint8_t kern_index = st8835_lookup_kern(font, this_char, next_char);

      // Add the tweak if found
      if (kern_index != 0xff)
        total += font->kerns[kern_index].advance;
    }

    this_char = next_char;
    next_char = (input < end) ? utf8_decode(input) : 0;
  }

  return total < 255U ? total : 255U;
}

static void st7735_fill(uint16_t color, uint16_t count, uint8_t base)
{
  uint8_t r = color & 0x0F;
  uint8_t g = (color >> 4) & 0x0F;
  uint8_t b = (color >> 8) & 0x0F;
  uint8_t rg = r | (g << 4);
  uint8_t br = b | (r << 4);
  uint8_t gb = g | (b << 4);

  while (count) {
    disp_tx_ST7735R(rg, true, base);
    disp_tx_ST7735R(br, true, base);    
    if (--count) {
      disp_tx_ST7735R(gb, true, base);
      --count;
    }
  }

  // Only complete data will be sent to video memory
  // On odd pixel count, the last br will not be sent
}

void render_rect(coord sx, coord sy, coord ex, coord ey, 
  uint16_t color, uint8_t base)
{
  coord w = ex - sx;
  coord h = ey - sy;
  uint16_t sz = w * h;
  st7735_caset(sx, ex, base);
  st7735_raset(sy, ey, base);
  st7735_fill(color, sz, base);
}

void render_tl_text(font_info const *font,
  char const *text, size_t sz,
  coord x, coord y, size_t w = 0)
{
  if (!w)
    w = st7735_width(font, text, sz);
}

void render_rb_text(font_info const *font,
  char const *text, size_t sz,
  coord ex, coord ey, uint8_t w = 0)
{
  if (!w)
    w = st7735_width(font, text, sz);
  coord x = ex - w;
  coord y = ey - font->height;
  render_tl_text(font, text, sz, x, y, w);
}

void render_init()
{
}
