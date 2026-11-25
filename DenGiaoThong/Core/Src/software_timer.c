#include "software_timer.h"

static uint32_t timer_counter[MAX_TIMERS] = {0};
uint8_t timer_flag[MAX_TIMERS] = {0};

void setTimer(uint8_t index, uint32_t duration_ms) {
    if (index >= MAX_TIMERS) return;
    timer_counter[index] = duration_ms / TIMER_TICK_MS;
    timer_flag[index] = (timer_counter[index] == 0) ? 1 : 0;
}

uint32_t timer_remaining_ms(uint8_t index) {
    if (index >= MAX_TIMERS) {
        return 0;
    }
    return timer_counter[index] * TIMER_TICK_MS;
}

void timer_run(void) {
    for (uint8_t i = 0; i < MAX_TIMERS; i++) {
        if (timer_counter[i] > 0) {
            timer_counter[i]--;
            if (timer_counter[i] == 0) {
                timer_flag[i] = 1;
            }
        }
    }
}
