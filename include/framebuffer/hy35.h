#pragma once

#include <cstdint>
// pico
#include "pico/stdlib.h"
// framebuffer
#include "framebuffer/tft.h"


// Hosyond 3.5" TFT, ST7796 controller, 320x480, SPI interface
class Hy35 : public Tft
{

public:

    Hy35(TftIf &io, int rst_pin, int bk_pin, int width, int height,
         void *work = nullptr, int work_bytes = 0);

    void init();

private:

    virtual uint8_t madctl() const;
};
