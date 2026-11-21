#include "dht20.h"

#define DHT20_STATUS_BUSY       0x80U
#define DHT20_STATUS_READY_MASK 0x18U

static uint8_t dht20_crc8(const uint8_t *data, uint8_t length);
static HAL_StatusTypeDef dht20_reset_register(DHT20_HandleTypeDef *dht, uint8_t reg);

HAL_StatusTypeDef dht20_is_connected(DHT20_HandleTypeDef *dht) {
    if (dht == NULL || dht->hi2c == NULL) {
        return HAL_ERROR;
    }

    return HAL_I2C_IsDeviceReady(dht->hi2c, DHT20_I2C_ADDR << 1, 2, 10);
}

HAL_StatusTypeDef dht20_read_status(DHT20_HandleTypeDef *dht, uint8_t *sensor_status) {
    if (dht == NULL || dht->hi2c == NULL || sensor_status == NULL) {
        return HAL_ERROR;
    }

    return HAL_I2C_Master_Receive(dht->hi2c, DHT20_I2C_ADDR << 1, sensor_status, 1, 10);
}

HAL_StatusTypeDef dht20_init(DHT20_HandleTypeDef *dht, I2C_HandleTypeDef *hi2c) {
    if (dht == NULL || hi2c == NULL) {
        return HAL_ERROR;
    }

    dht->hi2c = hi2c;
    for (uint8_t i = 0; i < DHT20_DATA_SIZE; ++i) {
        dht->data[i] = 0;
    }
    dht->temperature = 0.0f;
    dht->humidity = 0.0f;
    dht->last_req = 0;
    dht->last_read = 0;
    dht->pending = 0;
    dht->frame_ready = 0;
    dht->has_valid_data = 0;

    HAL_Delay(500);

    HAL_StatusTypeDef status = dht20_is_connected(dht);
    if (status != HAL_OK) {
        return status;
    }

    status = dht20_reset_sensor(dht);
    if (status != HAL_OK) {
        return status;
    }

    /* Allow the sensor to settle before the first measurement command. */
    HAL_Delay(10);
    return HAL_OK;
}

HAL_StatusTypeDef dht20_request_data(DHT20_HandleTypeDef *dht) {
    if (dht == NULL || dht->hi2c == NULL) {
        return HAL_ERROR;
    }
    if (dht->pending != 0) {
        return HAL_BUSY;
    }

    uint8_t command[3] = {0xACU, 0x33U, 0x00U};

    HAL_StatusTypeDef status;
    status = HAL_I2C_Master_Transmit(dht->hi2c, DHT20_I2C_ADDR << 1, command, 3, 10);
    if (status != HAL_OK) {
        return status;
    }

    dht->last_req = HAL_GetTick();
    dht->pending = 1;
    dht->frame_ready = 0;

    return HAL_OK;
}

HAL_StatusTypeDef dht20_read_data(DHT20_HandleTypeDef *dht) {
    if (dht == NULL || dht->hi2c == NULL ||
        dht->pending == 0) {
        return HAL_ERROR;
    }

    uint32_t elapsed = HAL_GetTick() - dht->last_req;
    if (elapsed < DHT20_MEASUREMENT_MS) {
        return HAL_BUSY;
    }
    if (elapsed >= DHT20_MEASUREMENT_TIMEOUT_MS) {
        dht->pending = 0;
        return HAL_TIMEOUT;
    }

    HAL_StatusTypeDef status;
    status = HAL_I2C_Master_Receive(dht->hi2c, DHT20_I2C_ADDR << 1, dht->data, DHT20_DATA_SIZE, 10);
    if (status == HAL_BUSY) {
        return HAL_BUSY;
    }
    if (status != HAL_OK) {
        dht->pending = 0;
        return status;
    }

    if ((dht->data[0] & DHT20_STATUS_BUSY) != 0U) {
        return HAL_BUSY;
    }

    dht->pending = 0;
    dht->frame_ready = 1;
    return HAL_OK;
}

