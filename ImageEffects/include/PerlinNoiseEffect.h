#pragma once

#include "ImageEffect.h"

class PerlinNoiseEffect : public ImageEffect {
    int _gheight;
    int _gwidth;
    float _block_width;
    float _block_height;
    std::vector<std::vector<cv::Vec2f>> _grid;
    bool _smooth;
    bool _dynamic;

public:
    PerlinNoiseEffect(cv::Mat image, int grid_height, int grid_width, bool smooth = true, bool dynamic = true)
    : ImageEffect(image), _gheight(grid_height), _gwidth(grid_width), _smooth(smooth), _dynamic(dynamic) {
        // Make grid
        _grid = std::vector<std::vector<cv::Vec2f>>(_gheight + 1, std::vector<cv::Vec2f>(_gwidth + 1));
        for (int gy = 0; gy < _gheight + 1; gy++) {
            for (int gx = 0; gx < _gwidth + 1; gx++) {
                float angle = (arc4random() % 360) / 360.0f * 2.0 * M_PI;
                cv::Vec2f vec(cos(angle), sin(angle));
                _grid[gy][gx] = vec;
            }
        }

        _block_width = image.cols / _gwidth;
        _block_height = image.rows / _gheight;
    }

    void update_grid(float t) {
        // Update vectors
        float speed = 5.0;
        for (int gy = 0; gy < _gheight + 1; gy++) {
            for (int gx = 0; gx < _gwidth + 1; gx++) {
                cv::Vec2f& g = _grid[gy][gx];
                float angle = atan2(g[1], g[0]);
                angle += (t - _last_t) * speed;
                g = cv::Vec2f(cos(angle), sin(angle));
            }
        }
    }

    void get_cell(cv::Vec2f &v_bl, cv::Vec2f &v_br, cv::Vec2f &v_tl, cv::Vec2f &v_tr, int x, int x0, int x1, int y, int y0, int y1) {
        auto smooth = [](cv::Vec2f vec) -> cv::Vec2f {
            float x = vec[0];
            float y = vec[1];
            x = 6 * pow(x, 5) - 15 * pow(x, 4) + 10 * pow(x, 3);
            y = 6 * pow(y, 5) - 15 * pow(y, 4) + 10 * pow(y, 3);
            return {x, y};
        };

        v_tl = cv::Vec2f((x - x0 * _block_width) / _block_width, (y0 * _block_height - y) / _block_height);
        v_tr = cv::Vec2f((x - x1 * _block_width) / _block_width, (y0 * _block_height - y) / _block_height);
        v_bl = cv::Vec2f((x - x0 * _block_width) / _block_width, (y1 * _block_height - y) / _block_height);
        v_br = cv::Vec2f((x - x1 * _block_width) / _block_width, (y1 * _block_height - y) / _block_height);

        if (_smooth) {
            v_tl = smooth(v_tl);
            v_tr = smooth(v_tr);
            v_bl = smooth(v_bl);
            v_br = smooth(v_br);
        }

        v_tl = v_tl / cv::norm(v_tl);
        v_tr = v_tr / cv::norm(v_tr);
        v_bl = v_bl / cv::norm(v_bl);
        v_br = v_br / cv::norm(v_br);

        v_tl = isnan(v_tl[0]) ? 0.0 : v_tl;
        v_tr = isnan(v_tr[0]) ? 0.0 : v_tr;
        v_bl = isnan(v_bl[0]) ? 0.0 : v_bl;
        v_br = isnan(v_br[0]) ? 0.0 : v_br;
    }

    cv::Mat operator()(float t) override {
        cv::Mat output = _init_image.clone();
        int height = output.rows;
        int width = output.cols;

        if (_dynamic)
            update_grid(t);

        // Interpolate
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Get surrounding grid
                int x0 = x / (int)_block_width;
                int y0 = y / (int)_block_height;
                int x1 = x0 + 1;
                int y1 = y0 + 1;

                cv::Vec2f v_tl, v_tr, v_bl, v_br;
                get_cell(v_bl, v_br, v_tl, v_tr, x, x0, x1, y, y0, y1);

                float dot_tl = v_tl.dot(_grid[y0][x0]);
                float dot_tr = v_tr.dot(_grid[y0][x1]);
                float dot_bl = v_bl.dot(_grid[y1][x0]);
                float dot_br = v_br.dot(_grid[y1][x1]);

                float alpha = (x - x0 * _block_width) / _block_width;
                float beta = (y - y0 * _block_height) / _block_height;

                // Interp tl tr
                float top = alpha * dot_tr + (1.0 - alpha) * dot_tl;
                // Interp bl br
                float bottom = alpha * dot_br + (1.0 - alpha) * dot_bl;
                // Interp top bottom
                float value = beta * bottom + (1.0 - beta) * top;

                output.at<float>(y, x) = value / (sqrt(2) / 2);
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
