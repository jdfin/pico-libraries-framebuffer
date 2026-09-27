#pragma once

#include <cstdint>
// pico
#include "hardware/pio.h"
// framebuffer
#include "framebuffer/tft_if.h"


// TftIf implementation over spi3.pio (3-wire, 9-bit serial: D/C sent
// in-band as a bit ahead of each byte, no separate D/C pin). See
// spi3.pio's own header comment for the protocol and how it was
// validated (single-byte synchronous commands, multi-byte synchronous
// parameters, 16-bit pixel bursts via DMA with autopull).
//
// SPI mode is fixed inside spi3.pio itself (runs mode 0, clock idles low -
// confirmed on real hardware even for NhdCfBsxv/NhdAfCxxx, which need mode
// 3 over 4-wire SPI; CS is managed explicitly per-transaction here to make
// up the difference, since spi3.pio doesn't drive it). There's no runtime
// mode selection the way TftSpi4If has one; a future 3-wire panel needing
// a different mode would need a new PIO program, not a constructor
// parameter here.
class TftSpi3If : public TftIf
{

public:

    // baud: requested; actual may be different - see freq(). cs_pin is
    // required (asserted) - spi3.pio doesn't drive CS itself, so this
    // class must, and does so per-transaction (see the .cpp).
    TftSpi3If(PIO pio, int sda_pin, int clk_pin, int cs_pin, float baud);

    virtual void init() override;

    virtual int freq() const override
    {
        return _freq;
    }

    virtual void write_cmd(uint8_t byte) override;
    virtual void write_data(uint8_t byte) override;
    virtual void write_data16(uint16_t value) override;
    virtual void write_data16_blocking(const uint16_t *buf, int count) override;

    virtual void begin_data16_burst() override;

    virtual volatile void *fifo_addr() const override;

    virtual uint dreq() const override
    {
        return pio_get_dreq(_pio, _sm, true);
    }

    virtual void wait_transport_idle() override;

private:

    PIO _pio;
    int _sda_pin, _clk_pin, _cs_pin;
    float _baud;
    uint _sm;
    int _freq;

    static constexpr bool cs_assert = false;
    static constexpr bool cs_deassert = true;

    static constexpr int dc_command = 0;
    static constexpr int dc_data = 1;
};
