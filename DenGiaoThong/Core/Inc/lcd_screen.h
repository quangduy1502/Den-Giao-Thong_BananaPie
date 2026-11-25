#ifndef INC_LCD_SCREEN_H_
#define INC_LCD_SCREEN_H_

#include "dht20.h"
#include "lcd.h"

/* Build the two space-padded LCD rows from the current FSM and DHT20 sample. */
void lcd_screen_format(char lines[LCD_ROWS][LCD_COLS + 1U],
                       const DHT20_HandleTypeDef *dht);

#endif /* INC_LCD_SCREEN_H_ */
