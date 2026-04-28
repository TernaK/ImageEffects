#pragma once

#include "ImageEffect.h"

/// @brief Inifinite loop which requests images from and effects generator and renders to a window
class DisplayRenderer {
public:
    /// Provide an effects generator for rendering
    DisplayRenderer(std::shared_ptr<ImageEffect> ie)
    : _ie(ie) {};

    /// Query effects generator for frames infinitely and display in window. May be broken using ESC key.
    void loop();

private:
    std::shared_ptr<ImageEffect> _ie; ///< Effects generator
    std::chrono::steady_clock::time_point _start_time; ///< Initialization timestamp
};
