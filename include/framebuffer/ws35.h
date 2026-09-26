#pragma once

#include <cstdint>
// pico
#include "pico/stdlib.h"
// framebuffer
#include "framebuffer/tft.h"


class Ws35 : public Tft
{

public:

    Ws35(TftIf &io, int rst_pin, int bk_pin, int width, int height,
         void *work = nullptr, int work_bytes = 0);

    void init();

private:

    virtual uint8_t madctl() const;
};
