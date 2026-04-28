#include "Random.h"
#import <XCTest/XCTest.h>

@interface RandomTests : XCTestCase

@end

@implementation RandomTests

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testRandomNumber {
    float number = Random::number();
    XCTAssertTrue(number >= 0.0 && number <= 1.0);
}

- (void)testRandomVector2d {
    auto vector = Random::vector2d();
    XCTAssertEqualWithAccuracy(cv::norm(vector), 1.0, 1e-7);

    auto vector2 = Random::vector2d();
    XCTAssertFalse(vector == vector2);
}

- (void)testRandomAngle {
    for (int i = 0; i < 10; i++) {
        float angle = Random::angle();
        XCTAssertTrue(angle >= 0.0 && angle <= (2.0 * M_PI));
    }
}

@end
