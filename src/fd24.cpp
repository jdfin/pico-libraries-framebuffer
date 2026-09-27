
#include <cassert>
#include <cstdint>
// pico
#include "hardware/gpio.h"
#include "pico/stdlib.h"
// framebuffer
#include "framebuffer/st7789_cmd.h"
#include "framebuffer/tft.h"
//
#include "framebuffer/fd24.h"

using namespace St7789Cmd;


// Init sequence transcribed from the sample init code on 4D Systems' own
// datasheet page for this display (resources.4dsystems.com.au, datasheet
// for 4DLCD-24320240-IPS, "ST7789V Initialization" listing). Reproduced
// faithfully, including the vendor's own apparent redundancy (VCOMS/
// LCMCTRL/VDVVRHEN/VRHS/VDVS/FRCTRL2/PWCTRL1 each get written once in the
// initial block, then some of them again with different final values in
// the "ST7789V Power setting" block) rather than collapsing it to just the
// final values -- this display is untested by this project beyond this
// transcription, so faithful reproduction of a sequence known to work is
// safer than an edit that can't be verified.
void Fd24::init()
{
    Tft::init();

    // at least 10 usec required
    hw_reset(100);

    // 120 msec required if it happens to be in sleep_out mode
    sleep_ms(120);

    const uint16_t cmds[] = {
        // clang-format off
        //------Start Initial Sequence---//
        wr_cmd | RAMCTRL, 0x00, 0xc4,
        wr_cmd | RGBCTRL, 0xc0,
        wr_cmd | COLMOD, 0x55,
        wr_cmd | MADCTL, madctl(),
        wr_cmd | PORCTRK, 0x0c, 0x0c, 0x00, 0x33, 0x33,
        wr_cmd | GCTRL, 0x75,
        wr_cmd | VCOMS, 0x20,
        wr_cmd | LCMCTRL, 0x2c,
        wr_cmd | VDVVRHEN, 0x01,
        wr_cmd | VRHS, 0x19,
        wr_cmd | VDVS, 0x20,
        wr_cmd | FRCTRL2, 0x0f,
        wr_cmd | PWCTRL1, 0xa7, 0xa1,

        //---ST7789V Power setting---//
        wr_cmd | VCOMS, 0x35,
        wr_cmd | LCMCTRL, 0x2c,
        wr_cmd | VDVVRHEN, 0x01,
        wr_cmd | VRHS, 0x11,
        wr_cmd | VDVS, 0x20,
        wr_cmd | FRCTRL2, 0x0f,
        wr_cmd | PWCTRL1, 0xa4, 0xa1,

        //---ST7789V Gamma setting---//
        wr_cmd | PVGAMCTRL, 0xd0, 0x0b, 0x11, 0x0b, 0x0a, 0x26,
                      0x36, 0x44, 0x4b, 0x38, 0x14, 0x14, 0x2a, 0x30,
        wr_cmd | NVGAMCTRL, 0xd0, 0x0b, 0x11, 0x0b, 0x0a, 0x26,
                      0x35, 0x43, 0x4a, 0x38, 0x14, 0x14, 0x2a, 0x30,

        wr_cmd | SLPOUT,
        wr_cmd | INVON,
        wr_delay_ms | 120,

        wr_cmd | DISPON,
        wr_delay_ms | 120,
        // clang-format on
    };
    const int cmds_len = sizeof(cmds) / sizeof(cmds[0]);

    write_cmds(cmds, cmds_len);
}


// MADCTL: see tft.h, madctl()
uint8_t Fd24::madctl() const
{
    // RGB bit is always 0
    if (get_rotation() == Rotation::portrait) {
        return 0x00;
    } else if (get_rotation() == Rotation::landscape) {
        return 0x60;
    } else if (get_rotation() == Rotation::portrait2) {
        return 0xc0;
    } else {
        assert(get_rotation() == Rotation::landscape2);
        return 0xa0;
    }
}
