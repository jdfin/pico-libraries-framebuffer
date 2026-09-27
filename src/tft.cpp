
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <utility>
// pico
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"
// framebuffer
#include "framebuffer/color.h"
#include "framebuffer/font.h"
#include "framebuffer/framebuffer.h"
#include "framebuffer/pixel_565.h"
#include "framebuffer/pixel_image.h"
//
#include "framebuffer/tft.h"
// misc
#include "misc/dbg_gpio.h"
#include "misc/dma_extra.h"
#include "misc/util.h"


// Backlight PWM: 8-bit duty resolution (plenty for a backlight) at a fixed
// frequency, well above any visible flicker or camera-shutter banding, and
// with no inductor in the LED backlight circuit to make a switching whine
// worth avoiding. clkdiv is computed from the actual sys clock at runtime
// (clock_get_hz(), not a hardcoded divisor) so this comes out right on both
// RP2040 and RP235x regardless of their clk_sys defaults.
//
// pwm_init() reconfigures bk_pin's whole slice (wrap/clkdiv are shared by
// both channels of a slice) - fine today since no test also drives PWM on
// bk_pin's slice-mate pin, but would need coordinating if one ever did.
static constexpr uint16_t bk_pwm_wrap = 255;
static constexpr float bk_pwm_freq_hz = 5000.0f;


Tft::Tft(TftIf &io, int rst_pin, int bk_pin, int width, int height, //
                 void *work, int work_bytes) :
    Framebuffer(width, height),
    _io(io),
    _rst_pin(rst_pin),
    _bk_pin(bk_pin),
    // _dma_ch
    // _dma_cfg
    _dma_running(false),
    _dma_pixel(0),
    _pix_buf((Pixel565 *)work),
    _pix_buf_len(work_bytes / sizeof(Pixel565)),
    // _ops[]
    _ops_stall_cnt(0),
    _op_next(0),
    _op_free(0)
{
    assert(_rst_pin >= 0);

    //DbgGpio::init(28);

    gpio_init(_rst_pin);
    gpio_put(_rst_pin, rst_assert);
    gpio_set_dir(_rst_pin, gpio_out);

    if (_bk_pin >= 0) {
        gpio_set_function(_bk_pin, GPIO_FUNC_PWM);
        pwm_config cfg = pwm_get_default_config();
        float clkdiv = (float)clock_get_hz(clk_sys) /
                       (bk_pwm_freq_hz * (bk_pwm_wrap + 1));
        pwm_config_set_clkdiv(&cfg, clkdiv);
        pwm_config_set_wrap(&cfg, bk_pwm_wrap);
        pwm_init(pwm_gpio_to_slice_num(_bk_pin), &cfg, true);
        brightness(0); // off
    } else {
        // no control, assume it's max brightness (e.g. pulled up)
        brightness(100);
    }

    // DMA is only used for pixel data
    _dma_ch = dma_claim_unused_channel(true);
    _dma_cfg = dma_channel_get_default_config(_dma_ch);
    channel_config_set_dreq(&_dma_cfg, _io.dreq());
    channel_config_set_transfer_data_size(&_dma_cfg, DMA_SIZE_16);
    channel_config_set_write_increment(&_dma_cfg, false); // write to transport
    dmax_irqn_set_channel_handler(0, _dma_ch, dma_raw_handler, (intptr_t)this);
    dmax_irqn_set_channel_enabled(0, _dma_ch, true);
}


Tft::~Tft()
{
    // probably much to do
}


void Tft::write_cmds(const uint16_t *b, int b_len)
{
    assert(b != nullptr && b_len >= 1);
    while (b_len > 0) {
        uint16_t next = *b++;
        if ((next & wr_mask) == wr_cmd) {
            _io.write_cmd(uint8_t(next));
        } else if ((next & wr_mask) == wr_data) {
            _io.write_data(uint8_t(next));
        } else if ((next & wr_mask) == wr_delay_ms) {
            sleep_ms(uint8_t(next));
        }
        b_len--;
    }
}


// pulse hardware reset signal to controller
void Tft::hw_reset(int pulse_us)
{
    gpio_put(_rst_pin, rst_assert);
    sleep_us(pulse_us);
    gpio_put(_rst_pin, rst_deassert);
}


// 'brightness_pct' is 0%-100% brightness, PWM'd on bk_pin (see constructor).
void Tft::brightness(int brightness_pct)
{
    if (brightness_pct < 0)
        brightness_pct = 0;
    else if (brightness_pct > 100)
        brightness_pct = 100;

    _brightness_pct = brightness_pct;

    if (_bk_pin >= 0) {
        // At brightness_pct == 100, level comes out one past bk_pwm_wrap,
        // which the PWM hardware treats as "always high" - full-on with no
        // switching, not a 255/256 duty cycle.
        uint16_t level =
            (uint16_t)((uint32_t)brightness_pct * (bk_pwm_wrap + 1) / 100);
        pwm_set_gpio_level(_bk_pin, level);
    }
}


