#include "traffic_light.h"
#include "main.h"

typedef struct {
    GPIO_TypeDef *port_a;
    uint16_t pin_a;
    GPIO_TypeDef *port_b;
    uint16_t pin_b;
} traffic_pins_t;

// chân của từng tuyến: tuyến 1 dùng TL1_A, TL1_B; tuyến 2 dùng TL2_A, TL2_B
static const traffic_pins_t traffic_pins[2] = {
    { TL1_A_GPIO_Port, TL1_A_Pin, TL1_B_GPIO_Port, TL1_B_Pin },
    { TL2_A_GPIO_Port, TL2_A_Pin, TL2_B_GPIO_Port, TL2_B_Pin },
};

// mức chân A, B cho từng màu (thứ tự: đỏ, xanh, vàng, tắt)
static const uint8_t traffic_combo[4][2] = {
    // A, B
    { 1, 1 },   // đỏ
    { 1, 0 },   // xanh
    { 0, 1 },   // vàng
    { 0, 0 },   // tắt
};

// đặt màu cho một tuyến
void traffic_set(uint8_t line, traffic_color_t color) {
    // tuyến hoặc màu sai thì bỏ qua
    if (line < 1 || line > NUM_LINES || line > 2) return;
    if ((uint8_t)color > TL_OFF) return;

    const traffic_pins_t *p = &traffic_pins[line - 1];
    HAL_GPIO_WritePin(p->port_a, p->pin_a,
                      traffic_combo[color][0] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(p->port_b, p->pin_b,
                      traffic_combo[color][1] ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// tắt đèn của tất cả các tuyến
void traffic_init(void) {
    for (uint8_t line = 1; line <= NUM_LINES; line++) {
        traffic_set(line, TL_OFF);
    }
}
