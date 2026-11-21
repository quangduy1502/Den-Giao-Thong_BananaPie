#include "lcd.h"

static void lcd_delay_us(uint32_t us);
static HAL_StatusTypeDef lcd_expander_write(LCD_HandleTypeDef *lcd, uint8_t value);
static HAL_StatusTypeDef lcd_write4bits(LCD_HandleTypeDef *lcd, uint8_t value);
static HAL_StatusTypeDef lcd_command(LCD_HandleTypeDef *lcd, uint8_t command);


static void lcd_delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;

    uint32_t cycles = (SystemCoreClock / 1000000U) * us;

    while ((uint32_t)(DWT->CYCCNT - start) < cycles) {}
}

static HAL_StatusTypeDef lcd_expander_write(LCD_HandleTypeDef *lcd, uint8_t value) {
    if (lcd == NULL || lcd->hi2c == NULL) {
        return HAL_ERROR;
    }

    uint8_t data = value | lcd->backlight;

    return HAL_I2C_Master_Transmit(lcd->hi2c, LCD_I2C_ADDR << 1, &data, 1, 10);
}

static HAL_StatusTypeDef lcd_write4bits(LCD_HandleTypeDef *lcd, uint8_t value) {
    uint8_t data = value & (uint8_t)~LCD_EN;

    HAL_StatusTypeDef status;
    status = lcd_expander_write(lcd, data);
    if (status != HAL_OK) {
        return status;
    }

    status = lcd_expander_write(lcd, data | LCD_EN);
    if (status != HAL_OK) {
        return status;
    }
    lcd_delay_us(1);

    status = lcd_expander_write(lcd, data);
    if (status != HAL_OK) {
        return status;
    }
    lcd_delay_us(50);

    return HAL_OK;
}

static HAL_StatusTypeDef lcd_command(LCD_HandleTypeDef *lcd, uint8_t command) {
    uint8_t high = command & 0xF0U;
    uint8_t low = (command << 4) & 0xF0U;

    HAL_StatusTypeDef status;

    status = lcd_write4bits(lcd, high);
    if (status != HAL_OK) {
        return status;
    }

    status = lcd_write4bits(lcd, low);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_OK;
}

HAL_StatusTypeDef lcd_init(LCD_HandleTypeDef *lcd, I2C_HandleTypeDef *hi2c)
{
    if (lcd == NULL || hi2c == NULL) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status;

    lcd->hi2c = hi2c;
    lcd->backlight = LCD_BACKLIGHT;
    lcd->display_function = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;

    /* Wait for LCD power to stabilize. */
    HAL_Delay(50);

    /* RS = RW = EN = 0; backlight stays on. */
    status = lcd_expander_write(lcd, 0);
    if (status != HAL_OK) {
        return status;
    }

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    /* Reset sequence: send only one nibble each time. */
    for (uint8_t i = 0; i < 3; i++) {
        status = lcd_write4bits(lcd, 0x30U);
        if (status != HAL_OK) {
            return status;
        }

        HAL_Delay(5);
    }

    /* Switch to 4-bit interface. */
    status = lcd_write4bits(lcd, 0x20U);
    if (status != HAL_OK) {
        return status;
    }
    HAL_Delay(1);

    /* Configure 4-bit interface, 2 lines, 5x8 font. */
    status = lcd_command(
        lcd,
        LCD_FUNCTIONSET | lcd->display_function
    );
    if (status != HAL_OK) {
        return status;
    }

    /* Turn display off while configuring it. */
    status = lcd_command(lcd, LCD_DISPLAYCONTROL | LCD_DISPLAYOFF);
    if (status != HAL_OK) {
        return status;
    }

    /* Clear display; this instruction needs a longer wait. */
    status = lcd_clear(lcd);
    if (status != HAL_OK) {
        return status;
    }

    /* Write left to right, without shifting the display. */
    lcd->display_mode =
        LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

    status = lcd_command(
        lcd,
        LCD_ENTRYMODESET | lcd->display_mode
    );
    if (status != HAL_OK) {
        return status;
    }

    /* Turn display on, with cursor and blinking disabled. */
    lcd->display_control =
        LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;

    return lcd_command(
        lcd,
        LCD_DISPLAYCONTROL | lcd->display_control
    );
}

