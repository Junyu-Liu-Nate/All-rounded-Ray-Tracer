#pragma once

#include "utils/rgba.h"
#include <iostream>

class filter
{
public:
    filter();

    void bilateral2D(RGBA* data, int width, int height, int kernelSize);
    float filterSpatialGaussian(int x, int y, double sigma);
    float filterRangeGaussian(double intensityDifference, double sigma);

    void median2D(RGBA* data, int width, int height, int kernelSize);

    RGBA getPixelRepeated(RGBA* data, int width, int height, int x, int y);
    RGBA getPixelReflected(RGBA* data, int width, int height, int x, int y);
    RGBA getPixelWrapped(RGBA* data, int width, int height, int x, int y);
    std::uint8_t floatToUint8(float x);
};
