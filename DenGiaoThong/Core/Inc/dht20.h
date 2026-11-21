#ifndef INC_DHT20_H
#define INC_DHT20_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define DHT20_I2C_ADDR                  0x38U
#define DHT20_DATA_SIZE                 7U
#define DHT20_MEASUREMENT_MS            80U
#define DHT20_MEASUREMENT_TIMEOUT_MS    250U

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t data[DHT20_DATA_SIZE];
    float temperature;
    float humidity;
    uint32_t last_req;
    uint32_t last_read;
    uint8_t pending;
    uint8_t frame_ready;
    uint8_t has_valid_data;
} DHT20_HandleTypeDef;

HAL_StatusTypeDef dht20_is_connected(DHT20_HandleTypeDef *dht);
HAL_StatusTypeDef dht20_read_status(DHT20_HandleTypeDef *dht, uint8_t *sensor_status);
/* Call after the I2C bus is initialized. Startup and recovery can block. */
HAL_StatusTypeDef dht20_init(DHT20_HandleTypeDef *dht, I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef dht20_request_data(DHT20_HandleTypeDef *dht);
/* Poll until HAL_OK or a terminal error. HAL_BUSY means to try again later. */
HAL_StatusTypeDef dht20_read_data(DHT20_HandleTypeDef *dht);
/* Consume one freshly read frame. A bad CRC leaves the last valid sample intact. */
HAL_StatusTypeDef dht20_convert(DHT20_HandleTypeDef *dht);
/* Get the last valid sample in degrees Celsius and percent relative humidity. */
HAL_StatusTypeDef dht20_get_temperature(const DHT20_HandleTypeDef *dht, float *temperature);
HAL_StatusTypeDef dht20_get_humidity(const DHT20_HandleTypeDef *dht, float *humidity);
HAL_StatusTypeDef dht20_reset_sensor(DHT20_HandleTypeDef *dht);
#endif
