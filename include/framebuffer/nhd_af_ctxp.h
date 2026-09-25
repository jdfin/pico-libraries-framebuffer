#pragma once

#include <cstdint>
// pico
#include "hardware/spi.h"
#include "pico/stdlib.h"
// framebuffer
#include "framebuffer/tft.h"


// Newhaven NHD-2.4-240320AF-CTXP, ST7789Vi controller, 240x320, SPI
// interface, no touchscreen. Physical panel is portrait-native (240 cols x
// 320 rows) silicon, so unlike Ws35/Hy35/Ws24, width/height are fixed at
// compile time rather than taken as constructor arguments.
//
// Electrically and physically similar to NhdBsxvF/NhdAfCsxp; assumed (but
// NOT yet confirmed against real hardware, unlike NhdBsxvF - see the
// comment there) to need only Newhaven's Cxxx-sample MADCTL values below,
// with no RGB/BGR bit and no SPI mode 3. If this part turns out to need
// the same fixes NhdBsxvF needed, update madctl()/spi_cpol()/spi_cpha()
// here to match.
class NhdAfCtxp : public Tft
{

public:

    // baud normally 15'000'000
    NhdAfCtxp(spi_inst_t *spi, int miso_pin, int mosi_pin, int clk_pin,
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
