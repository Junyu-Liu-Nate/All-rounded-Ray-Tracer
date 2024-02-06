#include "filter.h"

filter::filter()
{}

// Bilateral filter
void filter::bilateral2D(RGBA* data, int width, int height, int kernelSize) {
    std::vector<RGBA> result;
    result.assign(width * height, RGBA{0, 0, 0, 255});

    int dimension = int(std::sqrt(kernelSize));

    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            size_t centerIndex = r * width + c;

            RGBA centerPixel = data[r * width + c];

            double weightSum = 0;

            float redAcc = 0.0;
            float greenAcc = 0.0;
            float blueAcc = 0.0;

            int startX = c - dimension / 2;
            int startY = r - dimension / 2;

            for (int rFilter = 0; rFilter < dimension; rFilter++) {
                for (int cFilter = 0; cFilter < dimension; cFilter++) {
                    RGBA pixel = getPixelWrapped(data, width, height, startX + cFilter, startY + rFilter);

                    int xDifference = startX + cFilter - c;
                    int yDifference = startY + rFilter - r;
                    double spatialGaussianWeight = filterSpatialGaussian(xDifference, yDifference, 3.0);

                    int rDifference = pixel.r - centerPixel.r;
                    int gDifference = pixel.g - centerPixel.g;
                    int bDifference = pixel.b - centerPixel.b;
                    double intensityDifference = sqrt(rDifference * rDifference + gDifference * gDifference + bDifference * bDifference);
                    double intensityGaussianWeight = filterRangeGaussian(intensityDifference, 30.0);

                    double weight = spatialGaussianWeight * intensityGaussianWeight;
                    weightSum += weight;

                    redAcc += pixel.r * weight;
                    greenAcc += pixel.g * weight;
                    blueAcc += pixel.b * weight;
                }
            }

            redAcc = redAcc / weightSum;
            greenAcc = greenAcc / weightSum;
            blueAcc = blueAcc / weightSum;

            RGBA filteredColor = {floatToUint8(redAcc / 255.0), floatToUint8(greenAcc / 255.0), floatToUint8(blueAcc / 255.0)};
            result.at(centerIndex) = filteredColor;
        }
    }

    for (int i = 0; i < width * height; i++) {
        data[i] = result[i];
    }
}

float filter::filterSpatialGaussian(int x, int y, double sigma) {
    float weight = exp(-((x * x + y * y) / (2 * sigma * sigma)));
    return weight;
}

float filter::filterRangeGaussian(double intensityDifference, double sigma) {
    float weight = exp(-(intensityDifference * intensityDifference) / (2 * sigma * sigma));
    return weight;
}

// Median filter
void filter::median2D(RGBA* data, int width, int height, int kernelSize) {
    std::vector<RGBA> result;
    result.assign(width * height, RGBA{0, 0, 0, 255});

    int dimension = int(std::sqrt(kernelSize));

    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            size_t centerIndex = r * width + c;

            std::vector<int> rValues;
            rValues.assign(kernelSize, 0);
            std::vector<int> gValues;
            gValues.assign(kernelSize, 0);
            std::vector<int> bValues;
            bValues.assign(kernelSize, 0);

            int startX = c - dimension / 2 - 1;
            int startY = r - dimension / 2 - 1;

            for (int rFilter = dimension - 1; rFilter >= 0; rFilter--) {
                for (int cFilter = dimension - 1; cFilter >= 0; cFilter--) {
                    RGBA pixel = getPixelWrapped(data, width, height, startX + dimension - cFilter, startY + dimension - rFilter);
                    rValues.at(cFilter + rFilter * dimension) = pixel.r;
                    gValues.at(cFilter + rFilter * dimension) = pixel.g;
                    bValues.at(cFilter + rFilter * dimension) = pixel.b;
                }
            }

            std::sort(rValues.begin(), rValues.end());
            std::sort(gValues.begin(), gValues.end());
            std::sort(bValues.begin(), bValues.end());
            int rMedian = rValues.at(kernelSize / 2);
            int gMedian = gValues.at(kernelSize / 2);
            int bMedian = bValues.at(kernelSize / 2);

            RGBA filteredColor = {floatToUint8(rMedian / 255.0), floatToUint8(gMedian / 255.0), floatToUint8(bMedian / 255.0)};
            result.at(centerIndex) = filteredColor;
        }
    }

    for (int i = 0; i < width * height; i++) {
        data[i] = result[i];
    }
}

/******************************** Helper functions ********************************/
// Repeats the pixel on the edge of the image such that A,B,C,D looks like ...A,A,A,B,C,D,D,D...
RGBA filter::getPixelRepeated(RGBA* data, int width, int height, int x, int y) {
    int newX = (x < 0) ? 0 : std::min(x, width  - 1);
    int newY = (y < 0) ? 0 : std::min(y, height - 1);
    return data[width * newY + newX];
}

// Flips the edge of the image such that A,B,C,D looks like ...C,B,A,B,C,D,C,B...
RGBA filter::getPixelReflected(RGBA* data, int width, int height, int x, int y) {
    int newX = (x < 0) ? -x : std::min(x, 2*width  - x);
    int newY = (y < 0) ? -y : std::min(y, 2*height  - y);
    return data[width * newY + newX];
}

// Wraps the image such that A,B,C,D looks like ...C,D,A,B,C,D,A,B...
RGBA filter::getPixelWrapped(RGBA* data, int width, int height, int x, int y) {
    int newX = (x < 0) ? x + width  : x % width;
    int newY = (y < 0) ? y + height : y % height;
    return data[width * newY + newX];
}

std::uint8_t filter::floatToUint8(float x) {
    return round(x * 255.f);
}