HAL_StatusTypeDef lcd_write_char(LCD_HandleTypeDef *lcd, char character) {
    uint8_t value = (uint8_t)character;

    uint8_t high = (value & 0xF0U) | LCD_RS;
    uint8_t low = ((value << 4) & 0xF0U) | LCD_RS;

    HAL_StatusTypeDef status;

    status = lcd_write4bits(lcd, high);
    if (status != HAL_OK) {
        return status;
    }

    status = lcd_write4bits(lcd, low);
    if (status != HAL_OK) {
        return status;
    }

    return HAL_OK;
}

HAL_StatusTypeDef lcd_set_cursor(LCD_HandleTypeDef *lcd, uint8_t col, uint8_t row) {
    static const uint8_t row_offset[LCD_ROWS] = {0x00U, 0x40U};

    if (row >= LCD_ROWS || col >= LCD_COLS) {
        return HAL_ERROR;
    }

    uint8_t address = row_offset[row] + col;

    return lcd_command(lcd, LCD_SETDDRAMADDR | address);
}

HAL_StatusTypeDef lcd_print(LCD_HandleTypeDef *lcd, const char *text) {
    if (lcd == NULL || lcd->hi2c == NULL || text == NULL) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status;

    while (*text != '\0') {
        status = lcd_write_char(lcd, *text);
        if (status != HAL_OK) {
            return status;
        }
        ++text;
    }

    return HAL_OK;
}

HAL_StatusTypeDef lcd_clear(LCD_HandleTypeDef *lcd) {
    HAL_StatusTypeDef status;

    status = lcd_command(lcd, LCD_CLEARDISPLAY);
    if (status != HAL_OK) {
        return status;
    }
    lcd_delay_us(2000);

    return HAL_OK;
}

HAL_StatusTypeDef lcd_home(LCD_HandleTypeDef *lcd) {
    HAL_StatusTypeDef status;

    status = lcd_command(lcd, LCD_RETURNHOME);
    if (status != HAL_OK) {
        return status;
    }
    lcd_delay_us(2000);

    return HAL_OK;
}

HAL_StatusTypeDef lcd_set_backlight(LCD_HandleTypeDef *lcd, uint8_t enabled) {
    if (lcd == NULL || lcd->hi2c == NULL) {
        return HAL_ERROR;
    }

    uint8_t previous = lcd->backlight;

    if (enabled) {
        lcd->backlight = LCD_BACKLIGHT;
    } else {
        lcd->backlight = LCD_NOBACKLIGHT;
    }

    HAL_StatusTypeDef status = lcd_expander_write(lcd, 0);
    if (status != HAL_OK) {
        lcd->backlight = previous;
    }
    return status;
}

HAL_StatusTypeDef lcd_set_display(LCD_HandleTypeDef *lcd, uint8_t enabled) {
    if (lcd == NULL || lcd->hi2c == NULL) {
        return HAL_ERROR;
    }

    uint8_t control = lcd->display_control;
    if (enabled) {
        control |= LCD_DISPLAYON;
    } else {
        control &= (uint8_t)~LCD_DISPLAYON;
    }

    HAL_StatusTypeDef status = lcd_command(lcd, LCD_DISPLAYCONTROL | control);
    if (status == HAL_OK) {
        lcd->display_control = control;
    }
    return status;
}

HAL_StatusTypeDef lcd_show_cursor(LCD_HandleTypeDef *lcd, uint8_t enabled) {
    if (lcd == NULL || lcd->hi2c == NULL) {
        return HAL_ERROR;
    }

    uint8_t control = lcd->display_control;
    if (enabled) {
        control |= LCD_CURSORON;
    } else {
        control &= (uint8_t)~LCD_CURSORON;
    }

    HAL_StatusTypeDef status = lcd_command(lcd, LCD_DISPLAYCONTROL | control);
    if (status == HAL_OK) {
        lcd->display_control = control;
    }
    return status;
}

