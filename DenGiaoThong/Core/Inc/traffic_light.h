#ifndef INC_TRAFFIC_LIGHT_H_
#define INC_TRAFFIC_LIGHT_H_

#include <stdint.h>

// số tuyến đèn đang dùng (tối đa 2)
#ifndef NUM_LINES
#define NUM_LINES 2
#endif

// các màu đèn
typedef enum {
    TL_RED = 0,
    TL_GREEN,
    TL_AMBER,
    TL_OFF
} traffic_color_t;

void traffic_init(void);                               // tắt hết đèn
void traffic_set(uint8_t line, traffic_color_t color); // đặt màu cho tuyến line (1 hoặc 2)

#endif /* INC_TRAFFIC_LIGHT_H_ */
