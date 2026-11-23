#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include <stdint.h>

// chế độ hiện tại của hệ thống
typedef enum {
    MODE_AUTO = 0,
    MODE_MANUAL,
    MODE_TUNING
} system_mode_t;

// các trạng thái của hệ thống (dùng chung cho cả 3 FSM)
typedef enum {
    // chế độ tự động
    AUTO_INIT = 0,          // vào chế độ tự động (đặt lại đèn và timer)
    AUTO_RED1_GREEN2,
	AUTO_RED1_AMBER2,
    AUTO_GREEN1_RED2,
    AUTO_AMBER1_RED2,

    // chế độ bằng tay
    MAN_INIT,
    MAN_GREEN1_RED2,
    MAN_RED1_GREEN2,

    // chế độ chỉnh thời gian
    TUNE_INIT,
    TUNE_RED,
    TUNE_GREEN,
    TUNE_AMBER,

    // chuyển chế độ
    SWITCH_ALL_RED          // giữ cả hai tuyến đỏ trước khi vào chế độ mới
} system_status_t;

// đỏ = xanh + vàng
#define DEFAULT_TIME_RED    5000
#define DEFAULT_TIME_GREEN  3000
#define DEFAULT_TIME_AMBER  2000

// giới hạn khi chỉnh thời gian (ms)
#define TIME_MIN            1000
#define TIME_MAX            99000

// thời gian giữ cả hai tuyến đỏ khi đổi chế độ (ms)
#define TIME_ALL_RED        1000

// chỉ số software timer của từng FSM (tránh dùng trùng)
#define TIMER_AUTO          0
#define TIMER_MANUAL        1
#define TIMER_TUNING        2

extern system_mode_t mode; // chế độ hiện tại
extern system_status_t status; // trạng thái hiện tại
extern system_status_t next_status;// trạng thái sẽ vào sau SWITCH_ALL_RED
extern uint32_t time_red; // thời gian đèn đỏ (ms)
extern uint32_t time_green; // thời gian đèn xanh (ms)
extern uint32_t time_amber; // thời gian đèn vàng (ms)
extern uint32_t tuning_value; // giá trị đang tune

#endif /* INC_GLOBAL_H_ */
