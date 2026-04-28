#pragma once

#include <opencv2/opencv.hpp>
#include <stdlib.h>

/// @brief Generate random numbers and vectors
class Random {
public:
    /// Generate random positive number
    /// @param max Number upper limit
    template<typename T = float>
    static T number(T max = 1.0f) {
        return (arc4random() % 1000) / 1000.0f * max;
    }

    /// Generate random angle in radians
    /// @param max Angle upper limit
    template<typename T = float>
    static T angle(T max = 2.0 * M_PI) {
        return Random::number(max);
    }

    /// Generate random vector
    /// @param norm Vector norm
    template<typename T = float>
    static cv::Point_<T> vector2d(T norm = 1.0f) {
        T angle = Random::number(2.0 * M_PI);
        return cv::Point_<T>(cos(angle), sin(angle)) * norm;
    }
};
