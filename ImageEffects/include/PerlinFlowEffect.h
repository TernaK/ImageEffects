#pragma once

#include "ImageEffect.h"
#include "PerlinNoise.h"

struct Particle {
    static constexpr size_t MAX_HISTORY = 50;

    cv::Point2f init_position;
    cv::Point2f position;
    std::deque<cv::Point2f> history;
    float life;

    Particle(cv::Point2f position, float life) : position(position), life(life) {
        init_position = position;
        history.clear();
        history.push_back(position);
    }

    bool alive() { return life >= 0; }

    void update(const cv::Point2f& new_position) {
        position = new_position;
        history.push_back(new_position);
        if (history.size() > MAX_HISTORY)
            history.pop_front();
    }

    void draw(cv::Mat& image, bool lines = true, cv::Scalar color = {1.0, 1.0, 1.0}) {
        if (lines) {
            for (int i = 0; i < history.size() - 1; i++) {
                float mag = std::clamp(pow(float(i + 1) / 10.0f * 1.0f, 2.0f), 0.0f, 1.0f);
                cv::line(image, history[i], history[i+1], mag * color);
            }
        } else {
            for (int i = 0; i < history.size(); i++) {
                float mag = std::clamp(pow(float(i + 1) / 10.0f * 1.0f, 2.0f), 0.0f, 1.0f);
                cv::drawMarker(image, cv::Point(history[i].x, history[i].y), mag * color, cv::MARKER_SQUARE, 1, 1);
            }
        }
    }
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
            _particles.push_back(Particle(cv::Point2f(0, i), _life));
        }
    }

    void update_grid(float t) {
        // Update vectors
        float speed = 2.0;
        for (int gy = 0; gy < _pn.height() + 1; gy++) {
            for (int gx = 0; gx < _pn.width() + 1; gx++) {
                cv::Vec2f& g = _pn.grid()[gy][gx];
                float angle = atan2(g[1], g[0]);
                angle += (t - _last_t) * speed;
                g = cv::Vec2f(cos(angle), sin(angle));
            }
        }
    }

    void update_particles(float t, const cv::Mat field) {
        float delta_t = t - _last_t;
        int height = _init_image.rows;
        int width = _init_image.cols;

        for (auto& p : _particles) {
            p.life -= delta_t;

            if (p.alive() && p.position.x >= 0 && p.position.x < width && p.position.y >= 0 && p.position.y < height) {
                float angle = field.at<float>(p.position.y, p.position.x);
                cv::Point2f velocity_vector(cos(angle), sin(angle));
                auto new_position = p.position + (delta_t * _velocity * velocity_vector);

                p.update(new_position);
            }
            else {
                p = Particle(p.init_position, _life);
            }
        }
    }

    cv::Mat operator()(float t) override {
        if (_dynamic)
            update_grid(t);

        cv::Mat output = cv::Mat::zeros(_init_image.size(), CV_32FC3);

        cv::Mat field = _pn.generate(_init_image.size());

        PerlinNoise::draw_field(field, output, {0, 0, 0.7});

        update_particles(t, field);

        for (auto& p : _particles) {
            if (p.alive()) {
                p.draw(output, true, {0, 1.0, 0});
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
