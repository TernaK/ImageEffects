#include "NoiseEffect.h"
#include "DisplayRenderer.h"
#include <iostream>
#include <chrono>

int main(int argc, const char * argv[]) {
    float amplitude = 0.3;
    float frequency = 0.1;
    auto noise_effect = std::make_shared<NoiseEffect>(NoiseEffect::make_image(), amplitude, frequency);

    DisplayRenderer renderer(noise_effect);
    renderer.loop();
}
