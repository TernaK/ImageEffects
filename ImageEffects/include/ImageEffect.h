#pragma once

#include <opencv2/opencv.hpp>

/// @brief Base class for image effects
class ImageEffect {
public:
    ImageEffect(cv::Size size, float t = 0)
    : _size(size), _last_t(t) {};

    /// Produce an image at given time
    /// @param t Time at which image should be produced
    virtual cv::Mat operator()(float t) = 0;

protected:
    float _last_t;  ///< Most recent time when image was produced
    cv::Size _size; ///< Canvas size
};
