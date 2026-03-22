#include "sht31.h"
#include "sht31_types.h"
#include "sht31_config.h"

#include <string.h>

#define SHT31_CMD_LEN 2U
#define SHT31_CMD_SOFT_RESET  0x30A2


#define SHT31_CMD_SINGLE_SHOT_HIGH_CS_EN     0x2C06U
#define SHT31_CMD_SINGLE_SHOT_MEDIUM_CS_EN   0x2C0DU
#define SHT31_CMD_SINGLE_SHOT_LOW_CS_EN      0x2C10U

#define SHT31_CMD_SINGLE_SHOT_HIGH_CS_DIS    0x2400U
#define SHT31_CMD_SINGLE_SHOT_MEDIUM_CS_DIS  0x240BU
#define SHT31_CMD_SINGLE_SHOT_LOW_CS_DIS     0x2416U

#define SHT31_MEAS_DURATION_LOW_MS           3U
#define SHT31_MEAS_DURATION_MEDIUM_MS        5U
#define SHT31_MEAS_DURATION_HIGH_MS          13U

#define SHT31_MEASUREMENT_FRAME_LEN 6U

#define SHT31_CRC8_POLYNOMIAL  0x31U
#define SHT31_CRC8_INIT        0xFFU
#define SHT31_DATA_WORD_LEN 2U

sht31_status_t sht31_init(sht31_ctx_t *ctx, const sht31_io_t *io, uint8_t addr)
{
    if (ctx == NULL || io == NULL) {
        return SHT31_ERR_INVALID_PARAM;
    }

    if (io->i2c_write == NULL ||
        io->i2c_read == NULL ||
        io->delay_ms == NULL ||
        io->is_device_ready == NULL) {
        return SHT31_ERR_INVALID_PARAM;
    }

    if (addr != 0x44U && addr != 0x45U) {
        return SHT31_ERR_INVALID_PARAM;
    }

    memset(ctx, 0, sizeof(*ctx));

    ctx->io = *io;
    ctx->addr = addr;

    if (io->is_device_ready((uint16_t)addr, 3U, 100U) != 0) {
        return SHT31_ERR_DEVICE_NOT_READY;
    }

    ctx->initialized = true;

    return SHT31_OK;
}

sht31_status_t sht31_deinit(sht31_ctx_t *ctx)
{
    if (ctx == NULL) {
        return SHT31_ERR_INVALID_PARAM;
    }

    if (!ctx->initialized) {
        return SHT31_ERR_NOT_INITIALIZED;
    }

    memset(ctx, 0, sizeof(*ctx));

    return SHT31_OK;
}

static sht31_status_t sht31_write_command(sht31_ctx_t *ctx, uint16_t cmd)
{
    uint8_t tx_buf[2];
    int ret_val;

    if (ctx == NULL) {
        return SHT31_ERR_INVALID_PARAM;
    }

    if (!ctx->initialized) {
        return SHT31_ERR_NOT_INITIALIZED;
    }

    if (ctx->io.i2c_write == NULL) {
        return SHT31_ERR_INVALID_PARAM;
    }

    tx_buf[0] = (uint8_t)((cmd >> 8) & 0xFFU); /* MSB */
    tx_buf[1] = (uint8_t)(cmd & 0xFFU);        /* LSB */

    ret_val = ctx->io.i2c_write(ctx->addr, tx_buf, SHT31_CMD_LEN);
    if (ret_val != 0) {
        return SHT31_ERR_WRITE;
    }

    return SHT31_OK;
}

sht31_status_t sht31_soft_reset(sht31_ctx_t *ctx)
{
    return sht31_write_command(ctx, SHT31_CMD_SOFT_RESET);
}


static sht31_status_t sht31_select_single_shot_command(const sht31_singleshot_cfg_t *cfg,
                                                       uint16_t *cmd,
                                                       uint32_t *delay_ms)
{
    if (cfg == NULL || cmd == NULL || delay_ms == NULL) {
        return SHT31_ERR_INVALID_PARAM;
    }

    switch (cfg->repeatability) {
        case SHT31_REPEATABILITY_LOW:
            *delay_ms = SHT31_MEAS_DURATION_LOW_MS;
            *cmd = (cfg->clock_stretching == true) ?
                   SHT31_CMD_SINGLE_SHOT_LOW_CS_EN :
                   SHT31_CMD_SINGLE_SHOT_LOW_CS_DIS;
            break;

        case SHT31_REPEATABILITY_MEDIUM:
            *delay_ms = SHT31_MEAS_DURATION_MEDIUM_MS;
            *cmd = (cfg->clock_stretching == true) ?
                   SHT31_CMD_SINGLE_SHOT_MEDIUM_CS_EN :
                   SHT31_CMD_SINGLE_SHOT_MEDIUM_CS_DIS;
            break;

        case SHT31_REPEATABILITY_HIGH:
            *delay_ms = SHT31_MEAS_DURATION_HIGH_MS;
            *cmd = (cfg->clock_stretching == true) ?
                   SHT31_CMD_SINGLE_SHOT_HIGH_CS_EN :
                   SHT31_CMD_SINGLE_SHOT_HIGH_CS_DIS;
            break;

        default:
            return SHT31_ERR_INVALID_PARAM;
    }

    return SHT31_OK;
}

