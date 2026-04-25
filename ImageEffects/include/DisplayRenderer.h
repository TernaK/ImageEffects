#pragma once

#include "ImageEffect.h"

class DisplayRenderer {
public:
    DisplayRenderer(std::shared_ptr<ImageEffect> ie)
        : _ie(ie) {};

    void loop();

private:
    std::shared_ptr<ImageEffect> _ie;
    std::chrono::steady_clock::time_point _start_time;
};
