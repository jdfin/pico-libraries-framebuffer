#pragma once

#include <cstdint>


// Abstract panel I/O for Tft: everything Tft needs to talk to a controller
// chip over some serial link, independent of which link that actually is.
// TftSpi4If implements this over the Pico SDK's hardware SPI (4-wire, a
// separate D/C GPIO); TftSpi3If implements it over spi3.pio (3-wire, D/C
// sent in-band as a 9th bit per byte). Tft itself owns the DMA channel, its
// interrupt handler, and the async fill/copy queue (_ops[]) - none of that
// lives here. A TftIf implementation only ever needs to answer "write this
// byte/word now" or "here's where and how to DMA a burst of them."
class TftIf
{

public:

    virtual ~TftIf() = default;

    // Actual achieved clock frequency. Requested vs. actual can differ -
    // both spi_init()'s baud divisor and a PIO clock divider have real
    // quantization.
    virtual int freq() const = 0;

    // Synchronous single-byte writes. D/C framing - however this transport
    // signals it (a GPIO for TftSpi4If, an in-band bit for TftSpi3If) - is
    // the implementation's problem, not the caller's.
    virtual void write_cmd(uint8_t byte) = 0;
    virtual void write_data(uint8_t byte) = 0;

    // Synchronous single 16-bit data value (D/C=data).
    virtual void write_data16(uint16_t value) = 0;

    // Synchronous (blocking, no DMA) bulk write of 'count' 16-bit data
    // values (D/C=data) from 'buf'. Used by print(), which re-fills and
    // re-flushes a small working buffer one chunk at a time and so must
    // block between chunks - see the comment on Tft::print().
    virtual void write_data16_blocking(const uint16_t *buf, int count) = 0;

    // Prepares for a burst of 16-bit data values (D/C=data) about to be
    // DMA'd via fifo_addr()/dreq() below. Must complete before the caller
    // starts that DMA transfer.
    virtual void begin_data16_burst() = 0;

    // Destination address and DREQ for DMA'ing 16-bit words, valid after
    // begin_data16_burst().
    virtual volatile void *fifo_addr() const = 0;
    virtual uint dreq() const = 0;

    // Waits for the transport to have genuinely finished everything
    // written/DMA'd so far - not just "DMA channel reports done": some
    // transports (spi3) still have to flush their own internal shift state
    // after the DMA channel itself is finished.
    virtual void wait_transport_idle() = 0;
};
