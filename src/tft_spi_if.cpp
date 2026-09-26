
#include "framebuffer/tft_spi_if.h"

#include <cassert>
// pico
#include "hardware/gpio.h"
#include "pico/stdlib.h"


TftSpiIf::TftSpiIf(spi_inst_t *spi, int miso_pin, int mosi_pin, int clk_pin,
                    int cs_pin, int baud, int cd_pin, //
                    spi_cpol_t cpol, spi_cpha_t cpha) :
    _spi(spi),
    _freq(0),
    _miso_pin(miso_pin),
    _mosi_pin(mosi_pin),
    _clk_pin(clk_pin),
    _cs_pin(cs_pin),
    _cd_pin(cd_pin),
    _cpol(cpol),
    _cpha(cpha)
{
    assert(_spi != nullptr);
    assert(_miso_pin >= 0 && _mosi_pin >= 0 && _clk_pin >= 0);
    assert(_cd_pin >= 0);

    _freq = spi_init(_spi, baud);
    gpio_set_function(_miso_pin, GPIO_FUNC_SPI);
    gpio_set_function(_mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(_clk_pin, GPIO_FUNC_SPI);
    spi_set_format(_spi, 8, _cpol, _cpha, SPI_MSB_FIRST);

    if (_cs_pin >= 0) {
        gpio_init(_cs_pin);
        gpio_set_dir(_cs_pin, GPIO_OUT);
        gpio_put(_cs_pin, cs_assert);
    }

    gpio_init(_cd_pin);
    gpio_set_dir(_cd_pin, GPIO_OUT);
    // don't care if it's high or low at this point (but it's low)
}


void TftSpiIf::write_cmd(uint8_t byte)
{
    spi_set_format(_spi, 8, _cpol, _cpha, SPI_MSB_FIRST);
    command();
    spi_get_hw(_spi)->dr = byte;
    wait_transport_idle();
}


void TftSpiIf::write_data(uint8_t byte)
{
    spi_set_format(_spi, 8, _cpol, _cpha, SPI_MSB_FIRST);
    data();
    spi_get_hw(_spi)->dr = byte;
    wait_transport_idle();
}


void TftSpiIf::write_data16(uint16_t value)
{
    // Matches Tft's original spi_write_data(uint16_t): two 8-bit writes,
    // not a transient switch to 16-bit format - not worth it for a single
    // pixel (only bulk transfers, via begin_data16_burst(), switch modes).
    spi_set_format(_spi, 8, _cpol, _cpha, SPI_MSB_FIRST);
    data();
    spi_get_hw(_spi)->dr = (uint32_t)(value >> 8);
    spi_get_hw(_spi)->dr = (uint32_t)value;
    wait_transport_idle();
}


void TftSpiIf::write_data16_blocking(const uint16_t *buf, int count)
{
    data();
    spi_set_format(_spi, 16, _cpol, _cpha, SPI_MSB_FIRST);
    spi_write16_blocking(_spi, buf, count);
}


void TftSpiIf::begin_data16_burst()
{
    data();
    spi_set_format(_spi, 16, _cpol, _cpha, SPI_MSB_FIRST);
}
