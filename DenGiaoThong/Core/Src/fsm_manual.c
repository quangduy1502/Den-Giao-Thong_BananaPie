#include "fsm_manual.h"
#include "global.h"
#include "software_timer.h"
#include "button.h"
#include "traffic_light.h"
#include <stdbool.h>

// nút đổi chế độ và nút đảo tuyến trong chế độ bằng tay
#define BUTTON_CHANGE_MODE  0
#define BUTTON_TOGGLE_LINE  1

// nhấn nút 0 thì xin chuyển sang chế độ chỉnh thời gian (qua bước tất cả đỏ)
static bool manual_check_change_mode(void) {
    if (isButtonPressed(BUTTON_CHANGE_MODE)) {
        next_status = TUNE_INIT;
        status = SWITCH_ALL_RED;
        return true;
    }
    return false;
}

// chạy mỗi 10 ms: xử lý chế độ bằng tay và bước chuyển chế độ SWITCH_ALL_RED
void fsm_manual_run(void) {
    static bool all_red_started = false;

    switch (status) {

    case MAN_INIT:
        mode = MODE_MANUAL;
        (void)isButtonPressed(BUTTON_CHANGE_MODE);  // xóa lần nhấn cũ
        (void)isButtonPressed(BUTTON_TOGGLE_LINE);
        traffic_set(1, TL_GREEN);
        traffic_set(2, TL_RED);
        status = MAN_GREEN1_RED2;
        break;

    case MAN_GREEN1_RED2:
        traffic_set(1, TL_GREEN);
        traffic_set(2, TL_RED);
        if (manual_check_change_mode()) break;
        if (isButtonPressed(BUTTON_TOGGLE_LINE)) {
            status = MAN_RED1_GREEN2;
        }
        break;

    case MAN_RED1_GREEN2:
        traffic_set(1, TL_RED);
        traffic_set(2, TL_GREEN);
        if (manual_check_change_mode()) break;
        if (isButtonPressed(BUTTON_TOGGLE_LINE)) {
            status = MAN_GREEN1_RED2;
        }
        break;

    case SWITCH_ALL_RED:
        if (!all_red_started) {
            traffic_set(1, TL_RED);
            traffic_set(2, TL_RED);
            setTimer(TIMER_MANUAL, TIME_ALL_RED);
            all_red_started = true;
        }
        if (timer_flag[TIMER_MANUAL]) {
            all_red_started = false;
            status = next_status;
        }
        break;

    default:    // trạng thái của chế độ khác
        break;
    }
}
