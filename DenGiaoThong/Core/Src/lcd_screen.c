#include "lcd_screen.h"

#include "global.h"
#include "software_timer.h"
#include <stdio.h>
#include <string.h>

static unsigned int seconds_remaining(uint32_t milliseconds) {
    return (unsigned int)(milliseconds / 1000U + (milliseconds % 1000U != 0U));
}

static void format_auto(char *line, char color1, char color2) {
    uint32_t phase_ms = timer_remaining_ms(TIMER_AUTO);
    uint32_t red_ms = phase_ms;
    if (color1 == 'G' || color2 == 'G') {
        /* The opposite line stays red through both the green and amber phases. */
        red_ms += time_amber;
    }

    unsigned int remaining1 = seconds_remaining(color1 == 'R' ? red_ms : phase_ms);
    unsigned int remaining2 = seconds_remaining(color2 == 'R' ? red_ms : phase_ms);
    snprintf(line, LCD_COLS + 1U, "AUTO 1:%c%02u 2:%c%02u",
             color1, remaining1, color2, remaining2);
}

static void format_mode(char *line) {
    const char *tuning_color;

    switch (status) {
    case AUTO_RED1_GREEN2:
        format_auto(line, 'R', 'G');
        return;
    case AUTO_RED1_AMBER2:
        format_auto(line, 'R', 'Y');
        return;
    case AUTO_GREEN1_RED2:
        format_auto(line, 'G', 'R');
        return;
    case AUTO_AMBER1_RED2:
        format_auto(line, 'Y', 'R');
        return;
    case MAN_GREEN1_RED2:
        strcpy(line, "MANUAL 1:G 2:R");
        return;
    case MAN_RED1_GREEN2:
        strcpy(line, "MANUAL 1:R 2:G");
        return;
    case TUNE_RED:
        tuning_color = "RED";
        break;
    case TUNE_GREEN:
        tuning_color = "GREEN";
        break;
    case TUNE_AMBER:
        tuning_color = "YELLOW";
        break;
    case SWITCH_ALL_RED:
        snprintf(line, LCD_COLS + 1U, "ALL RED %us",
                 seconds_remaining(timer_remaining_ms(TIMER_MANUAL)));
        return;
    case MAN_INIT:
        strcpy(line, "MANUAL ...");
        return;
    case TUNE_INIT:
        strcpy(line, "TUNE ...");
        return;
    default:
        strcpy(line, "AUTO ...");
        return;
    }

    snprintf(line, LCD_COLS + 1U, "TUNE %s %02us",
             tuning_color, seconds_remaining(tuning_value));
}

void lcd_screen_format(char lines[LCD_ROWS][LCD_COLS + 1U],
                       const DHT20_HandleTypeDef *dht) {
    format_mode(lines[0]);

    float temperature;
    float humidity;
    if (dht20_get_temperature(dht, &temperature) == HAL_OK &&
        dht20_get_humidity(dht, &humidity) == HAL_OK) {
        snprintf(lines[1], LCD_COLS + 1U, "T:%3dC H:%3d%%",
                 (int)temperature, (int)humidity);
    } else {
        strcpy(lines[1], "T: --C H: --%");
    }

    for (uint8_t row = 0; row < LCD_ROWS; ++row) {
        size_t length = strlen(lines[row]);
        memset(lines[row] + length, ' ', LCD_COLS - length);
        lines[row][LCD_COLS] = '\0';
    }
}
