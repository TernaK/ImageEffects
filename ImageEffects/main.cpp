#include "NoiseEffect.h"
#include "PerlinNoiseEffect.h"
#include "DisplayRenderer.h"
#include <iostream>
#include <chrono>

int main(int argc, const char * argv[]) {
    auto noise_effect = std::make_shared<PerlinNoiseEffect>(PerlinNoiseEffect::make_image(), 5, 5, true, true);

    DisplayRenderer renderer(noise_effect);
    renderer.loop();
}
