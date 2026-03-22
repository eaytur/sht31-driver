#include "bsp_i2c.h"

#define BSP_I2C_TIMEOUT_MS 100U

static I2C_HandleTypeDef *s_hi2c = NULL;

void bsp_i2c_set_handle(I2C_HandleTypeDef *hi2c)
{
    s_hi2c = hi2c;
}

int bsp_i2c_write(uint8_t addr, uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status;
    status = HAL_I2C_Master_Transmit(s_hi2c, (uint16_t)(addr << 1U), data, len, BSP_I2C_TIMEOUT_MS);
    return (status == HAL_OK) ? 0 : -1;
}

int bsp_i2c_read(uint8_t addr, uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status;
    status = HAL_I2C_Master_Receive(s_hi2c, (uint16_t)(addr << 1U), data, len, BSP_I2C_TIMEOUT_MS);
    return (status == HAL_OK) ? 0 : -1;
}

int bsp_i2c_is_device_ready(uint16_t addr, uint32_t trials, uint32_t timeout)
{
    HAL_StatusTypeDef status;
    status = HAL_I2C_IsDeviceReady(s_hi2c, (uint16_t)(addr << 1U), trials, timeout);
    return (status == HAL_OK) ? 0 : -1;
}

void bsp_i2c_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}
