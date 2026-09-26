#pragma once

#include <cstdint>
// pico
#include "pico/stdlib.h"
// framebuffer
#include "framebuffer/tft.h"


// Newhaven NHD-2.4-240320CF-BSXV#-F, ST7789Vi controller, 240x320, SPI
// interface, no touchscreen. Physical panel is portrait-native (240 cols x
// 320 rows) silicon, so unlike Ws35/Hy35/Ws24, width/height are fixed at
// compile time rather than taken as constructor arguments.
//
// Confirmed against real hardware: needs SPI mode 3 (over 4-wire SPI - see
// NhdAfCxxx for the 3-wire-only Newhaven parts, which need no such mode
// selection), and madctl() (see tft.h) does not set the RGB/BGR bit --
// setting it swaps red and blue on this part.
//
// SPI mode selection now lives on TftSpiIf, not here - the caller is
// responsible for constructing its TftSpiIf with SPI_CPOL_1/SPI_CPHA_1.
class NhdCfBsxv : public Tft
{

public:

    NhdCfBsxv(TftIf &io, int rst_pin, int bk_pin, //
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
