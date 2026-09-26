
#include "framebuffer/tft_spi3_if.h"

#include "spi3.pio.h"
// pico
#include "hardware/clocks.h"
#include "hardware/gpio.h"


TftSpi3If::TftSpi3If(PIO pio, int sda_pin, int clk_pin, int cs_pin, float baud) :
    _pio(pio),
    _cs_pin(cs_pin)
{
    assert(sda_pin >= 0 && clk_pin >= 0 && cs_pin >= 0);

    _sm = spi3_init(_pio, sda_pin, clk_pin, baud);

    // spi3_init() doesn't report back the actual achieved clock (PIO clock
    // dividers are 16.8 fixed-point, so requested vs actual can differ, the
    // same as spi_init()'s return value for hardware SPI) -- read the
    // divider back directly from the SM's own hardware register instead of
    // duplicating spi3_init()'s internal float-to-fixed-point rounding.
    uint32_t clkdiv_reg = _pio->sm[_sm].clkdiv;
    uint32_t div_int =
        (clkdiv_reg & PIO_SM0_CLKDIV_INT_BITS) >> PIO_SM0_CLKDIV_INT_LSB;
    uint32_t div_frac8 =
        (clkdiv_reg & PIO_SM0_CLKDIV_FRAC_BITS) >> PIO_SM0_CLKDIV_FRAC_LSB;
    float divisor = (float)div_int + (float)div_frac8 / 256.0f;
    float sm_hz = (float)clock_get_hz(clk_sys) / divisor;
    // DCX toggles once per SM clock (one side-set change per instruction),
    // so its frequency is half the SM clock - see spi3.pio.
    _freq = (int)(sm_hz / 2.0f);

    gpio_init(_cs_pin);
    gpio_set_dir(_cs_pin, GPIO_OUT);
    gpio_put(_cs_pin, cs_deassert);
}


void TftSpi3If::write_cmd(uint8_t byte)
{
    spi3_set_bits(_pio, _sm, 8);
    spi3_set_dc(_pio, _sm, dc_command);
    gpio_put(_cs_pin, cs_assert);
    spi3_put8(_pio, _sm, byte);
    spi3_wait_idle(_pio, _sm);
    gpio_put(_cs_pin, cs_deassert);
}


void TftSpi3If::write_data(uint8_t byte)
{
    spi3_set_bits(_pio, _sm, 8);
    spi3_set_dc(_pio, _sm, dc_data);
    gpio_put(_cs_pin, cs_assert);
    spi3_put8(_pio, _sm, byte);
    spi3_wait_idle(_pio, _sm);
    gpio_put(_cs_pin, cs_deassert);
}


void TftSpi3If::write_data16(uint16_t value)
{
    spi3_set_bits(_pio, _sm, 16);
    spi3_set_dc(_pio, _sm, dc_data);
    gpio_put(_cs_pin, cs_assert);
    spi3_put16(_pio, _sm, value);
    spi3_wait_idle(_pio, _sm);
    gpio_put(_cs_pin, cs_deassert);
}


void TftSpi3If::write_data16_blocking(const uint16_t *buf, int count)
{
    spi3_set_bits(_pio, _sm, 16);
    spi3_set_dc(_pio, _sm, dc_data);
    gpio_put(_cs_pin, cs_assert);
    for (int i = 0; i < count; i++)
        spi3_put16(_pio, _sm, buf[i]);
    spi3_wait_idle(_pio, _sm);
    gpio_put(_cs_pin, cs_deassert);
}


void TftSpi3If::begin_data16_burst()
{
    // spi3_set_bits()/spi3_set_dc() each wait for the SM to be genuinely
    // idle before reconfiguring it, so this is safe to call regardless of
    // what the SM was doing just before - no separate wait needed here.
    spi3_set_bits(_pio, _sm, 16);
    spi3_set_dc(_pio, _sm, dc_data);
    gpio_put(_cs_pin, cs_assert);
}


volatile void *TftSpi3If::fifo_addr() const
{
    return &_pio->txf[_sm];
}


void TftSpi3If::wait_transport_idle()
{
    spi3_wait_idle(_pio, _sm);
    gpio_put(_cs_pin, cs_deassert);
}
