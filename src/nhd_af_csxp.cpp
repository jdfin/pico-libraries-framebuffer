
#include <cassert>
#include <cstdint>
// pico
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
// framebuffer
#include "framebuffer/st7789_cmd.h"
#include "framebuffer/tft.h"
//
#include "framebuffer/nhd_af_csxp.h"

using namespace St7789Cmd;


// Init sequence transcribed from Newhaven's own sample code for these parts
// (see newhaven/NHD-2.4-240320CF-Cxxx.ino, function setup()). comm_out()/
// data_out() in the sample are just its bit-banged wire protocol - the
// register writes and values themselves don't depend on that, so the same
// sequence applies here over SPI.
void NhdAfCsxp::init()
{
    hw_reset(2000);

    // Newhaven's sample sleeps 250ms both before and after toggling reset;
    // Ws35/Hy35 (same family of controller) get away with 10ms, so we use
    // that here too rather than the more conservative Arduino delay.
    sleep_ms(10);

    const uint16_t cmds[] = {
        // clang-format off
        wr_cmd | DISPOFF,
        wr_cmd | SLPOUT,
        wr_delay_ms | 100,

        wr_cmd | MADCTL, madctl(),

        wr_cmd | COLMOD, 0x55, // 16 bits/pixel, 65k colors

        wr_cmd | PORCTRK, 0x0c, 0x0c, 0x00, 0x33, 0x33,
        wr_cmd | GCTRL, 0x35,
        wr_cmd | VCOMS, 0x2b,
        wr_cmd | LCMCTRL, 0x2c,
        wr_cmd | VDVVRHEN, 0x01, 0xff,
        wr_cmd | VRHS, 0x11,
        wr_cmd | VDVS, 0x20,
        wr_cmd | FRCTRL2, 0x0f,
        wr_cmd | PWCTRL1, 0xa4, 0xa1,

        wr_cmd | PVGAMCTRL, 0xd0, 0x00, 0x05, 0x0e, 0x15, 0x0d,
                      0x37, 0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19,
        wr_cmd | NVGAMCTRL, 0xd0, 0x00, 0x05, 0x0d, 0x0c, 0x06,
                      0x2d, 0x44, 0x40, 0x0e, 0x1c, 0x18, 0x16, 0x19,

        // Address the full raw panel once, as Newhaven's sample does. Every
        // draw call below sets its own window via set_window(), so this is
        // mostly boilerplate carried over from the sample.
        wr_cmd | CASET, 0x00, 0x00,
                      uint8_t((raw_cols - 1) >> 8),
                      uint8_t(raw_cols - 1),
        wr_cmd | RASET, 0x00, 0x00,
                      uint8_t((raw_rows - 1) >> 8),
                      uint8_t(raw_rows - 1),
        wr_delay_ms | 10,

        wr_cmd | DISPON,
        wr_delay_ms | 10,
        // clang-format on
    };
    const int cmds_len = sizeof(cmds) / sizeof(cmds[0]);

    write_cmds(cmds, cmds_len); // sets to 8-bit spi
}


// MADCTL: top three bits control orientation and y/row direction
//   80 MY  row address order
//   40 MX  column address order
//   20 MV  row/column exchange
//   10 ML  vertical refresh order (always 0)
//   08 RGB RGB-BGR order (this part: BGR, so the bit is clear)
//   04 MH  horizontal refresh order (always 0)
//
// Only the Rotation::portrait value here (MY set, RGB bit clear) is
// confirmed: it's exactly what Newhaven's own NHD-2.4-240320CF-Cxxx.ino
// sample uses (0x80), and it matches this panel's native raw 240 (cols) x
// 320 (rows) addressing with no row/column exchange.
//
// The other three rotations are derived using the standard ST7789 MADCTL
// rotation combinations (the same ones e.g. Adafruit's ST7789 driver uses
// for portrait-native glass). AF-CSXP is assumed to follow the Cxxx (BGR)
// sample's convention; none of these three have been verified against real
// hardware - if a non-default rotation comes out mirrored or flipped,
// adjust the MY/MX/MV bits below.
uint8_t NhdAfCsxp::madctl() const
{
    if (get_rotation() == Rotation::portrait) {
        return 0x80; // MY -- confirmed by Newhaven's Cxxx sample init code
    } else if (get_rotation() == Rotation::landscape) {
        return 0x60; // MX | MV -- derived, unverified
    } else if (get_rotation() == Rotation::portrait2) {
        return 0xc0; // MY | MX -- derived, unverified
    } else {
        assert(get_rotation() == Rotation::landscape2);
        return 0xa0; // MY | MV -- derived, unverified
    }
}
