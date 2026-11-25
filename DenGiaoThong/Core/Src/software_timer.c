#include "software_timer.h"
#include "scheduler.h"

static uint32_t started_at[MAX_TIMERS];
static uint32_t duration_ticks[MAX_TIMERS];
uint8_t timer_flag[MAX_TIMERS] = {0};

static uint32_t remaining_ticks(uint8_t index, uint32_t now) {
    uint32_t elapsed = now - started_at[index];
    return elapsed < duration_ticks[index] ? duration_ticks[index] - elapsed : 0;
}

void setTimer(uint8_t index, uint32_t duration_ms) {
    if (index >= MAX_TIMERS) return;
    started_at[index] = SCH_Get_Tick();
    duration_ticks[index] = duration_ms / TIMER_TICK_MS;
    timer_flag[index] = (duration_ticks[index] == 0) ? 1 : 0;
}

uint32_t timer_remaining_ms(uint8_t index) {
    if (index >= MAX_TIMERS) {
        return 0;
    }
    return remaining_ticks(index, SCH_Get_Tick()) * TIMER_TICK_MS;
}

void timer_run(void) {
    uint32_t now = SCH_Get_Tick();
    for (uint8_t i = 0; i < MAX_TIMERS; i++) {
        if (duration_ticks[i] > 0 && remaining_ticks(i, now) == 0) {
            duration_ticks[i] = 0;
            timer_flag[i] = 1;
        }
    }
}
