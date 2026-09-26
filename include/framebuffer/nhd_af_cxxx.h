#pragma once

#include <cstdint>
// pico
#include "pico/stdlib.h"
// framebuffer
#include "framebuffer/tft.h"


// Newhaven NHD-2.4-240320AF-CTXP and NHD-2.4-240320AF-CSXP, ST7789Vi
// controller, 240x320, SPI interface, no touchscreen. Physical panel is
// portrait-native (240 cols x 320 rows) silicon, so unlike Ws35/Hy35/Ws24,
// width/height are fixed at compile time rather than taken as constructor
// arguments.
//
// Confirmed against real hardware (both parts run this same class/init
// sequence - electrically and physically identical here, unlike
// NhdCfBsxv's own confirmed differences): only 3-wire SPI is supported (see
// TftSpi3If), and madctl() (below) does not set the RGB/BGR bit.
class NhdAfCxxx : public Tft
{

public:

    NhdAfCxxx(TftIf &io, int rst_pin, int bk_pin, //
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
