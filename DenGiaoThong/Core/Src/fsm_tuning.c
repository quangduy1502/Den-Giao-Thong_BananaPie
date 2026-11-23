/*
 * fsm_tuning.c
 *
 *  Created on: 23 Nov 2026
 *      Author: Admin
 */

#include "fsm_tuning.h"
#include "global.h"
#include "button.h"
#include "traffic_light.h"
#include <stdbool.h>

#define BTN_CHANGE_MODE 0
#define BTN_SELECT 		1
#define BTN_INCREASE 	2
#define BTN_CONFIRM		3

#define TUNE_STEP 		1000

static void tune_show_color(traffic_color_t color){
	traffic_set(1, color);
	traffic_set(2, color);
}

static void tune_load(void) {
	switch(status) {
	case TUNE_RED:	 tuning_value = time_red;		break;
	case TUNE_GREEN: tuning_value = time_green; 	break;
	case TUNE_AMBER: tuning_value = time_amber;		break;
	default: break;
	}
}

static void tune_next_param(void) {
	switch(status) {
	case TUNE_RED: status = TUNE_GREEN; 	break;
	case TUNE_GREEN: status = TUNE_AMBER; 	break;
	case TUNE_AMBER: status = TUNE_RED; 	break;
	default: break;
	}

	tune_load();
}

static void tune_increase(void) {
	tuning_value += TUNE_STEP;
	if (tuning_value > TIME_MAX)
		tuning_value = TIME_MIN;
}

static bool tune_confirm(void) {
	uint32_t new_red;
	uint32_t new_green = time_green;
	uint32_t new_amber = time_amber;

	switch(status) {
	case TUNE_RED:
		if (tuning_value <= time_amber)	return false;
		new_red = tuning_value;
		new_green = tuning_value - time_amber;
		break;
	case TUNE_GREEN:
		new_green = tuning_value;
		new_red = new_green + new_amber;
		break;
	case TUNE_AMBER:
		new_amber = tuning_value;
		new_red = new_amber + new_green;
		break;
	default: return false;
	}

	if (new_red < TIME_MIN || new_red > TIME_MAX)	return false;
	if (new_green < TIME_MIN || new_green > TIME_MAX)	return false;
	if (new_amber < TIME_MIN || new_amber > TIME_MAX)	return false;

	time_red = new_red;
	time_green = new_green;
	time_amber = new_amber;

	return true;
}

static bool tune_check_exit(void) {
	if (isButtonPressed(BTN_CHANGE_MODE)) {
		next_status = AUTO_INIT;
		status = SWITCH_ALL_RED;

		return true;
	}

	return false;
}

void fsm_tuning_run(void) {
	switch(status) {
	case TUNE_INIT:
		mode = MODE_TUNING;
		for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
			(void)isButtonPressed(i); // xóa lần nhấn cũ
		}
		status = TUNE_RED;
		tune_load();
		break;

	 case TUNE_RED:
	case TUNE_GREEN:
	case TUNE_AMBER:
		// cả hai tuyến sáng màu đang chỉnh
		if (status == TUNE_RED)        tune_show_color(TL_RED);
		else if (status == TUNE_GREEN) tune_show_color(TL_GREEN);
		else                           tune_show_color(TL_AMBER);

		if (tune_check_exit()) break; // nút thoát kiểm tra trước

		if (isButtonPressed(BTN_SELECT)) {
			tune_next_param();
		}
		if (isButtonPressed(BTN_INCREASE)) {
			tune_increase();
		}
		if (isButtonPressed(BTN_CONFIRM)) {
			(void)tune_confirm(); // sai thì không lưu
			tune_load(); // nạp lại giá trị đã lưu (hoặc giá trị cũ)
		}
		break;

	default:
		break;
	}
}
