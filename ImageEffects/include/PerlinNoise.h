#pragma once

#include <opencv2/opencv.hpp>

/// @brief Generate Perlin noise vector grids and vector fields
class PerlinNoise {
public:
    /// Create vector grid
    PerlinNoise(cv::Size grid_size = {1, 1});

    /// Generate vector field
    cv::Mat generate(cv::Size image_size, bool smooth = false);

    /// Return the vector grid
    std::vector<std::vector<cv::Vec2f>>& grid() { return _grid; }

    /// Height of vector grid
    size_t height() { return _grid.empty() ? 0 : _grid.size() - 1; }

    /// Width of vector grid
    size_t width() { return _grid.empty() ? 0 : _grid[1].size() - 1; }

    /// Generate vector field
    /// @param image_size Vector field image size
    /// @param grid_size Vector grid size
    static cv::Mat make(cv::Size image_size, cv::Size grid_size = {1, 1}, bool smooth = false);

    /// Generate vector field using stacked vector noise fields
    /// @param octaves Numnber of noise octaves
    /// @param image_size Output vector field image size
    /// @param grid_size Vector grid size for first octave
    static cv::Mat make_octaves(int octaves, cv::Size image_size, cv::Size grid_size = {1, 1}, bool smooth = false);

    /// Render a vector field
    /// @param field Vector field to render
    /// @param image Canvas
    /// @param color Vector field color
    static void draw_field(cv::Mat& field, cv::Mat& image, cv::Scalar color = {1.0, 1.0, 1.0});

private:
    std::vector<std::vector<cv::Vec2f>> _grid;
};
