#pragma once

#include "ImageEffect.h"
#include "PerlinNoise.h"
#include "Random.h"

/// @brief Particle with a life timer, velocity, position & position history tracking.
struct Particle {
    static constexpr size_t MAX_HISTORY = 50;

    cv::Point2f init_position;
    cv::Point2f init_velocity;

    cv::Point2f position;
    cv::Point2f velocity;
    std::deque<cv::Point2f> history;
    float life;
    bool track_history;

    Particle() = default;

    Particle(cv::Point2f position, cv::Point2f velocity, float life, bool track_history = true)
    : position(position), velocity(velocity), life(life), track_history(track_history) {
        init_position = position;
        init_velocity = velocity;

        history.clear();
        history.push_back(position);
    }

    /// Check if particle's life is expended
    bool alive() { return life >= 0; }

    /// Set new velocity, position, and update position tracking
    void update(const cv::Point2f& new_position, const cv::Point2f& new_velocity) {
        position = new_position;
        velocity = new_velocity;

        if (track_history) {
            history.push_back(new_position);
            if (history.size() > MAX_HISTORY)
                history.pop_front();
        }
    }

    /// Render particle to canvas
    /// @param image Canvas
    /// @param color Particle color
    /// @param draw_history If true render position history
    /// @param lines If true render position history as connected lines, dots otherwise
    void draw(cv::Mat& image, cv::Scalar color = {1.0, 1.0, 1.0}, bool draw_history = true, bool lines = true) {
        if (draw_history) {
            if (lines) {
                for (int i = 0; i < history.size() - 1; i++) {
                    float mag = std::clamp(float(i + 1) / MAX_HISTORY * 1.0f, 0.0f, 1.0f);
                    auto resultant = mag * cv::Scalar(1.0, 0, 1.0) + (1.0 - mag) * cv::Scalar(1.0, 0, 0);
                    cv::line(image, history[i], history[i+1], mag * resultant);
                }
            } else {
                for (int i = 0; i < history.size(); i++) {
                    float mag = std::clamp(float(i + 1) / MAX_HISTORY * 1.0f, 0.0f, 1.0f);
                    auto resultant = mag * cv::Scalar(1.0, 0, 1.0) + (1.0 - mag) * cv::Scalar(1.0, 0, 0);
                    cv::drawMarker(image, cv::Point(history[i].x, history[i].y), mag * resultant, cv::MARKER_SQUARE, 1, 1);
                }
            }
        } else {
            cv::drawMarker(image, cv::Point(position.x, position.y), color, cv::MARKER_SQUARE, 2, 1);
        }
    }
};

/// @brief Generate a vector field using Perlin noise and instantiate particles to move in the field.
class PerlinFlowEffect : public ImageEffect {
    PerlinNoise _pn;
    std::vector<Particle> _particles;
    bool _smooth;
    bool _dynamic;
    float _life = 8;
    int _history = 50;
    float _velocity = 40;

public:
    PerlinFlowEffect(cv::Size size, cv::Size grid_size, bool smooth = true, bool dynamic = false)
    : ImageEffect(size), _pn(grid_size), _smooth(smooth), _dynamic(dynamic) {
        for (int i = 0; i < size.height; i++) {
            Particle p;
            init_particle(p, _size, i);

            _particles.push_back(p);
        }
    }

    void update_grid(float t) {
        // Update vectors
        float speed = 5e-1;
        for (int gy = 0; gy < _pn.height() + 1; gy++) {
            for (int gx = 0; gx < _pn.width() + 1; gx++) {
                cv::Vec2f& g = _pn.grid()[gy][gx];
                float angle = atan2(g[1], g[0]);
                angle += (t - _last_t) * speed;
                g = cv::Vec2f(cos(angle), sin(angle));
            }
        }
    }

    void init_particle(Particle& p, cv::Size image_size, int index = 0) {
        cv::Point2f init_position = cv::Point2f(Random::number(image_size.width), Random::number(image_size.height));
        cv::Point2f init_velocity(10, 0);
        p = Particle(init_position, init_velocity, _life);
    }

    void update_particles(float t, const cv::Mat field) {
        float delta_t = t - _last_t;
        int height = _size.height;
        int width = _size.width;
        float accel_mag = 20;

        for (auto& p : _particles) {
            p.life -= delta_t;

            if (p.position.x >= 0 && p.position.x < width && p.position.y >= 0 && p.position.y < height) {
                float angle = field.at<float>(p.position.y, p.position.x);
                cv::Point2f accel_vector = cv::Point2f(cos(angle), sin(angle)) * accel_mag;

                // s = ut + 0.5at^2
                auto new_position = p.position + delta_t * (p.velocity + (0.5 * delta_t * accel_vector));
                // v = u + at
                auto new_velocity = p.velocity + delta_t * accel_vector;

                p.update(new_position, new_velocity);
            } else {
                auto new_position = p.position + (delta_t * p.velocity);
                p.update(new_position, p.velocity);
            }

            if (!p.alive())
                init_particle(p, _size);
        }
    }

    cv::Mat operator()(float t) override {
        if (_dynamic)
            update_grid(t);

        cv::Mat output = cv::Mat::zeros(_size, CV_32FC3);

        // Get vector field
        cv::Mat field = _pn.generate(_size) * 1.0;
        PerlinNoise::draw_field(field, output, {0.1, 0.1, 0.1});

        // Update particles
        update_particles(t, field);
        for (auto& p : _particles) {
            if (p.alive()) {
                p.draw(output, {1.0, 0, 1.0}, true, true);
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
