#include "DisplayRenderer.h"

void DisplayRenderer::loop() {
    _start_time = std::chrono::steady_clock::now();

    for (;;) {
        auto current_time = std::chrono::steady_clock::now();
        float t = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - _start_time).count() / 1e3;

        cv::Mat output = _ie->operator()(t);
        if (output.channels() == 1)
            cv::cvtColor(output, output, cv::COLOR_GRAY2BGR);

        char text[256];
        std::snprintf(text, 255, "t = %.3fs", t);
        cv::putText(output, text, {20, 20}, cv::FONT_HERSHEY_PLAIN, 1, {0, 255, 0}, 2);

        cv::imshow("effect", output);
        if (cv::waitKey(30) == 27)
            break;
    }
}
