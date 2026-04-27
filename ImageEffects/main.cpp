#include "PerlinNoiseEffect.h"
#include "PerlinFlowEffect.h"
#include "DisplayRenderer.h"
#include <iostream>
#include <chrono>

int main(int argc, const char * argv[]) {
//    auto ne = std::make_shared<OctavePerlinNoiseEffect>(PerlinNoiseEffect::make_image({400,400}), 4, 0.5, cv::Size(5, 5), false, true);
//    DisplayRenderer renderer(ne);
//    renderer.loop();

    auto ne = std::make_shared<PerlinFlowEffect>(PerlinNoiseEffect::make_image({256, 256}), cv::Size(1, 1), false, true);
    DisplayRenderer renderer(ne);
    renderer.loop();


//    cv::Mat image = PerlinNoise::make_octaves(3, {256, 256}, {16, 16}, false);
//    cv::imshow("image", image / 2.0 + 0.5);
//    cv::waitKey();
}
