#include "PerlinNoise.h"

PerlinNoise::PerlinNoise(cv::Size grid_size) {
    size_t gheight = grid_size.height;
    size_t gwidth = grid_size.width;
    _grid = std::vector<std::vector<cv::Vec2f>>(gheight + 1, std::vector<cv::Vec2f>(gwidth + 1));
    for (size_t gy = 0; gy < gheight + 1; gy++) {
        for (size_t gx = 0; gx < gwidth + 1; gx++) {
            float angle = (arc4random() % 360) / 360.0f * 2.0 * M_PI;
            cv::Vec2f vec(cos(angle), sin(angle));
            _grid[gy][gx] = vec;
        }
    }
}

cv::Mat PerlinNoise::generate(cv::Size image_size, bool smooth) {
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
    for (int y = 0; y < iheight; y++) {
        for (int x = 0; x < iwidth; x++) {
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

            image.at<float>(y, x) = value;
        }
    }

    return image;
}

cv::Mat PerlinNoise::make(cv::Size image_size, cv::Size grid_size, bool smooth) {
    PerlinNoise pn(grid_size);
    return pn.generate(image_size, smooth);
}

cv::Mat PerlinNoise::make_octaves(int octaves, cv::Size image_size, cv::Size grid_size, bool smooth) {
    int gheight = grid_size.height;
    int gwidth = grid_size.width;

    cv::Mat output = make(image_size, grid_size, smooth);
    float amplitude = 1.0;
    for (int i = 0; i < octaves - 1; i++) {
        amplitude *= 0.5;
        gheight *= 2;
        gwidth *= 2;
        output += amplitude * make(image_size, {gwidth * 2, gheight * 2}, smooth);
    }
    return output;
}
