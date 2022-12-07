#pragma once

#include <stdint.h>

void timer_init();
void timer_wait_for_ms(uint64_t wait_millisec);
void timer_wait_until(uint64_t wakeup_millisec);
