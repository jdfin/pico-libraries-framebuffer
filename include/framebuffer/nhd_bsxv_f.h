#pragma once

#include <cstdint>
// pico
#include "hardware/spi.h"
#include "pico/stdlib.h"
// framebuffer
#include "framebuffer/tft.h"


// Newhaven NHD-2.4-240320CF-BSXV#-F, ST7789Vi controller, 240x320, SPI
// interface, no touchscreen. Physical panel is portrait-native (240 cols x
// 320 rows) silicon, so unlike Ws35/Hy35/Ws24, width/height are fixed at
// compile time rather than taken as constructor arguments.
//
// Electrically and physically identical to NhdAfCtxp/NhdAfCsxp except for
// one MADCTL bit (RGB vs BGR color order) - see madctl().
class NhdBsxvF : public Tft
{

public:

    // baud normally 15'000'000
    NhdBsxvF(spi_inst_t *spi, int miso_pin, int mosi_pin, int clk_pin,
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
};
