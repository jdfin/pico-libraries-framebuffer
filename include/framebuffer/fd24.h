#pragma once

#include <cstdint>
// pico
#include "pico/stdlib.h"
// framebuffer
#include "framebuffer/tft.h"


// 4D Systems 4DLCD-24320240-IPS, 2.4" 240x320 SPI IPS panel, no
// touchscreen -- older manufacturing batch, ST7789V controller. (A newer
// batch of this same product uses ILI9341V instead and would need its own
// driver; see st7789_cmd.h.) Physical panel is portrait-native (240 cols
// x 320 rows) silicon, so like NhdBsxvF/NhdAfCtxp/NhdAfCsxp, width/height
// are fixed at compile time rather than taken as constructor arguments.
//
// Needs SPI mode 3 (CPOL=1, CPHA=1) - the caller is responsible for
// constructing its TftSpiIf with SPI_CPOL_1/SPI_CPHA_1; that's no longer
// this class's concern now that mode selection lives on TftSpiIf, not Tft.
class Fd24 : public Tft
{

public:

    Fd24(TftIf &io, int rst_pin, int bk_pin, //
         void *work = nullptr, int work_bytes = 0) :
        Tft(io, rst_pin, bk_pin, raw_cols, raw_rows, work, work_bytes)
    {
    }

    void init();

private:

    static constexpr int raw_cols = 240; // native column count
    static constexpr int raw_rows = 320; // native row count

    virtual uint8_t madctl() const;
};
