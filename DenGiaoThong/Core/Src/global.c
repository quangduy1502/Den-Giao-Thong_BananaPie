#include "global.h"

// bắt đầu ở chế độ tự động, đi qua AUTO_INIT để đặt đèn và timer
system_mode_t   mode        = MODE_AUTO;
system_status_t status      = AUTO_INIT;
system_status_t next_status = AUTO_INIT;

// thời gian đèn (ms)
uint32_t time_red   = DEFAULT_TIME_RED;
uint32_t time_green = DEFAULT_TIME_GREEN;
uint32_t time_amber = DEFAULT_TIME_AMBER;
