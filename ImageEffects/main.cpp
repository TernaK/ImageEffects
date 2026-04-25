#include "ImageEffects.h"
#include <iostream>
#include <chrono>

class NoiseEffect : public ImageEffects {
    float _amplitude;
    float _frequency;

public:
    NoiseEffect(cv::Mat image, float amplitude = 0.2, float f = 1)
        : ImageEffects(image), _amplitude(amplitude), _frequency(f) {}

    cv::Mat operator()(float t) override {
        cv::Mat output = _init_image.clone();

        for (int y = 0; y < output.rows; y++) {
            for (int x = 0; x < output.cols; x++) {
                float d = arc4random() % (output.rows / 2);
                float delta = _amplitude * sin(2.0 * M_PI * _frequency * d * (t - _last_t));
                output.at<float>(y, x) = std::clamp(output.at<float>(y, x) + delta, 0.0f, 1.0f);
            }
        }

        _last_t = t;
        return output;
    }

    static cv::Mat make_image(cv::Size resolution = {360, 360}) {
        cv::Mat image(resolution, CV_32F);
        image = 0.5;
        return image;
    }
};

int main(int argc, const char * argv[]) {
    cv::Mat init_image = NoiseEffect::make_image();
    NoiseEffect ne(init_image, 0.7, 0.01);

    auto start_time = std::chrono::steady_clock::now();

    for (;;) {
        auto current_time = std::chrono::steady_clock::now();
        float t = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count() / 1e3;

        cv::Mat output = ne(t);

        cv::putText(output, std::to_string(t), {20, 20}, cv::FONT_HERSHEY_PLAIN, 2, {255});
        cv::imshow("effect", output);
        if (cv::waitKey(30) == 27)
            break;
    }
}
