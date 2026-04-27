#pragma once

#include "ImageEffect.h"
#include "PerlinNoise.h"

struct Particle {
    cv::Vec2f init_position;
    cv::Vec2f position;
    std::deque<cv::Vec2f> history;
    float life;

    Particle(cv::Vec2f position, float life) : position(position), life(life) {
        init_position = position;
        history.push_back(position);
    }

    bool alive() { return life >= 0; }
};

class PerlinFlowEffect : public ImageEffect {
    PerlinNoise _pn;
    std::vector<Particle> _particles;
    bool _smooth;
    bool _dynamic;
    float _life = 10;
    int _history = 50;
    float _velocity = 75;

public:
    PerlinFlowEffect(cv::Mat image, cv::Size grid_size, bool smooth = true, bool dynamic = false)
    : ImageEffect(image), _pn(grid_size), _smooth(smooth), _dynamic(dynamic) {
        for (int i = 1; i < image.rows; i += 2) {
            _particles.push_back(Particle(cv::Vec2f(0, i), _life));
        }
    }

    void update_grid(float t) {
        // Update vectors
        float speed = 5.0;
        for (int gy = 0; gy < _pn.height() + 1; gy++) {
            for (int gx = 0; gx < _pn.width() + 1; gx++) {
                cv::Vec2f& g = _pn.grid()[gy][gx];
                float angle = atan2(g[1], g[0]);
                angle += (t - _last_t) * speed;
                g = cv::Vec2f(cos(angle), sin(angle));
            }
        }
    }

    void update_particles(float t) {
        float delta_t = t - _last_t;
        cv::Mat field = _pn.generate(_init_image.size());
        int height = _init_image.rows;
        int width = _init_image.cols;

        for (auto& p : _particles) {
            p.life -= delta_t;

           if (p.alive() && p.position[0] >= 0 && p.position[0] < width && p.position[1] >= 0 && p.position[1] < height) {
                float angle = field.at<float>(p.position[1], p.position[0]);
                cv::Vec2f velocity_vector(cos(angle), sin(angle));
                p.position += delta_t * _velocity * velocity_vector;
                p.history.push_back(p.position);
                if (p.history.size() > _history)
                    p.history.pop_front();
            }
            else {
                p = Particle(p.init_position, _life);
            }
        }
    }

    cv::Mat operator()(float t) override {
        if (_dynamic)
            update_grid(t);

        update_particles(t);

        cv::Mat output = _init_image.clone();

        for (auto& p : _particles) {
            if (p.alive()) {
                for (int i = 0; i < p.history.size(); i++) {
                    float color = std::clamp(pow(float(i + 1) / 10.0f * 1.0f, 2.0f), 0.0f, 1.0f);
                    cv::drawMarker(output, cv::Point(p.history[i][0], p.history[i][1]), {color, color, color}, cv::MARKER_SQUARE, 1, 1);
                }
            }
        }

        _last_t = t;
        return output;
    }

    static cv::Mat make_image(cv::Size resolution = {360, 360}) {
        cv::Mat image(resolution, CV_32F);
        image = 0;
        return image;
    }
};