void Tft::set_rotation(Rotation r)
{
    Framebuffer::set_rotation(r);

    wait_idle(); // wait for any queued dmas to finish

    _io.write_cmd(MADCTL);
    _io.write_data(madctl());
}


void Tft::set_window(uint16_t hor, uint16_t ver, uint16_t wid, uint16_t hgt)
{
    //DbgGpio d(28);

    _io.write_cmd(CASET);

    const uint h2 = hor + wid - 1;
    _io.write_data(uint8_t(hor >> 8));
    _io.write_data(uint8_t(hor));
    _io.write_data(uint8_t(h2 >> 8));
    _io.write_data(uint8_t(h2));

    _io.write_cmd(RASET);

    const uint v2 = ver + hgt - 1;
    _io.write_data(uint8_t(ver >> 8));
    _io.write_data(uint8_t(ver));
    _io.write_data(uint8_t(v2 >> 8));
    _io.write_data(uint8_t(v2));
}


void Tft::pixel(int hor, int ver, const Color c)
{
    wait_idle();

    set_window(hor, ver, 1, 1);
    _io.write_cmd(RAMWR);
    const Pixel565 p = c; // Pixel565::operator= converts from Color
    _io.write_data16(p.value());
}


// line extends right from ('h', 'v') for 'len' pixels
void Tft::hline(int h, int v, int wid, const Color c)
{
    fill_rect(h, v, wid, 1, c);
}


// line extends up or down (positive direction) from ('h', 'v') for 'len' pixels
void Tft::vline(int h, int v, int hgt, const Color c)
{
    fill_rect(h, v, 1, hgt, c);
}


void Tft::line(int h1, int v1, int h2, int v2, const Color c)
{
    if (h1 == h2) {
        if (v1 > v2)
            std::swap(v1, v2);
        vline(h1, v1, v2 - v1 + 1, c);
    } else if (v1 == v2) {
        if (h1 > h2)
            std::swap(h1, h2);
        hline(h1, v1, h2 - h1 + 1, c);
    } else {
        Framebuffer::line(h1, v1, h2, v2, c);
    }
}


// ('hor', 'ver') is the top left pixel
// 'wid' and 'hgt' are the number of pixels in each direction
void Tft::draw_rect(int hor, int ver, int wid, int hgt, const Color c)
{
    // no need to paint the corner pixels twice
    hline(hor, ver, wid - 1, c);               // top
    vline(hor + wid - 1, ver, hgt - 1, c);     // right
    hline(hor + 1, ver + hgt - 1, wid - 1, c); // bottom
    vline(hor, ver + 1, hgt - 1, c);           // left
}


void Tft::dma_handler()
{
    _io.wait_transport_idle();

    // anything new to do?
    if (ops_empty()) {
        busy(false);
    } else {
        if (_ops[_op_next].op == AsyncOp::Fill) {
            const int hor = _ops[_op_next].hor;
            const int ver = _ops[_op_next].ver;
            const int wid = _ops[_op_next].wid;
            const int hgt = _ops[_op_next].hgt;
            _dma_pixel = _ops[_op_next].pixel;
            __dmb(); // _dma_pixel must be in memory before starting dma
            set_window(hor, ver, wid, hgt);
            _io.write_cmd(RAMWR);
            _io.begin_data16_burst();
            channel_config_set_read_increment(&_dma_cfg, false);
            dma_channel_configure(_dma_ch, &_dma_cfg, _io.fifo_addr(),
                                  &_dma_pixel, wid * hgt, true); // go!
        } else if (_ops[_op_next].op == AsyncOp::Copy) {
            const int hor = _ops[_op_next].hor;
            const int ver = _ops[_op_next].ver;
            const int wid = _ops[_op_next].wid;
            const int hgt = _ops[_op_next].hgt;
            const void *pixels = _ops[_op_next].pixels;
            set_window(hor, ver, wid, hgt);
            _io.write_cmd(RAMWR);
            _io.begin_data16_burst();
            channel_config_set_read_increment(&_dma_cfg, true);
            dma_channel_configure(_dma_ch, &_dma_cfg, _io.fifo_addr(),
                                  pixels, wid * hgt, true); // go!
        } else {
            assert(false); // only Fill and Copy
        }
        op_next_inc();
    }
}


