#pragma once

#include "ImageEffect.h"

class PerlinNoise {
public:
    PerlinNoise(cv::Size grid_size = {1, 1});

    cv::Mat generate(cv::Size image_size, bool smooth = true);

    std::vector<std::vector<cv::Vec2f>>& grid() { return _grid; }

    size_t height() { return _grid.empty() ? 0 : _grid.size() - 1; }

    size_t width() { return _grid.empty() ? 0 : _grid[1].size() - 1; }

    static cv::Mat make(cv::Size image_size, cv::Size grid_size = {1, 1}, bool smooth = true);

    static cv::Mat make_octaves(int octaves, cv::Size image_size, cv::Size grid_size = {1, 1}, bool smooth = true);

private:
    std::vector<std::vector<cv::Vec2f>> _grid;
};
