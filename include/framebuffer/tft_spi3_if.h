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
// SPI mode is fixed to mode 3 (CPOL=1, CPHA=1) inside spi3.pio itself --
// confirmed needed by NhdCfBsxv, same Newhaven/ST7789Vi family as the
// af_csxp/af_ctxp boards this transport exists for. There's no runtime
// mode selection the way TftSpiIf has one; a future 3-wire panel needing
// a different mode would need a new PIO program, not a constructor
// parameter here.
class TftSpi3If : public TftIf
{

public:

    // baud: requested; actual may be different - see freq(). cs_pin may
    // be -1 if the panel's CS is hardwired rather than GPIO-controlled.
    TftSpi3If(PIO pio, int sda_pin, int clk_pin, int cs_pin, float baud);

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
    uint _sm;
    int _cs_pin;
    int _freq;

    static constexpr bool cs_assert = false;
    static constexpr bool cs_deassert = true;

    static constexpr int dc_command = 0;
    static constexpr int dc_data = 1;
};
