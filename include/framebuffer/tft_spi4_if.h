#pragma once

#include <cstdint>
// pico
#include "hardware/gpio.h"
#include "hardware/spi.h"
// framebuffer
#include "framebuffer/tft_if.h"


// TftIf implementation over the Pico SDK's hardware SPI (4-wire: separate
// MISO/MOSI/CLK/CS, plus a D/C GPIO). This is exactly what Tft did directly
// before TftIf existed - extracted verbatim, no behavior change.
class TftSpi4If : public TftIf
{

public:

    // baud: requested; actual may be different - see freq().
    // cpol/cpha: every panel driven over hardware SPI so far uses mode 0
    // (the defaults); Fd24 needs mode 3.
    TftSpi4If(spi_inst_t *spi, int miso_pin, int mosi_pin, int clk_pin,
              int cs_pin, int baud, int cd_pin, //
              spi_cpol_t cpol = SPI_CPOL_0, spi_cpha_t cpha = SPI_CPHA_0);

    virtual int freq() const override
    {
        return _freq;
    }

    virtual void write_cmd(uint8_t byte) override;
    virtual void write_data(uint8_t byte) override;
    virtual void write_data16(uint16_t value) override;
    virtual void write_data16_blocking(const uint16_t *buf, int count) override;

    virtual void begin_data16_burst() override;

    virtual volatile void *fifo_addr() const override
    {
        return &spi_get_hw(_spi)->dr;
    }

    virtual uint dreq() const override
    {
        return spi_get_dreq(_spi, true);
    }

    virtual void wait_transport_idle() override
    {
        while (spi_is_busy(_spi))
            tight_loop_contents();
    }

private:

    spi_inst_t *_spi;
    int _freq;

    int _miso_pin, _mosi_pin, _clk_pin, _cs_pin;
    int _cd_pin;

    spi_cpol_t _cpol;
    spi_cpha_t _cpha;

    static constexpr bool cs_assert = false;
    static constexpr bool cs_deassert = true;

    static constexpr bool cd_gpio_command = false;
    static constexpr bool cd_gpio_data = true;

    void data()
    {
        gpio_put(_cd_pin, cd_gpio_data);
    }

    void command()
    {
        gpio_put(_cd_pin, cd_gpio_command);
    }
};
