#pragma once

#include "ImageEffect.h"
#include "PerlinNoise.h"
#include "Random.h"

struct Particle {
    static constexpr size_t MAX_HISTORY = 50;

    cv::Point2f init_position;
    cv::Point2f init_velocity;

    cv::Point2f position;
    cv::Point2f velocity;
    std::deque<cv::Point2f> history;
    float life;

    Particle(cv::Point2f position, cv::Point2f velocity, float life) : position(position), velocity(velocity), life(life) {
        init_position = position;
        init_velocity = velocity;

        history.clear();
        history.push_back(position);
    }

    bool alive() { return life >= 0; }

    void update(const cv::Point2f& new_position, const cv::Point2f& new_velocity) {
        position = new_position;
        velocity = new_velocity;

        history.push_back(new_position);
        if (history.size() > MAX_HISTORY)
            history.pop_front();
    }

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

        cv::Point2f center(image.cols / 2, + image.rows / 2);

        for (int i = 1; i < image.rows * 10; i += 2) {
            float angle = Random::angle();
            cv::Point2f position(0.5, 0);

            angle = Random::angle();
            cv::Point2f velocity(0, 0);

//            float delta = cv::norm(center) / (10);

//            _particles.push_back(Particle(center + delta * position, _velocity * velocity, _life));
            cv::Point2f init_position(arc4random() % image.cols, arc4random() % image.rows);
            _particles.push_back(Particle(init_position, _velocity * velocity, _life));
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

    void update_particles(float t, const cv::Mat field) {
        float delta_t = t - _last_t;
        int height = _init_image.rows;
        int width = _init_image.cols;
        float accel_mag = 50;

        for (auto& p : _particles) {
            p.life -= delta_t;

//            if (!p.alive()) {
//                p = Particle(p.init_position, p.init_velocity, _life);
//            }
            if (p.position.x >= 0 && p.position.x < width && p.position.y >= 0 && p.position.y < height) {
                float angle = field.at<float>(p.position.y, p.position.x);

                cv::Point2f accel_vector(cos(angle), sin(angle));
                cv::Point2f delta_velocity = delta_t * accel_vector * accel_mag;

                auto new_velocity = p.velocity + delta_velocity;
                auto new_position = p.position + delta_t * accel_mag * accel_vector;
//                auto new_position = p.position + (delta_t * new_velocity);
                p.update(new_position, new_velocity);
            } else {
                auto new_position = p.position + (delta_t * p.velocity);
                p.update(new_position, p.velocity);

                p = Particle(p.init_position, p.init_velocity, _life);
            }
        }
    }

    cv::Mat operator()(float t) override {
        if (_dynamic)
            update_grid(t);

        cv::Mat output = cv::Mat::zeros(_init_image.size(), CV_32FC3);

        cv::Mat field = _pn.generate(_init_image.size()) * 2.0;

        PerlinNoise::draw_field(field, output, {0.1, 0.1, 0.1});

        update_particles(t, field);

        for (auto& p : _particles) {
            if (p.alive()) {
                p.draw(output, {1.0, 0, 1.0}, true, false);
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
