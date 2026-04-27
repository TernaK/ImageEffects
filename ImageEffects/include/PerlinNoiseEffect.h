#pragma once

#include "ImageEffect.h"
#include "PerlinNoise.h"

class PerlinNoiseEffect : public ImageEffect {
    PerlinNoise _pn;
    bool _smooth;
    bool _dynamic;

public:
    PerlinNoiseEffect(cv::Mat image, cv::Size grid_size, bool smooth = true, bool dynamic = true)
    : ImageEffect(image), _pn(grid_size), _smooth(smooth), _dynamic(dynamic) {}

    void update_grid(float t) {
        // Update vectors
        float speed = 1.0;
        for (int gy = 0; gy < _pn.height() + 1; gy++) {
            for (int gx = 0; gx < _pn.width() + 1; gx++) {
                cv::Vec2f& g = _pn.grid()[gy][gx];
                float angle = atan2(g[1], g[0]);
                angle += (t - _last_t) * speed;
                g = cv::Vec2f(cos(angle), sin(angle));
            }
        }
    }

    cv::Mat operator()(float t) override {
        if (_dynamic)
            update_grid(t);

        cv::Mat output = _pn.generate(_init_image.size(), _smooth);

        _last_t = t;
        return output;
    }

    static cv::Mat make_image(cv::Size resolution = {360, 360}) {
        cv::Mat image(resolution, CV_32F);
        image = 0;
        return image;
    }
};

class OctavePerlinNoiseEffect : public ImageEffect {
    std::vector<std::shared_ptr<PerlinNoiseEffect>> _octaves;
    float _attenuation;

public:
    OctavePerlinNoiseEffect(cv::Mat image, int octaves, float attenuation, cv::Size grid_size,
                            bool smooth = true, bool dynamic = true)
    : ImageEffect(image), _attenuation(attenuation) {
        for (int i = 0; i < octaves; i++) {
            _octaves.push_back(std::make_shared<PerlinNoiseEffect>(image, grid_size, smooth, dynamic));
            grid_size.width *= 2;
            grid_size.height *= 2;
        }
    }

    cv::Mat operator()(float t) override {
        cv::Mat output = _init_image.clone();

        float amplitude = 1.0;
        for (int i = 0; i < _octaves.size(); i++) {
            output += amplitude * _octaves[i]->operator()(t);
            amplitude *= _attenuation;
        }

        return output;
    }
};
