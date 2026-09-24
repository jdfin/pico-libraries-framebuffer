#pragma once

#include <cstdint>
// pico
#include "hardware/spi.h"
#include "pico/stdlib.h"
// framebuffer
#include "framebuffer/tft.h"


// 4D Systems 4DLCD-24320240-IPS, 2.4" 240x320 SPI IPS panel, no
// touchscreen -- older manufacturing batch, ST7789V controller. (A newer
// batch of this same product uses ILI9341V instead and would need its own
// driver; see st7789_cmd.h.) Physical panel is portrait-native (240 cols
// x 320 rows) silicon, so like NhdBsxvF/NhdAfCtxp/NhdAfCsxp, width/height
// are fixed at compile time rather than taken as constructor arguments.
class Fd24 : public Tft
{

public:

    // baud normally 15'000'000
    Fd24(spi_inst_t *spi, int miso_pin, int mosi_pin, int clk_pin,
         int cs_pin, int baud, int cd_pin, int rst_pin, int bk_pin,
         void *work = nullptr, int work_bytes = 0) :
        Tft(spi, miso_pin, mosi_pin, clk_pin, cs_pin, baud, cd_pin, rst_pin,
            bk_pin, raw_cols, raw_rows, work, work_bytes)
    {
    }

    void init();

private:

    static constexpr int raw_cols = 240; // native column count
    static constexpr int raw_rows = 320; // native row count

    virtual uint8_t madctl() const;

    // This panel needs SPI mode 3 (CPOL=1, CPHA=1), confirmed against a
    // known-working Arduino driver for this same display (arduino/fd24/),
    // which hardcodes SPI_MODE3. Tft defaults to mode 0, which is what
    // Ws35/Hy35/Ws24/the Newhaven boards use.
    virtual spi_cpol_t spi_cpol() const
    {
        return SPI_CPOL_1;
    }
    virtual spi_cpha_t spi_cpha() const
    {
        return SPI_CPHA_1;
    }
};
