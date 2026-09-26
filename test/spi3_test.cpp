
#include "spi3.pio.h"

#include <cstdint>
#include <cstdio>
// pico
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"

//                     +-----| USB |-----+
//                  D0 | 1            40 | VBUS
//                  D1 | 2            39 | VSYS
//                 GND | 3            38 | GND
//  (3w)   CS       D2 | 4            37 | 3V3_EN
//  (3w)  CLK       D3 | 5            36 | 3V3_OUT
//  (3w)  DAT       D4 | 6            35 | AREF
//                  D5 | 7            34 | D28
//                 GND | 8            33 | AGND
//                     | :             : |
//                     +-----------------+

static constexpr int cs_gpio = 2;
static constexpr int clk_gpio = 3;
static constexpr int dat_gpio = 4;

static constexpr float clk_hz = 1'000'000; // slow enough to read easily on a scope

// One "command-like" value (D/C=0),
// sent in 8-bit mode
static constexpr uint8_t cmd_byte = 0x36;

// A few 16-bit "pixel-like" values (D/C=1),
// sent in 16 bit mode (hi byte first)
constexpr uint16_t pixel_words[] = {0xa55a, 0x1234};
constexpr int pixel_words_len = sizeof(pixel_words) / sizeof(pixel_words[0]);

// chip select active low
static constexpr bool cs_assert = false;
static constexpr bool cs_deassert = true;

static constexpr bool dc_command = false;   // lo means command
static constexpr bool dc_data = true;       // hi means data


int main()
{
    stdio_init_all();

#if 0
    while (!stdio_usb_connected())
        tight_loop_contents();

    sleep_ms(10);

    printf("\n");
    printf("spi3_test\n");
    printf("\n");
#endif

    gpio_init(cs_gpio);
    gpio_set_dir(cs_gpio, GPIO_OUT);
    gpio_put(cs_gpio, cs_deassert);

    PIO pio = pio0;
    uint sm = spi3_init(pio, dat_gpio, clk_gpio, clk_hz);

#if 1
    int dma_ch = dma_claim_unused_channel(true);
    dma_channel_config dma_cfg = dma_channel_get_default_config(dma_ch);
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, true);
    channel_config_set_write_increment(&dma_cfg, false);
    channel_config_set_dreq(&dma_cfg, pio_get_dreq(pio, sm, true));
#endif

    printf("dat=%d clk=%d cs=%d, clk target %g Hz\n", //
           dat_gpio, clk_gpio, cs_gpio, clk_hz);
    printf("sending cmd byte 0x%02x + %d pixel words (via DMA) every loop\n", //
           cmd_byte, pixel_words_len);

    while (true) {
        gpio_put(cs_gpio, cs_assert);

        // command
        spi3_set_bits(pio, sm, 8);
        spi3_set_dc(pio, sm, dc_command);
        spi3_put8(pio, sm, cmd_byte);

#if 1
        // data
        spi3_set_bits(pio, sm, 16);
        spi3_set_dc(pio, sm, dc_data);
        dma_channel_configure(dma_ch, &dma_cfg, &pio->txf[sm], pixel_words,
                              pixel_words_len, true);
        dma_channel_wait_for_finish_blocking(dma_ch);
#elif 0
        // data
        spi3_set_bits(pio, sm, 16);
        spi3_set_dc(pio, sm, dc_data);
        for (int i = 0; i < pixel_words_len; i++)
            spi3_put16(pio, sm, pixel_words[i]);
#endif

        spi3_wait_idle(pio, sm);

        gpio_put(cs_gpio, cs_deassert);

        sleep_ms(5);
    }

    return 0;
}
