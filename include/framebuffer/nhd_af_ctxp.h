#pragma once

#include <cstdint>
// pico
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
// the same fixes NhdBsxvF needed, update madctl() here and construct this
// board's TftSpiIf with SPI_CPOL_1/SPI_CPHA_1 (mode selection now lives on
// TftSpiIf, not here).
class NhdAfCtxp : public Tft
{

public:

    NhdAfCtxp(TftIf &io, int rst_pin, int bk_pin, //
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
