#pragma once

#include <stdint.h>

void timer_init();
void timer_wait_for_ms(uint32_t wait_millisec);
void timer_wait_until(uint32_t wakeup_millisec);
void timer_tick();