HAL_StatusTypeDef lcd_set_blink(LCD_HandleTypeDef *lcd, uint8_t enabled) {
    if (lcd == NULL || lcd->hi2c == NULL) {
        return HAL_ERROR;
    }

    uint8_t control = lcd->display_control;
    if (enabled) {
        control |= LCD_BLINKON;
    } else {
        control &= (uint8_t)~LCD_BLINKON;
    }

    HAL_StatusTypeDef status = lcd_command(lcd, LCD_DISPLAYCONTROL | control);
    if (status == HAL_OK) {
        lcd->display_control = control;
    }
    return status;
}

// direction = 0 : shift left && direction != 0 : shift right
HAL_StatusTypeDef lcd_scroll(LCD_HandleTypeDef *lcd, uint8_t direction) {
    if (lcd == NULL || lcd->hi2c == NULL) {
        return HAL_ERROR;
    }

    uint8_t command = LCD_CURSORSHIFT | LCD_DISPLAYMOVE;

    if (direction) {
        command |= LCD_MOVERIGHT;
    } else {
        command |= LCD_MOVELEFT;
    }

    return lcd_command(lcd, command);
}

// direction = 0 : to left && direction != 0 : to right
HAL_StatusTypeDef lcd_set_text_direction(LCD_HandleTypeDef *lcd, uint8_t direction) {
    if (lcd == NULL || lcd->hi2c == NULL) {
        return HAL_ERROR;
    }

    uint8_t mode = lcd->display_mode;
    if (direction) {
        mode |= LCD_ENTRYLEFT;
    } else {
        mode &= (uint8_t)~LCD_ENTRYLEFT;
    }

    HAL_StatusTypeDef status = lcd_command(lcd, LCD_ENTRYMODESET | mode);
    if (status == HAL_OK) {
        lcd->display_mode = mode;
    }
    return status;
}

HAL_StatusTypeDef lcd_set_autoscroll(LCD_HandleTypeDef *lcd, uint8_t enabled) {
    if (lcd == NULL || lcd->hi2c == NULL) {
        return HAL_ERROR;
    }

    uint8_t mode = lcd->display_mode;
    if (enabled) {
        mode |= LCD_ENTRYSHIFTINCREMENT;
    } else {
        mode &= (uint8_t)~LCD_ENTRYSHIFTINCREMENT;
    }

    HAL_StatusTypeDef status = lcd_command(lcd, LCD_ENTRYMODESET | mode);
    if (status == HAL_OK) {
        lcd->display_mode = mode;
    }
    return status;
}

HAL_StatusTypeDef lcd_create_char(LCD_HandleTypeDef *lcd, uint8_t slot, const uint8_t bitmap[8]) {
    if (lcd == NULL || lcd->hi2c == NULL || slot >= 8 || bitmap == NULL) {
        return HAL_ERROR;
    }

    const uint8_t mode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    HAL_StatusTypeDef status;

    /* CGRAM rows must be written in increasing address order. */
    if (lcd->display_mode != mode) {
        status = lcd_command(lcd, LCD_ENTRYMODESET | mode);
        if (status != HAL_OK) {
            return status;
        }
    }

    status = lcd_command(lcd, LCD_SETCGRAMADDR | (slot << 3));
    if (status == HAL_OK) {
        for (uint8_t i = 0; i < 8; ++i) {
            status = lcd_write_char(lcd, (char)(bitmap[i] & 0x1FU));
            if (status != HAL_OK) {
                break;
            }
        }
    }

    /* Restore entry mode even when a CGRAM transfer failed. */
    if (lcd->display_mode != mode) {
        HAL_StatusTypeDef restore = lcd_command(lcd, LCD_ENTRYMODESET | lcd->display_mode);
        if (status == HAL_OK) {
            status = restore;
        }
    }
    return status;
}

uint8_t lcd_get_backlight(const LCD_HandleTypeDef *lcd) {
    if (lcd == NULL) {
        return 0;
    }

    return lcd->backlight == LCD_BACKLIGHT;
}
