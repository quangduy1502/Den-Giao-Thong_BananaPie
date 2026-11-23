#include "global.h"

system_mode_t mode        = MODE_AUTO;
system_status_t status      = AUTO_INIT;
system_status_t next_status = AUTO_INIT;

uint32_t time_red   = DEFAULT_TIME_RED;
uint32_t time_green = DEFAULT_TIME_GREEN;
uint32_t time_amber = DEFAULT_TIME_AMBER;
uint32_t tuning_value = DEFAULT_TIME_RED;