// ('hor', 'ver') is the top left pixel
// 'wid' and 'hgt' are the number of pixels in each direction
void Tft::fill_rect(int hor, int ver, int wid, int hgt, const Color c)
{
    int h2 = hor + wid - 1;
    if (h2 >= width())
        h2 = width() - 1;
    if (hor > h2)
        return;
    wid = h2 - hor + 1;

    int v2 = ver + hgt - 1;
    if (v2 >= height())
        v2 = height() - 1;
    if (ver > v2)
        return;
    hgt = v2 - ver + 1;

    if (ops_full()) {
        // wait for space
        _ops_stall_cnt++;
        while (ops_full())
            tight_loop_contents();
    }

    Pixel565 p = c; // Pixel565::operator= converts from Color

    _ops[_op_free].op = AsyncOp::Fill;
    _ops[_op_free].hor = uint16_t(hor);
    _ops[_op_free].ver = uint16_t(ver);
    _ops[_op_free].wid = uint16_t(wid);
    _ops[_op_free].hgt = uint16_t(hgt);
    _ops[_op_free].pixel = p.value();

    // _ops[] must be visible in memory (to isr) before updating _op_free
    __dmb();

    uint32_t irq_state = save_and_disable_interrupts();

    op_free_inc();

    // force interrupt to start if it's there's not something already running
    if (!busy()) {
        dmax_irqn_set_channel_force(0, _dma_ch);
        busy(true);
    }

    restore_interrupts(irq_state);

} // void Tft::fill_rect


// write array of pixels to screen
// ('hor', 'ver') is the top left pixel
// 'image' points to a PixelImage, which contains width, height, and the
// pixel data. It cannot be reused or reallocated until the write is finished,
// which will be some time after this function returns.
// 'align' controls horizontal alignment: left (default), center, or right.
void Tft::write(int hor, int ver, const PixelImageHdr *image, HAlign align)
{
    // adjust for alignment
    if (align == HAlign::Center)
        hor -= image->wid / 2;
    else if (align == HAlign::Right)
        hor -= image->wid;

    // can't start off the left edge or above the top
    if (hor < 0 || ver < 0)
        return;

    // Don't try to go past right edge. Since (hor + wid) is the first pixel
    // after the one we're writing, (hor + wid) == width is okay
    if ((hor + image->wid) > width())
        return;

    // Don't try to go past bottom edge.
    if ((ver + image->hgt) > height())
        return;

    if (ops_full()) {
        // Wait for space. We want waiting here to be rare. Very rare.
        _ops_stall_cnt++;
        while (ops_full())
            tight_loop_contents();
    }

    const void *pixels = reinterpret_cast<const PixelImage565 *>(image)->pixels;

    // if pixels is in XIP memory (flash), use non-cached access
    if (is_xip(pixels))
        pixels = xip_nocache(pixels);

    _ops[_op_free].op = AsyncOp::Copy;
    _ops[_op_free].hor = uint16_t(hor);
    _ops[_op_free].ver = uint16_t(ver);
    _ops[_op_free].wid = uint16_t(image->wid);
    _ops[_op_free].hgt = uint16_t(image->hgt);
    _ops[_op_free].pixels = pixels;

    // _ops[] must be visible in memory (to isr) before updating _op_free
    __dmb();

    uint32_t irq_state = save_and_disable_interrupts();

    op_free_inc();

    // force interrupt to start if it's there's not something already running
    if (!busy()) {
        dmax_irqn_set_channel_force(0, _dma_ch);
        busy(true);
    }

    restore_interrupts(irq_state);

} // Tft::write


// Write a number to the screen as a series of digit images.
//
// The digit images are pre-created, normally at compile time and stored in
// flash.
//
// Writing a number to the screen this way is asynchronous. The call returns
// in a few tens of usec whereas rendering the digits (as in print) can take
// several milliseconds.
//
// Negative numbers TBD, but could work through this same function. Where's
// the minus image?
//
// hor, ver top left/center/right pixel of the number (see 'align')
// num      number to write
// dig_img  array of 10 pointers to PixelImageInfo for each digit 0..9, i.e.
//          const PixelImageInfo *dig_img[10];
// align    Left (default), Center, Right
// wid, hgt receive the width and height of the rendered number in pixels
//
void Tft::write(int hor, int ver, int num, const PixelImageHdr *dig[10],
                    HAlign align, int *wid, int *hgt)
{
    assert(num >= 0);

    // extract digits in reverse order
    constexpr int max_digits = 11; // "-2147483648"
    const PixelImageHdr *num_digits[max_digits];
    int ndigits = 0;
    int total_wid = 0;
    do {
        int d = num % 10;
        num = num / 10;
        assert(d >= 0 && d <= 9);
        assert(ndigits < max_digits);
        num_digits[ndigits++] = dig[d];
        total_wid += dig[d]->wid;
    } while (num > 0);

    // adjust for alignment
    if (align == HAlign::Center)
        hor -= total_wid / 2;
    else if (align == HAlign::Right)
        hor -= total_wid;
    // else align == HAlign::Left, no adjustment

    // write digits in correct order
    for (int i = ndigits - 1; i >= 0; i--) {
        write(hor, ver, num_digits[i]);
        hor += num_digits[i]->wid;
    }

    if (wid != nullptr)
        *wid = total_wid;

    if (hgt != nullptr)
        *hgt = dig[0]->hgt;
}


