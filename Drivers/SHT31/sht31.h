#ifndef SHT31_H
#define SHT31_H

#include "sht31_types.h"
#include "sht31_config.h"

sht31_status_t sht31_init(sht31_ctx_t *ctx, const sht31_io_t *io, uint8_t addr);
sht31_status_t sht31_deinit(sht31_ctx_t *ctx);
sht31_status_t sht31_soft_reset(sht31_ctx_t *ctx);
sht31_status_t sht31_single_shot(sht31_ctx_t *ctx, const sht31_singleshot_cfg_t *cfg, sht31_measurement_t *out);

#endif/*SHT31_H*/