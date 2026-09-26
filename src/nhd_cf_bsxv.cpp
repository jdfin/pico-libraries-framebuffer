
#include <cassert>
#include <cstdint>
// pico
#include "hardware/gpio.h"
#include "pico/stdlib.h"
// framebuffer
#include "framebuffer/st7789_cmd.h"
#include "framebuffer/tft.h"
//
#include "framebuffer/nhd_cf_bsxv.h"

using namespace St7789Cmd;


// Init sequence transcribed from Newhaven's own sample code for these parts
// (see newhaven/NHD-2.4-240320CF-BSXV.ino, function setup()). comm_out()/
// data_out() in the sample are just its bit-banged wire protocol - the
// register writes and values themselves don't depend on that, so the same
// sequence applies here over SPI.
void NhdCfBsxv::init()
{
    // at least 10 usec required
    hw_reset(100);

    // 120 msec required if it happens to be in sleep_out mode
    sleep_ms(120);

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
        wr_cmd | PVGAMCTRL, 0xd0, 0x00, 0x05, 0x0e, 0x15, 0x0d, 0x37,
                            0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19,
        wr_cmd | NVGAMCTRL, 0xd0, 0x00, 0x05, 0x0d, 0x0c, 0x06, 0x2d,
                            0x44, 0x40, 0x0e, 0x1c, 0x18, 0x16, 0x19,
        wr_delay_ms | 10,
        wr_cmd | DISPON,
        wr_delay_ms | 10,
        // clang-format on
    };
    const int cmds_len = sizeof(cmds) / sizeof(cmds[0]);

    write_cmds(cmds, cmds_len);
}


// MADCTL: see tft.h, madctl()
uint8_t NhdCfBsxv::madctl() const
{
    constexpr uint8_t rgb = 0x00;
    if (get_rotation() == Rotation::portrait) {
        return 0x00 | rgb;
    } else if (get_rotation() == Rotation::landscape) {
        return 0x60 | rgb;
    } else if (get_rotation() == Rotation::portrait2) {
        return 0xc0 | rgb;
    } else {
        assert(get_rotation() == Rotation::landscape2);
        return 0xa0 | rgb;
    }
}