// Print one character to screen
//
// 'hor', 'ver' top left pixel of the character cell
// 'c'          character to print
// 'font'       font to use
// 'fg', 'bg'   the foreground (glyph) and background (margin) colors
// 'align'      left (default), center, or right
//
// The combination of 'c' and 'font' determine how big the character cell is.
//
// If the character would extend off the screen, nothing is printed.
//
// It doesn't make sense to do this asynchronously (with dma) because of the
// rendering of the glyph into _pix_buf. Even if _pix_buf were big enough to
// hold the entire character, we'd have to wait for the dma to finish before
// reusing _pix_buf for the next character.
void Tft::print(int hor, int ver, char c, const Font &font, //
                    const Color fg, const Color bg, HAlign align)
{
    if (!font.printable(c))
        return;

    int ci = int(c);

    // handle alignment
    if (align != HAlign::Left) {
        // Right-aligned or centered, back up horizontal location.
        int adjust = font.info[ci].x_adv; // char width in pixels
        if (align == HAlign::Center)
            adjust = adjust / 2; // centered
        hor -= adjust;
    }

    // Can't start off the left edge or above the top.
    if (hor < 0 || ver < 0)
        return;

    // Start of glyph data - each byte is a grayscale.
    const uint8_t *gs = font.data + font.info[ci].off;

    // These are used to "raster through" the glyph data.
    const int8_t x_off = font.info[ci].x_off;
    const int8_t y_off = font.info[ci].y_off;
    const int8_t wid = font.info[ci].w;
    const int8_t hgt = font.info[ci].h;
    const int8_t x_adv = font.info[ci].x_adv;

    // Don't try to go past right edge. Since (hor + x_adv) is the first pixel
    // after the one we're printing, (hor + x_adv) == width is okay.
    if ((hor + x_adv) > width())
        return;

    // Don't try to go past bottom edge.
    if ((ver + font.y_adv) > height())
        return;

    // The character's 'box' is [hor...hor+x_adv) horizontally, and
    // [ver...ver+y_adv) vertically; the pixel at (hor, ver) will be filled,
    // and the pixel at (hor+x_adv, ver+y_adv) will not.
    //
    // But the glyph does not necessarly fit completely in the box (and often
    // doesn't); any of these can be true, and we just crop any part of the
    // glyph outside the character's box:
    //   x_off can be negative - glyph extends left of 'hor'
    //   y_off can be negative - glyph extends above 'ver'
    //   x_off + wid can extend past x_adv
    //   y_off + hgt can extend below y_adv
    //
    // Fonts that make a habit of extending outside the character box don't
    // render nicely. Many do it occasionally and you don't notice.

    // Wait for any queued dmas to finish.
    wait_idle();

    // Set spi transfer window - all pixels in this window will be filled.
    set_window(hor, ver, font.info[ci].x_adv, font.y_adv);

    _io.write_cmd(RAMWR);

    // Iterate through entire character box. Margins around the glyph are
    // background. Pixels in the glyph are interpolated from the glyph data.
    // As we iterate through the character box, we gradually fill up the
    // working buffer _pix_buf, which might be smaller than the character box.
    // When _pix_buf fills up, we write it out to the display then start over.

    // Assigning to _pix_buf[] from Color uses Pixel565::operator=.

    Pixel565 bg_pix = bg; // convert once

    int p = 0; // indexes through _pix_buf
    // row, col covers character box
    for (int row = 0; row < font.y_adv; row++) {
        for (int col = 0; col < x_adv; col++) {
            // See if the glyph covers this pixel.
            if (row >= y_off && row < (y_off + hgt) && //
                col >= x_off && col < (x_off + wid)) {
                // Yes.
                int g_row = row - y_off;
                assert(g_row >= 0 && g_row < hgt);
                int g_col = col - x_off;
                assert(g_col >= 0 && g_col < wid);
                uint8_t gray = gs[g_row * wid + g_col];
                _pix_buf[p++] = Color::interpolate(gray, bg, fg);
            } else {
                // No, outside glyph's margins.
                _pix_buf[p++] = bg_pix;
            }
            if (p >= _pix_buf_len) {
                // Send buffer and restart it.
                _io.write_data16_blocking((const uint16_t *)(_pix_buf), _pix_buf_len);
                p = 0;
            }
        }
    }
    // Send final (partial) buffer if necessary.
    if (p > 0)
        _io.write_data16_blocking((const uint16_t *)(_pix_buf), p);
}
