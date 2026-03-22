#ifndef SHT31_CONFIG_H
#define SHT31_CONFIG_H

#include "sht31_types.h"

#include <stdbool.h>

typedef struct {
    sht31_repeatability_t repeatability;
    bool clock_stretching;
} sht31_singleshot_cfg_t;

typedef struct {
    float temperature_c;
    float humidity_rh;
} sht31_measurement_t;

#endif/*SHT31_CONFIG_H*/