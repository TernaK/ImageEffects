#pragma once

#include <opencv2/opencv.hpp>

class ImageEffect {
public:
    ImageEffect(cv::Mat image, float t = 0)
        : _init_image(image), _last_t(t) {};

    virtual cv::Mat operator()(float t) = 0;

protected:
    float _last_t;
    cv::Mat _init_image;
};
