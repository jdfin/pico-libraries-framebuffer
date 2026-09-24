#pragma once

#include <cstdint>

// Command opcodes for the Sitronix ST7789V / ST7789Vi controllers: ST7789V
// on the 4D Systems 4DLCD-24320240-IPS (older, ST7789V batch -- see
// Fd24::init()), ST7789Vi on the Newhaven NHD-2.4-240320-* family (see
// NhdBsxvF/NhdAfCtxp/NhdAfCsxp::init()).
//
// Per Sitronix's own datasheets (docs/ST7789V.pdf, docs/ST7789VI.pdf),
// ST7789Vi's command set is a strict superset of ST7789V's: every register
// below has the identical address and identical bit-level definition on
// both chips (confirmed by direct comparison, not just inference from the
// vendor sample code). The two vendor sample init sequences differ --
// different power/gamma tuning throughout, INVON used by 4D Systems' V
// sample but not Newhaven's Vi sample, etc. -- but that's a difference in
// which registers each *sample* happens to write, not in what the silicon
// implements, so one shared table is correct for both chips. (The lone
// register genuinely exclusive to ST7789Vi is GATESEL (0xd6), "Gate Output
// Selection in Sleep In Mode" -- omitted here since neither driver in this
// project writes it, per the "only opcodes actually used" rule below.)
//
// Despite sharing some opcode values with St7796Cmd (st7796_cmd.h), this
// table's vendor-specific registers have DIFFERENT meanings on that chip
// (e.g. 0xb7 is GCTRL here, but EM on ST7796) -- so it is not safe to reuse
// St7796Cmd for either of these drivers.
//
// Only the opcodes actually used by Fd24::init() and NhdBsxvF/NhdAfCtxp/
// NhdAfCsxp::init() (taken from 4D Systems' and Newhaven's own sample init
// code, respectively) are listed here. RAMCTRL/RGBCTRL names are this
// project's best-effort label based on common ST7789-family documentation
// -- not verbatim from the datasheet's own prose -- but the addresses and
// bit layouts are confirmed against it.

namespace St7789Cmd {

// Command Table 1 (generic MIPI DCS)
constexpr uint8_t SLPOUT = 0x11;  // Sleep Out
constexpr uint8_t INVON = 0x21;   // Display Inversion On
constexpr uint8_t DISPOFF = 0x28; // Display Off
constexpr uint8_t DISPON = 0x29;  // Display On
constexpr uint8_t CASET = 0x2a;   // Column Address Set
constexpr uint8_t RASET = 0x2b;   // Row Address Set
constexpr uint8_t RAMWR = 0x2c;   // Memory Write
constexpr uint8_t MADCTL = 0x36;  // Memory Data Access Control
constexpr uint8_t COLMOD = 0x3a;  // Interface Pixel Format

// Command Table 2 (vendor-specific, shared by ST7789V and ST7789Vi)
constexpr uint8_t RAMCTRL = 0xb0;  // RAM Control (best-effort name)
constexpr uint8_t RGBCTRL = 0xb1;  // RGB Interface Control (best-effort name)
constexpr uint8_t PORCTRK = 0xb2;  // Porch Setting
constexpr uint8_t GCTRL = 0xb7;    // Gate Control
constexpr uint8_t VCOMS = 0xbb;    // VCOM Setting
constexpr uint8_t LCMCTRL = 0xc0;  // LCM Control
constexpr uint8_t VDVVRHEN = 0xc2; // VDV and VRH Command Enable
constexpr uint8_t VRHS = 0xc3;     // VRH Set
constexpr uint8_t VDVS = 0xc4;     // VDV Set
constexpr uint8_t FRCTRL2 = 0xc6;  // Frame Rate Control in Normal Mode
constexpr uint8_t PWCTRL1 = 0xd0;  // Power Control 1
constexpr uint8_t PVGAMCTRL = 0xe0; // Positive Voltage Gamma Control
constexpr uint8_t NVGAMCTRL = 0xe1; // Negative Voltage Gamma Control

}; // namespace St7789Cmd
