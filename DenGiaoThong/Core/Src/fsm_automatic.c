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

// chạy mỗi 10 ms
void fsm_automatic_run(void) {
    switch (status) {

    case AUTO_INIT:
        mode = MODE_AUTO;
        (void)isButtonPressed(BUTTON_CHANGE_MODE);  // xóa lần nhấn cũ
        traffic_set(1, TL_RED);
        traffic_set(2, TL_GREEN);
        setTimer(TIMER_AUTO, time_green);
        status = AUTO_RED1_GREEN2;
        break;

    case AUTO_RED1_GREEN2:
        traffic_set(1, TL_RED);
        traffic_set(2, TL_GREEN);
        if (auto_check_change_mode()) break;
        if (timer_flag[TIMER_AUTO]) {               // hết giờ xanh
            status = AUTO_RED1_AMBER2;
            setTimer(TIMER_AUTO, time_amber);
        }
        break;

    case AUTO_RED1_AMBER2:
        traffic_set(1, TL_RED);
        traffic_set(2, TL_AMBER);
        if (auto_check_change_mode()) break;
        if (timer_flag[TIMER_AUTO]) {               // hết giờ vàng
            status = AUTO_GREEN1_RED2;
            setTimer(TIMER_AUTO, time_green);
        }
        break;

    case AUTO_GREEN1_RED2:
        traffic_set(1, TL_GREEN);
        traffic_set(2, TL_RED);
        if (auto_check_change_mode()) break;
        if (timer_flag[TIMER_AUTO]) {               // hết giờ xanh
            status = AUTO_AMBER1_RED2;
            setTimer(TIMER_AUTO, time_amber);
        }
        break;

    case AUTO_AMBER1_RED2:
        traffic_set(1, TL_AMBER);
        traffic_set(2, TL_RED);
        if (auto_check_change_mode()) break;
        if (timer_flag[TIMER_AUTO]) {               // hết giờ vàng
            status = AUTO_RED1_GREEN2;
            setTimer(TIMER_AUTO, time_green);
        }
        break;

    default:    // trạng thái của chế độ khác
        break;
    }
}
