#pragma once

#include "ImageEffect.h"

/// @brief Basic noise video generator
class NoiseEffect : public ImageEffect {
    float _amplitude;
    float _frequency;

public:
    NoiseEffect(cv::Size size, float amplitude = 0.2, float f = 1)
    : ImageEffect(size), _amplitude(amplitude), _frequency(f) {}

    cv::Mat operator()(float t) override {
        cv::Mat output(_size, CV_32F, cv::Scalar(0.5));

        for (int y = 0; y < output.rows; y++) {
            for (int x = 0; x < output.cols; x++) {
                float d = arc4random() % (output.rows / 2);
                float delta = _amplitude * sin(2.0 * M_PI * _frequency * d * (t - _last_t));
                output.at<float>(y, x) = std::clamp(output.at<float>(y, x) + delta, 0.0f, 1.0f);
            }
        }

        _last_t = t;
        return output;
    }

    static cv::Mat make_image(cv::Size resolution = {360, 360}) {
        cv::Mat image(resolution, CV_32F);
        image = 0.5;
        return image;
    }
};