static uint8_t dht20_crc8(const uint8_t *data, uint8_t length) {
    uint8_t crc = 0xFFU;

    for (uint8_t i = 0; i < length; ++i) {
        crc ^= data[i];
        /* Process all eight bits before moving to the next byte. */
        for (uint8_t j = 0; j < 8; ++j) {
            if ((crc & 0x80U) != 0U) {
                crc <<= 1;
                crc ^= 0x31U;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

HAL_StatusTypeDef dht20_convert(DHT20_HandleTypeDef *dht) {
    if (dht == NULL) {
        return HAL_ERROR;
    }
    if (dht->pending != 0U) {
        return HAL_BUSY;
    }
    if (dht->frame_ready == 0U) {
        return HAL_ERROR;
    }
    dht->frame_ready = 0;

    uint8_t crc = dht20_crc8(dht->data, DHT20_DATA_SIZE - 1U);
    if (crc != dht->data[DHT20_DATA_SIZE - 1U]) {
        return HAL_ERROR;
    }

    uint32_t raw_humid = ((uint32_t)dht->data[1] << 12) |
                         ((uint32_t)dht->data[2] << 4) |
                         (dht->data[3] >> 4);
    uint32_t raw_temp = ((uint32_t)(dht->data[3] & 0x0FU) << 16) |
                        ((uint32_t)dht->data[4] << 8) |
                        dht->data[5];

    dht->humidity = raw_humid * 100.0f / 1048576.0f;
    dht->temperature = raw_temp * 200.0f / 1048576.0f - 50.0f;
    dht->last_read = HAL_GetTick();
    dht->has_valid_data = 1;

    return HAL_OK;
}

HAL_StatusTypeDef dht20_get_temperature(const DHT20_HandleTypeDef *dht, float *temperature) {
    if (dht == NULL || temperature == NULL ||
        dht->has_valid_data == 0) {
        return HAL_ERROR;
    }

    *temperature = dht->temperature;

    return HAL_OK;
}

HAL_StatusTypeDef dht20_get_humidity(const DHT20_HandleTypeDef *dht, float *humidity) {
    if (dht == NULL || humidity == NULL ||
        dht->has_valid_data == 0) {
        return HAL_ERROR;
    }

    *humidity = dht->humidity;

    return HAL_OK;
}

static HAL_StatusTypeDef dht20_reset_register(DHT20_HandleTypeDef *dht, uint8_t reg) {
    if (dht == NULL || dht->hi2c == NULL ||
        (reg != 0x1B && reg != 0x1C && reg != 0x1E)) {
        return HAL_ERROR;
    }

    uint8_t command[3] = {reg, 0x00U, 0x00U};
    uint8_t value[3];
    HAL_StatusTypeDef status;

    status = HAL_I2C_Master_Transmit(dht->hi2c, DHT20_I2C_ADDR << 1, command, 3, 10);
    if (status != HAL_OK) {
        return status;
    }

    HAL_Delay(5);

    status = HAL_I2C_Master_Receive(dht->hi2c, DHT20_I2C_ADDR << 1, value, 3, 10);
    if (status != HAL_OK) {
        return status;
    }

    HAL_Delay(10);

    command[0] = 0xB0U | reg;
    command[1] = value[1];
    command[2] = value[2];

    status = HAL_I2C_Master_Transmit(dht->hi2c, DHT20_I2C_ADDR << 1, command, 3, 10);
    if (status != HAL_OK) {
        return status;
    }

    HAL_Delay(5);
    return HAL_OK;
}

HAL_StatusTypeDef dht20_reset_sensor(DHT20_HandleTypeDef *dht) {
    if (dht == NULL || dht->hi2c == NULL) {
        return HAL_ERROR;
    }

    if (dht->pending != 0) {
        return HAL_BUSY;
    }

    HAL_StatusTypeDef status;
    uint8_t sensor_status;

    status = dht20_read_status(dht, &sensor_status);
    if (status != HAL_OK) {
        return status;
    }

    if ((sensor_status & DHT20_STATUS_BUSY) != 0U) {
        return HAL_BUSY;
    }

    if ((sensor_status & DHT20_STATUS_READY_MASK) == DHT20_STATUS_READY_MASK) {
        return HAL_OK;
    }

    static const uint8_t registers[3] = {
        0x1BU, 0x1CU, 0x1EU
    };

    dht->frame_ready = 0;
    for (uint8_t i = 0; i < 3; ++i) {
        status = dht20_reset_register(dht, registers[i]);
        if (status != HAL_OK) {
            return status;
        }
    }

    HAL_Delay(10);

    status = dht20_read_status(dht, &sensor_status);
    if (status != HAL_OK) {
        return status;
    }

    if ((sensor_status & DHT20_STATUS_BUSY) != 0U) {
        return HAL_BUSY;
    }

    if ((sensor_status & DHT20_STATUS_READY_MASK) != DHT20_STATUS_READY_MASK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}
