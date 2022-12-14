#include "display.h"
#include <stdint.h>
#include <avr/io.h>

  // +----------------|---------+
  // |                |         |
  // |                |    Vset |
  // |                | 12.000V |
  // |              V |         |
  // |                |      CV |
  // +----------------+---------+
  // |                |         |
  // |                |    Iset |
  // |                |    45mA |
  // |              A |         |
  // |                |      CC |
  // +----------------+---------+
  // |                |         |
  // |                |    Wset |
  // |                |     15W |
  // |              W |         |
  // |                |      CW |
  // +---------+------+---------+
  // | history | 54mW |         |
  // +---------+------+---------+
  // |         | 4mWh |         |
  // +---------+------+---------+
  // |                          |
  // +---------+---------+------+
  // | config  | monitor |      |
  // +---------+---------+------+

// commands 
