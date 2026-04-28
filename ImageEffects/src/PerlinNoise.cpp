#include "PerlinNoise.h"
#include "Random.h"

PerlinNoise::PerlinNoise(cv::Size grid_size) {
    if (grid_size.width < 1 || grid_size.height < 1)
        throw std::runtime_error("grid_size width or height cannot be < 1");

    size_t gheight = grid_size.height;
    size_t gwidth = grid_size.width;
    _grid = std::vector<std::vector<cv::Vec2f>>(gheight + 1, std::vector<cv::Vec2f>(gwidth + 1));
    for (size_t gy = 0; gy < gheight + 1; gy++) {
        for (size_t gx = 0; gx < gwidth + 1; gx++) {
            _grid[gy][gx] = Random::vector2d();
        }
    }
}

cv::Mat PerlinNoise::generate(cv::Size image_size, bool smooth) {
    if (image_size.width < width() || image_size.height < height())
        throw std::runtime_error("image_size width or height cannot be < grid size");
    size_t iheight = image_size.height;
    size_t iwidth = image_size.width;
    size_t gheight = height();
    size_t gwidth = width();

    float block_width = iwidth / gwidth;
    float block_height = iheight / gheight;

    auto smooth_func = [](cv::Vec2f vec) -> cv::Vec2f {
        float x = vec[0];
        float y = vec[1];
        x = 6 * pow(x, 5) - 15 * pow(x, 4) + 10 * pow(x, 3);
        y = 6 * pow(y, 5) - 15 * pow(y, 4) + 10 * pow(y, 3);
        return {x, y};
    };

    // Generate image
    cv::Mat image(image_size, CV_32F);

    // Interpolate
    image.forEach<float>([&](float& pixel, const int* pos) {
        int y = pos[0];
        int x = pos[1];
        // Get surrounding grid
        int x0 = x / (int)block_width;
        int y0 = y / (int)block_height;
        int x1 = x0 + 1;
        int y1 = y0 + 1;

        auto v_tl = cv::Vec2f((x - x0 * block_width) / block_width, (y0 * block_height - y) / block_height);
        auto v_tr = cv::Vec2f((x - x1 * block_width) / block_width, (y0 * block_height - y) / block_height);
        auto v_bl = cv::Vec2f((x - x0 * block_width) / block_width, (y1 * block_height - y) / block_height);
        auto v_br = cv::Vec2f((x - x1 * block_width) / block_width, (y1 * block_height - y) / block_height);

        if (smooth) {
            v_tl = smooth_func(v_tl);
            v_tr = smooth_func(v_tr);
            v_bl = smooth_func(v_bl);
            v_br = smooth_func(v_br);
        }

        v_tl = v_tl / cv::norm(v_tl);
        v_tr = v_tr / cv::norm(v_tr);
        v_bl = v_bl / cv::norm(v_bl);
        v_br = v_br / cv::norm(v_br);

        v_tl = isnan(v_tl[0]) ? 0.0 : v_tl;
        v_tr = isnan(v_tr[0]) ? 0.0 : v_tr;
        v_bl = isnan(v_bl[0]) ? 0.0 : v_bl;
        v_br = isnan(v_br[0]) ? 0.0 : v_br;

        float dot_tl = v_tl.dot(_grid[y0][x0]);
        float dot_tr = v_tr.dot(_grid[y0][x1]);
        float dot_bl = v_bl.dot(_grid[y1][x0]);
        float dot_br = v_br.dot(_grid[y1][x1]);

        float alpha = (x - x0 * block_width) / block_width;
        float beta = (y - y0 * block_height) / block_height;

        // Interp tl tr
        float top = alpha * dot_tr + (1.0 - alpha) * dot_tl;
        // Interp bl br
        float bottom = alpha * dot_br + (1.0 - alpha) * dot_bl;
        // Interp top bottom
        float value = beta * bottom + (1.0 - beta) * top;

        pixel = value;
    });

    return image;
}

cv::Mat PerlinNoise::make(cv::Size image_size, cv::Size grid_size, bool smooth) {
    PerlinNoise pn(grid_size);
    return pn.generate(image_size, smooth);
}

cv::Mat PerlinNoise::make_octaves(int octaves, cv::Size image_size, cv::Size grid_size, bool smooth) {
    std::vector<cv::Mat> images(octaves);
    cv::parallel_for_(cv::Range(0, octaves), [&](const cv::Range& range) {
        for (int i = range.start; i < range.end; i++) {
            int power = pow(2, i);
            cv::Size octage_grid_size = grid_size * power;
            images[i] = make(image_size, octage_grid_size, smooth);
        }
    });

    float amplitude = 1.0;
    cv::Mat output = images[0];
    for (int i = 1; i < octaves; i++) {
        amplitude *= 0.5;
        output += amplitude * images[i];
    }

    return output;
}

void PerlinNoise::draw_field(cv::Mat& field, cv::Mat &image, cv::Scalar color) {
    if (field.size() != image.size())
        throw std::runtime_error("field and output image must be the same size");
    
    field.forEach<float>([&](float& angle, const int* pos) {
        if (pos[0] % 15 == 0 && pos[1] % 15 == 0) {
            cv::Point2f vec(cos(angle), sin(angle));
            float mag = 8;
            cv::Point2f start = mag * vec;
            cv::Point2f end = -mag * vec;
            cv::Point px(pos[1], pos[0]);
            cv::line(image, px + cv::Point(start), px + cv::Point(end), color);
        }
    });
}
