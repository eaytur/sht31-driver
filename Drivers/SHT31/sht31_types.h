#ifndef SHT31_TYPES_H
#define SHT31_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef enum 
{
    SHT31_OK = 0U,
    SHT31_ERROR,
    SHT31_ERR_INVALID_PARAM,
    SHT31_ERR_NOT_INITIALIZED,
    SHT31_ERR_WRITE,
    SHT31_ERR_READ,
    SHT31_ERR_CRC,
    SHT31_ERR_TIMEOUT,
    SHT31_ERR_INVALID_STATE,
    SHT31_ERR_DEVICE_NOT_READY,
} sht31_status_t;

typedef enum
{
    SHT31_REPEATABILITY_LOW = 0U,
    SHT31_REPEATABILITY_MEDIUM,
    SHT31_REPEATABILITY_HIGH,
} sht31_repeatability_t;

typedef struct {
    int (*i2c_write)(uint8_t addr, uint8_t *data, uint16_t len);
    int (*i2c_read)(uint8_t addr, uint8_t *data, uint16_t len);
    int (*is_device_ready)(uint16_t device_addr, uint32_t trials, uint32_t timeout);
    void (*delay_ms)(uint32_t);
} sht31_io_t;

typedef struct {
    sht31_io_t io;
    bool initialized;
    uint8_t addr;
} sht31_ctx_t;


#endif/*SHT31_TYPES_H*/