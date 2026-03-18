# SHT31 Driver

STM32F407 I2C driver for the Sensirion SHT31 temperature and humidity sensor.

## Hardware

| SHT31 Pin | STM32 |
|-----------|-------|
| SDA | PB7 (I2C1) |
| SCL | PB6 (I2C1) |
| ADDR | GND → address `0x44` |
| VDD | 3.3V |
| GND | GND |

## Project Structure

```
Core/
  Src/main.c          # Application entry point
Drivers/
  STM32F4xx_HAL_Driver/   # STM32 HAL (CubeMX generated)
  CMSIS/                  # CMSIS headers
```

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
