#include "fsm_automatic.h"
#include "global.h"
#include "software_timer.h"
#include "button.h"
#include "traffic_light.h"
#include <stdbool.h>

// nút đổi sang chế độ bằng tay
#define BUTTON_CHANGE_MODE  0

// nhấn nút thì xin đổi sang chế độ bằng tay (qua bước tất cả đỏ)
static bool auto_check_change_mode(void) {
    if (isButtonPressed(BUTTON_CHANGE_MODE)) {
        next_status = MAN_INIT;
        status = SWITCH_ALL_RED;
        return true;
    }
    return false;
}

static void auto_start_phase(system_status_t phase, uint8_t active_line,
                             traffic_color_t color, uint32_t duration_ms) {
    traffic_set(active_line == 1 ? 2 : 1, TL_RED);
    traffic_set(active_line, color);
    setTimer(TIMER_AUTO, duration_ms);
    status = phase;
}

// chạy mỗi 10 ms
void fsm_automatic_run(void) {
    switch (status) {

    case AUTO_INIT:
        mode = MODE_AUTO;
        (void)isButtonPressed(BUTTON_CHANGE_MODE);  // xóa lần nhấn cũ
        auto_start_phase(AUTO_RED1_GREEN2, 2, TL_GREEN, time_green);
        break;

    case AUTO_RED1_GREEN2:
        if (!auto_check_change_mode() && timer_flag[TIMER_AUTO]) {
            auto_start_phase(AUTO_RED1_AMBER2, 2, TL_AMBER, time_amber);
        }
        break;

    case AUTO_RED1_AMBER2:
        if (!auto_check_change_mode() && timer_flag[TIMER_AUTO]) {
            auto_start_phase(AUTO_GREEN1_RED2, 1, TL_GREEN, time_green);
        }
        break;

    case AUTO_GREEN1_RED2:
        if (!auto_check_change_mode() && timer_flag[TIMER_AUTO]) {
            auto_start_phase(AUTO_AMBER1_RED2, 1, TL_AMBER, time_amber);
        }
        break;

    case AUTO_AMBER1_RED2:
        if (!auto_check_change_mode() && timer_flag[TIMER_AUTO]) {
            auto_start_phase(AUTO_RED1_GREEN2, 2, TL_GREEN, time_green);
        }
        break;

    default:    // trạng thái của chế độ khác
        break;
    }
}
