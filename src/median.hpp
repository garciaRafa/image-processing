#pragma once

#include "image.hpp"

// Median filter, implemented from scratch: each output pixel is the median of
// the d x d neighbourhood centred on it (d odd). Near the borders, the image
// is extended by replicating its edge pixels, so the output has the same size
// as the input. Throws std::invalid_argument if d is not a positive odd number.
Image medianFilter(const Image& img, int d = 3);

// Fraction (in percent) of pixels equal to 0 or 255, the two values produced
// by salt-and-pepper noise. Used as a simple measure of remaining impulses.
double impulseFraction(const Image& img);
