#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include <stdint.h>
#include <stdbool.h>

// số nút đang dùng (tối đa 4)
#ifndef NUM_BUTTONS
#define NUM_BUTTONS 4
#endif

// mức logic khi nhấn nút (có pull-up nên nhấn = 0)
#define BUTTON_ACTIVE_LEVEL 0

// thời gian kiểm tra nút (ms)
#define BUTTON_SCAN_MS       10
// thời gian giữ nút để tính là nhấn giữ (ms)
#define BUTTON_LONG_PRESS_MS 1000

void button_init(void);                  // đặt lại trạng thái các nút
void button_read(void);                  // đọc nút, chạy mỗi 10 ms
bool isButtonPressed(uint8_t i);         // nút i vừa được nhấn (đọc xong tự xóa)
bool isButtonLongPressed(uint8_t i);     // nút i được nhấn giữ (đọc xong tự xóa)

#endif /* INC_BUTTON_H_ */
