#pragma once

#include <string>
#include <vector>

// Single-channel (grayscale) image stored in row-major order.
// Pixels are kept as float so that intermediate results (negative values
// from high-pass kernels, FFT output, etc.) are not clipped. Conversion to
// 8 bits happens only when saving.
struct Image {
    int width = 0;
    int height = 0;
    std::vector<float> data;

    Image() = default;
    Image(int w, int h, float value = 0.0f)
        : width(w), height(h), data(static_cast<size_t>(w) * h, value) {}

    bool empty() const { return data.empty(); }

    float& at(int row, int col) { return data[static_cast<size_t>(row) * width + col]; }
    float at(int row, int col) const { return data[static_cast<size_t>(row) * width + col]; }
};

// Loads an image as grayscale. PGM files (P2 and P5) are parsed by our own
// reader; any other format (PNG, JPG, BMP, ...) is decoded with stb_image,
// which converts color images to luminance. Throws std::runtime_error on failure.
Image loadImage(const std::string& path);

// Saves an image, choosing the format from the file extension:
// ".pgm" writes binary PGM (P5), ".png" writes PNG. Values are rounded and
// clamped to [0, 255]. Throws std::runtime_error on failure.
void saveImage(const Image& img, const std::string& path);

// Linearly maps [min, max] of the image to [0, 255]. Useful for displaying
// results that fall outside the 8-bit range, such as high-pass outputs.
Image normalizeToByteRange(const Image& img);

// Displays a signed response (e.g. Laplacian) with zero mapped to mid-grey
// (128). Values are divided by the given percentile of |value| and clipped to
// [-1, 1], so a few extreme pixels do not compress the typical edge response
// into a narrow band of greys, as plain min-max scaling would.
Image normalizeSymmetric(const Image& img, float percentile = 99.0f);
