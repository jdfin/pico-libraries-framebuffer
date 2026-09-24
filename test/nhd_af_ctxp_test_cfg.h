#pragma once

#include "hardware/spi.h"

// No touchscreen on these Newhaven NHD-2.4-240320 parts, so only the SPI
// display pins are needed. Edit these to match your wiring.

constexpr int fb_spi_miso_gpio = 16;
constexpr int fb_spi_mosi_gpio = 19;
constexpr int fb_spi_clk_gpio = 18;
constexpr int fb_spi_cs_gpio = 17;
spi_inst_t *const fb_spi_inst = spi0;

constexpr int fb_cd_gpio = 20;
constexpr int fb_rst_gpio = 21;
constexpr int fb_led_gpio = 22;