static sht31_status_t sht31_read_data(sht31_ctx_t *ctx, uint8_t *rx_buf, uint16_t len)
{
    int ret_val;

    if (ctx == NULL || rx_buf == NULL || len == 0U) {
        return SHT31_ERR_INVALID_PARAM;
    }

    if (!ctx->initialized) {
        return SHT31_ERR_NOT_INITIALIZED;
    }

    if (ctx->io.i2c_read == NULL) {
        return SHT31_ERR_INVALID_PARAM;
    }

    ret_val = ctx->io.i2c_read(ctx->addr, rx_buf, len);
    if (ret_val != 0) {
        return SHT31_ERR_READ;
    }

    return SHT31_OK;
}


static uint8_t sht31_calculate_crc(const uint8_t *data, uint16_t len)
{
    uint8_t crc = SHT31_CRC8_INIT;
    uint16_t i;
    uint8_t bit;

    for (i = 0; i < len; i++) {
        crc ^= data[i];

        for (bit = 0; bit < 8U; bit++) {
            if ((crc & 0x80U) != 0U) {
                crc = (uint8_t)((crc << 1U) ^ SHT31_CRC8_POLYNOMIAL);
            } else {
                crc <<= 1U;
            }
        }
    }

    return crc;
}

static bool sht31_check_crc(const uint8_t *data, uint16_t len, uint8_t expected_crc)
{
    if (data == NULL) {
        return false;
    }

    return (sht31_calculate_crc(data, len) == expected_crc);
}

static float sht31_raw_to_temperature_c(uint16_t raw_temp)
{
    return -45.0f + (175.0f * (float)raw_temp / 65535.0f);
}

static float sht31_raw_to_humidity_rh(uint16_t raw_humidity)
{
    return 100.0f * (float)raw_humidity / 65535.0f;
}

sht31_status_t sht31_single_shot(sht31_ctx_t *ctx,
                                 const sht31_singleshot_cfg_t *cfg,
                                 sht31_measurement_t *out)
{
    sht31_status_t status;
    uint16_t cmd;
    uint32_t delay_ms;
    uint8_t rx_buf[SHT31_MEASUREMENT_FRAME_LEN];
    uint16_t raw_temp;
    uint16_t raw_humidity;

    if (ctx == NULL || cfg == NULL || out == NULL) {
        return SHT31_ERR_INVALID_PARAM;
    }

    if (!ctx->initialized) {
        return SHT31_ERR_NOT_INITIALIZED;
    }

    if (ctx->io.delay_ms == NULL) {
        return SHT31_ERR_INVALID_PARAM;
    }

    status = sht31_select_single_shot_command(cfg, &cmd, &delay_ms);
    if (status != SHT31_OK) {
        return status;
    }

    status = sht31_write_command(ctx, cmd);
    if (status != SHT31_OK) {
        return status;
    }

    ctx->io.delay_ms(delay_ms);

    status = sht31_read_data(ctx, rx_buf, SHT31_MEASUREMENT_FRAME_LEN);
    if (status != SHT31_OK) {
        return status;
    }

    if (!sht31_check_crc(&rx_buf[0], SHT31_DATA_WORD_LEN, rx_buf[2])) {
        return SHT31_ERR_CRC;
    }

    if (!sht31_check_crc(&rx_buf[3], SHT31_DATA_WORD_LEN, rx_buf[5])) {
        return SHT31_ERR_CRC;
    }

    raw_temp = ((uint16_t)rx_buf[0] << 8U) | (uint16_t)rx_buf[1];
    raw_humidity = ((uint16_t)rx_buf[3] << 8U) | (uint16_t)rx_buf[4];

    out->temperature_c = sht31_raw_to_temperature_c(raw_temp);
    out->humidity_rh = sht31_raw_to_humidity_rh(raw_humidity);

    return SHT31_OK;
}
