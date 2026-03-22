#ifndef BSP_I2C_H
#define BSP_I2C_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

void bsp_i2c_set_handle(I2C_HandleTypeDef *hi2c);

int  bsp_i2c_write(uint8_t addr, uint8_t *data, uint16_t len);
int  bsp_i2c_read(uint8_t addr, uint8_t *data, uint16_t len);
int  bsp_i2c_is_device_ready(uint16_t addr, uint32_t trials, uint32_t timeout);
void bsp_i2c_delay_ms(uint32_t ms);

#endif /* BSP_I2C_H */
