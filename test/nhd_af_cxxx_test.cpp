
#include "framebuffer/nhd_af_cxxx.h"

#include <cstdio>
// pico
#include "hardware/pio.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
// misc
#include "misc/sys_led.h"
// framebuffer
#include "framebuffer/font.h"
#include "framebuffer/roboto.h"
#include "framebuffer/tft_spi3_if.h"

//                                   +----|USB|----+
// UART0_TX I2C0_SDA SPI0_RX   GP0   | 1        40 |   VBUS
// UART0_RX I2C0_SCL SPI0_CSn  GP1   | 2        39 |   VSYS
//                             GND   | 3        38 |   GND
//          I2C1_SDA SPI0_SCK  GP2   | 4        37 |   3V3_EN
//          I2C1_SCL SPI0_TX   GP3   | 5        36 |   3V3(OUT)
// UART1_TX I2C0_SDA SPI0_RX   GP4   | 6        35 |   ADC_VREF
// UART1_RX I2C0_SCL SPI0_CSn  GP5   | 7        34 |   GP28 ADC2
//                             GND   | 8        33 |   AGND
//          I2C1_SDA SPI0_SCK  GP6   | 9        32 |   GP27 ADC1     I2C1_SCL
//          I2C1_SCL SPI0_TX   GP7   | 10       31 |   GP26 ADC0     I2C1_SDA
// UART1_TX I2C0_SDA SPI1_RX   GP8   | 11       30 |   RUN
// UART1_RX I2C0_SCL SPI1_CSn  GP9   | 12       29 |   GP22
//                             GND   | 13       28 |   GND
//          I2C1_SDA SPI1_SCK GP10   | 14       27 |   GP21          I2C0_SCL UART1_RX
//          I2C1_SCL SPI1_TX  GP11   | 15       26 | * GP20          I2C0_SDA UART1_TX
// UART0_TX I2C0_SDA SPI1_RX  GP12   | 16       25 | * GP19 SPI0_TX  I2C1_SCL
// UART0_RX I2C0_SCL SPI1_CSn GP13   | 17       24 | * GP18 SPI0_SCK I2C1_SDA
//                             GND   | 18       23 |   GND
//          I2C1_SDA SPI1_SCK GP14   | 19       22 | * GP17 SPI0_CSn I2C0_SCL UART0_RX
//          I2C1_SCL SPI1_TX  GP15   | 20       21 | * GP16 SPI0_RX  I2C0_SDA UART0_TX
//                                   +-------------+

constexpr int spi3_mosi_gpio = 16;
constexpr int spi3_clk_gpio = 17;
constexpr int spi3_cs_gpio = 18;
static PIO const spi3_pio = pio0;

constexpr int rst_gpio = 19;
constexpr int led_gpio = 20;

static constexpr int spi_baud_request = 15'000'000;

// NhdAfCxxx's physical size (240x320 portrait) is fixed at compile time, not
// a constructor argument; fb_width/fb_height here are just the landscape
// logical size used by fb_tests.h's layout math. Same physical size as
// Ws24, so the same font picks apply.
static constexpr int fb_width = 320;
static constexpr int fb_height = 240;
static const Font &font = roboto_24;           // height/10
static const Font &nav_font = roboto_20;        // ~fb_height/11.5
static const Font &id_font = roboto_36;         // ~fb_height/6.67
static const Font &toots_font = roboto_26;      // ~fb_height/9.4
static const Font &slider_font = roboto_26;     // ~fb_height/9.4

static constexpr int work_bytes = 128;
static uint8_t work[work_bytes];

#include "fb_tests.h"


int main()
{
    stdio_init_all();

    SysLed::init();
    SysLed::pattern(50, 950);

    while (!stdio_usb_connected()) {
        tight_loop_contents();
        SysLed::loop();
    }

    sleep_ms(10);

    SysLed::off();

    printf("\n");
    printf("nhd_af_cxxx_test\n");
    printf("\n");

    TftSpi3If io(spi3_pio, spi3_mosi_gpio, spi3_clk_gpio, spi3_cs_gpio, spi_baud_request);

    NhdAfCxxx fb(io, rst_gpio, led_gpio, work, work_bytes);

    // spi_freq() isn't valid until fb.init() has run - that's what actually
    // calls down to TftSpi3If::init(), which computes it.
    fb.init();

    int spi_baud_actual = fb.spi_freq();
    int spi_rate_max = spi_baud_actual / 8;
    printf("spi: requested %d Hz, got %d Hz (max %d bytes/sec)\n", //
           spi_baud_request, spi_baud_actual, spi_rate_max);

    // Turning on the backlight here shows whatever happens to be in RAM
    // (previously displayed or random junk), so we fill the screen with
    // something first; fb_tests_run() turns the backlight on (to its own
    // default) once that's done.

    reinit_screen(fb); // set rotation, fill background

    fb_tests_run(fb);
    return 0;
}
