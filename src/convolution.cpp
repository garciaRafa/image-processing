#include "convolution.hpp"

#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>

Image convolve(const Image& img, const std::vector<float>& kernel, int d) {
    if (d <= 0 || d % 2 == 0)
        throw std::invalid_argument("Kernel size must be a positive odd number, got " +
                                    std::to_string(d));
    if (kernel.size() != static_cast<size_t>(d) * d)
        throw std::invalid_argument("Kernel of size " + std::to_string(d) + " needs " +
                                    std::to_string(d * d) + " weights, got " +
                                    std::to_string(kernel.size()));
    if (img.width < d || img.height < d)
        throw std::invalid_argument("Image is smaller than the kernel");

    const int r = d / 2;
    Image out(img.width - 2 * r, img.height - 2 * r);

    // (row, col) are coordinates in the input image; only positions where the
    // whole kernel fits are visited, which skips the first and last r rows/columns.
    for (int row = r; row < img.height - r; ++row) {
        for (int col = r; col < img.width - r; ++col) {
            float sum = 0.0f;
            for (int s = -r; s <= r; ++s) {
                for (int t = -r; t <= r; ++t) {
                    const float w = kernel[static_cast<size_t>(s + r) * d + (t + r)];
                    sum += w * img.at(row - s, col - t);  // flipped kernel
                }
            }
            out.at(row - r, col - r) = sum;
        }
    }
    return out;
}

Image convolve(const Image& img, const Kernel& kernel) {
    return convolve(img, kernel.weights, kernel.size);
}

Kernel meanKernel(int d) {
    if (d <= 0 || d % 2 == 0) throw std::invalid_argument("Mean kernel size must be odd");
    return {d, std::vector<float>(static_cast<size_t>(d) * d, 1.0f / static_cast<float>(d * d))};
}

Kernel gaussianKernel(int d, float sigma) {
    if (d <= 0 || d % 2 == 0) throw std::invalid_argument("Gaussian kernel size must be odd");
    if (sigma <= 0.0f) throw std::invalid_argument("Gaussian sigma must be positive");
    const int r = d / 2;
    Kernel k{d, std::vector<float>(static_cast<size_t>(d) * d)};
    float total = 0.0f;
    for (int s = -r; s <= r; ++s) {
        for (int t = -r; t <= r; ++t) {
            const float w = std::exp(-static_cast<float>(s * s + t * t) / (2.0f * sigma * sigma));
            k.weights[static_cast<size_t>(s + r) * d + (t + r)] = w;
            total += w;
        }
    }
    for (float& w : k.weights) w /= total;  // weights sum to 1: mean intensity is preserved
    return k;
}

Kernel laplacianKernel4() {
    return {3, {0, -1, 0,
                -1, 4, -1,
                0, -1, 0}};
}

Kernel laplacianKernel8() {
    return {3, {-1, -1, -1,
                -1, 8, -1,
                -1, -1, -1}};
}

Kernel sharpenKernel() {
    return {3, {0, -1, 0,
                -1, 5, -1,
                0, -1, 0}};
}

Kernel loadKernel(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open kernel file: " + path);

    std::stringstream content;
    std::string line;
    while (std::getline(in, line)) {
        const auto hash = line.find('#');
        content << line.substr(0, hash) << '\n';
    }

    Kernel k;
    if (!(content >> k.size)) throw std::runtime_error("Missing kernel size in " + path);
    float w;
    while (content >> w) k.weights.push_back(w);
    if (!content.eof()) throw std::runtime_error("Invalid weight in kernel file " + path);
    if (k.weights.size() != static_cast<size_t>(k.size) * k.size)
        throw std::runtime_error("Kernel file " + path + " declares size " +
                                 std::to_string(k.size) + " but has " +
                                 std::to_string(k.weights.size()) + " weights");
    return k;
}
