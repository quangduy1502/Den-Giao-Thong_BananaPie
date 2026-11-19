#include "button.h"
#include "main.h"

// số lần đọc liên tiếp để tính là nhấn giữ
#define LONG_PRESS_TICKS (BUTTON_LONG_PRESS_MS / BUTTON_SCAN_MS)

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} button_pin_t;

// chân của từng nút: 0 = BTN_1, 1 = BTN_2, 2 = BTN_3, 3 = BTN_4
static const button_pin_t button_pins[4] = {
    { BTN_1_GPIO_Port, BTN_1_Pin },
    { BTN_2_GPIO_Port, BTN_2_Pin },
    { BTN_3_GPIO_Port, BTN_3_Pin },
    { BTN_4_GPIO_Port, BTN_4_Pin },
};

static bool     prev_sample[NUM_BUTTONS];   // lần đọc trước
static bool     stable_state[NUM_BUTTONS];  // trạng thái nút sau khi chống dội
static uint16_t hold_ticks[NUM_BUTTONS];    // đếm thời gian giữ nút
static bool     pressed_flag[NUM_BUTTONS];  // cờ vừa nhấn
static bool     long_flag[NUM_BUTTONS];     // cờ nhấn giữ

// đọc chân nút, true nếu đang nhấn
static bool button_sample(uint8_t i) {
    GPIO_PinState level = HAL_GPIO_ReadPin(button_pins[i].port, button_pins[i].pin);
    return (level == (GPIO_PinState)BUTTON_ACTIVE_LEVEL);
}

// đặt lại tất cả về trạng thái ban đầu
void button_init(void) {
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        prev_sample[i] = false;
        stable_state[i] = false;
        hold_ticks[i] = 0;
        pressed_flag[i] = false;
        long_flag[i] = false;
    }
}

// đọc các nút, gọi mỗi 10 ms
void button_read(void) {
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        bool sample = button_sample(i);

        // chống dội: 2 lần đọc liên tiếp giống nhau mới tin
        if (sample == prev_sample[i]) {
            if (sample != stable_state[i]) {
                // trạng thái nút vừa đổi
                stable_state[i] = sample;
                hold_ticks[i] = 0;
                if (sample) {
                    pressed_flag[i] = true;      // vừa nhấn xuống
                }
            } else if (sample) {
                // nút đang được giữ, đếm thời gian giữ
                if (hold_ticks[i] < LONG_PRESS_TICKS) {
                    hold_ticks[i]++;
                    if (hold_ticks[i] == LONG_PRESS_TICKS) {
                        long_flag[i] = true;     // giữ đủ lâu, chỉ báo 1 lần
                    }
                }
            }
        }
        prev_sample[i] = sample;
    }
}

// nút i vừa được nhấn không (đọc xong thì xóa cờ)
bool isButtonPressed(uint8_t i) {
    if (i >= NUM_BUTTONS) return false;
    bool f = pressed_flag[i];
    pressed_flag[i] = false;
    return f;
}

// nút i đang được nhấn giữ không (đọc xong thì xóa cờ)
bool isButtonLongPressed(uint8_t i) {
    if (i >= NUM_BUTTONS) return false;
    bool f = long_flag[i];
    long_flag[i] = false;
    return f;
}
