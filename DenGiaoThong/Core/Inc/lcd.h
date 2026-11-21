#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* HD44780 16x2 with a PCF8574 backpack; unshifted 7-bit address.
 * P0=RS, P1=RW, P2=EN, P3=backlight, P4..P7=D4..D7.
 * The project configures I2C1 on PB8 (SCL) and PB9 (SDA).
 */
#define LCD_I2C_ADDR 0x21U
#define LCD_COLS     16U
#define LCD_ROWS      2U

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define LCD_EN 0x04U //Enable bit
#define LCD_RW 0x02U //Read/Write bit
#define LCD_RS 0x01U //Registor select bit

typedef struct {
    I2C_HandleTypeDef *hi2c;

    uint8_t backlight;
    uint8_t display_control;
    uint8_t display_mode;
    uint8_t display_function;
} LCD_HandleTypeDef;

/* Call lcd_init once after HAL/clock/I2C initialization, before the scheduler.
 * Transfers are blocking. Short waits use the Cortex-M3 DWT cycle counter;
 * millisecond waits occur only during initialization.
 */
HAL_StatusTypeDef lcd_init(LCD_HandleTypeDef *lcd, I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef lcd_write_char(LCD_HandleTypeDef *lcd, char character);
/* Coordinates start at zero. lcd_print does not wrap or interpret newlines. */
HAL_StatusTypeDef lcd_set_cursor(LCD_HandleTypeDef *lcd, uint8_t col, uint8_t row);
HAL_StatusTypeDef lcd_print(LCD_HandleTypeDef *lcd, const char *text);
HAL_StatusTypeDef lcd_clear(LCD_HandleTypeDef *lcd);
HAL_StatusTypeDef lcd_home(LCD_HandleTypeDef *lcd);
HAL_StatusTypeDef lcd_set_backlight(LCD_HandleTypeDef *lcd, uint8_t enabled);
HAL_StatusTypeDef lcd_set_display(LCD_HandleTypeDef *lcd, uint8_t enabled);
HAL_StatusTypeDef lcd_show_cursor(LCD_HandleTypeDef *lcd, uint8_t enabled);
HAL_StatusTypeDef lcd_set_blink(LCD_HandleTypeDef *lcd, uint8_t enabled);
HAL_StatusTypeDef lcd_scroll(LCD_HandleTypeDef *lcd, uint8_t direction);
HAL_StatusTypeDef lcd_set_text_direction(LCD_HandleTypeDef *lcd, uint8_t direction);
HAL_StatusTypeDef lcd_set_autoscroll(LCD_HandleTypeDef *lcd, uint8_t enabled);
/* Slots 0..7, eight rows of five pixels. Restores entry mode after writing.
 * Call lcd_set_cursor afterward to select DDRAM before printing text.
 */
HAL_StatusTypeDef lcd_create_char(LCD_HandleTypeDef *lcd, uint8_t slot, const uint8_t bitmap[8]);

/* Returns the last successfully requested state, not hardware feedback. */
uint8_t lcd_get_backlight(const LCD_HandleTypeDef *lcd);
#endif
