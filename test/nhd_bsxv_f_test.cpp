
#include "framebuffer/nhd_bsxv_f.h"

#include <cstdio>
// pico
#include "hardware/spi.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
// misc
#include "misc/sys_led.h"
// framebuffer
#include "framebuffer/font.h"
#include "framebuffer/roboto.h"
//
#include "nhd_bsxv_f_test_cfg.h"

static constexpr int spi_baud_request = 15'000'000;

// NhdBsxvF's physical size (240x320 portrait) is fixed at compile time, not
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
    printf("nhd_bsxv_f_test\n");
    printf("\n");

    NhdBsxvF fb(fb_spi_inst, fb_spi_miso_gpio, fb_spi_mosi_gpio, fb_spi_clk_gpio,
                fb_spi_cs_gpio, spi_baud_request, fb_cd_gpio, fb_rst_gpio,
                fb_led_gpio, work, work_bytes);

    int spi_baud_actual = fb.spi_freq();
    int spi_rate_max = spi_baud_actual / 8;
    printf("spi: requested %d Hz, got %d Hz (max %d bytes/sec)\n", //
           spi_baud_request, spi_baud_actual, spi_rate_max);

    fb.init();

    // Turning on the backlight here shows whatever happens to be in RAM
    // (previously displayed or random junk), so we turn it on after filling
    // the screen with something.

    reinit_screen(fb); // set rotation, fill background

    // Now turn on backlight
    fb.brightness(100);

    fb_tests_run(fb);
    return 0;
}
