# SHT31 Driver

Platform-independent C driver for the Sensirion SHT31 temperature and humidity sensor, with an STM32 HAL BSP layer.

## Project Structure

```
Drivers/SHT31/
  sht31.c             # Driver implementation
  sht31.h             # Public API
  sht31_types.h       # Types: status codes, IO struct, context
  sht31_config.h      # Measurement config and result structs

bsp/
  src/bsp_i2c.c       # STM32 HAL I2C bindings
  inc/bsp_i2c.h       # BSP interface

Core/
  Src/main.c          # Usage example
```

## Driver API

### Initialization

```c
sht31_io_t io = {
    .i2c_write       = bsp_i2c_write,
    .i2c_read        = bsp_i2c_read,
    .is_device_ready = bsp_i2c_is_device_ready,
    .delay_ms        = bsp_i2c_delay_ms,
};

sht31_ctx_t ctx;
sht31_status_t status = sht31_init(&ctx, &io, 0x44U);
```

`sht31_init` checks device presence on the bus before marking the context as initialized. Returns `SHT31_ERR_DEVICE_NOT_READY` if the sensor does not respond.

### Single-Shot Measurement

```c
sht31_singleshot_cfg_t cfg = {
    .repeatability    = SHT31_REPEATABILITY_HIGH,
    .clock_stretching = false,
};

sht31_measurement_t result;
status = sht31_single_shot(&ctx, &cfg, &result);

if (status == SHT31_OK) {
    float temp = result.temperature_c;
    float hum  = result.humidity_rh;
}
```

Repeatability options: `SHT31_REPEATABILITY_LOW`, `_MEDIUM`, `_HIGH`.
Measurement delay is handled internally by the driver.

### Other

```c
sht31_soft_reset(&ctx);   // Send soft reset command
sht31_deinit(&ctx);       // Clear context
```

## BSP Layer

The BSP provides the four I2C callbacks required by the driver IO struct.

```c
// Call once after HAL I2C initialization
bsp_i2c_set_handle(&hi2c1);
```

| Function | Description |
|---|---|
| `bsp_i2c_write` | HAL_I2C_Master_Transmit wrapper |
| `bsp_i2c_read` | HAL_I2C_Master_Receive wrapper |
| `bsp_i2c_is_device_ready` | HAL_I2C_IsDeviceReady wrapper |
| `bsp_i2c_delay_ms` | HAL_Delay wrapper |

All functions return `0` on success, `-1` on failure.

> **Note:** The driver uses 7-bit I2C addresses (`0x44` / `0x45`). The BSP is responsible for shifting the address left by 1 bit before passing it to the HAL (`addr << 1`), as required by `HAL_I2C_Master_Transmit` and related functions. This shift is already handled in `bsp_i2c.c`.

The driver is BSP-agnostic — the IO struct can be populated with any platform's I2C implementation.

## Hardware

| SHT31 Pin | STM32 |
|-----------|-------|
| SDA | PB7 (I2C1) |
| SCL | PB6 (I2C1) |
| ADDR | GND → address `0x44` |
| VDD | 3.3V |
| GND | GND |

## Build

Requires:
- [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html)
- [CMake](https://cmake.org/) >= 3.22
- `arm-none-eabi-gcc`

```bash
cmake --preset Debug
cmake --build build/Debug
```

## License

[MIT](LICENSE)
