#include "PerlinNoise.h"
#import <XCTest/XCTest.h>

@interface PerlinNoiseTests : XCTestCase

@end

@implementation PerlinNoiseTests

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testPerlinNoise {
    cv::Size grid_size{2, 4};
    PerlinNoise pn(grid_size);

    XCTAssertEqual(pn.height(), grid_size.height);
    XCTAssertEqual(pn.width(), grid_size.width);

    cv::Size image_size{10, 12};
    cv::Mat noise = pn.generate(image_size);

    XCTAssertEqual(noise.rows, image_size.height);
    XCTAssertEqual(noise.cols, image_size.width);
    noise.forEach<float>([](float& value, const int* pos) {
        XCTAssertTrue(std::fabs(value) <= 1);
    });
}

- (void)testPerlinNoiseMake {
    cv::Size image_size{10, 12};
    cv::Size grid_size{2, 4};
    cv::Mat noise = PerlinNoise::make(image_size, grid_size);

    XCTAssertEqual(noise.rows, image_size.height);
    XCTAssertEqual(noise.cols, image_size.width);
    noise.forEach<float>([](float& value, const int* pos) {
        XCTAssertTrue(std::fabs(value) <= 1);
    });
}

- (void)testPerlinNoiseMakeOctaves {
    int octaves = 4;
    cv::Mat noise = PerlinNoise::make_octaves(octaves, {512, 512}, {2, 2});

    float amplitude = 1.0;
    float sum = amplitude;
    for (int i = 1; i < octaves; i++) {
        sum += 0.5 * amplitude;
    }

    noise.forEach<float>([&](float& value, const int* pos) {
        XCTAssertTrue(std::fabs(sum));
    });
}

@end
