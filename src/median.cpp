#include "median.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

Image medianFilter(const Image& img, int d) {
    if (d <= 0 || d % 2 == 0)
        throw std::invalid_argument("Median window size must be a positive odd number, got " +
                                    std::to_string(d));
    if (img.empty()) throw std::invalid_argument("Cannot filter an empty image");

    const int r = d / 2;
    const size_t middle = static_cast<size_t>(d) * d / 2;  // index of the median in d*d values
    Image out(img.width, img.height);
    std::vector<float> window(static_cast<size_t>(d) * d);

    for (int row = 0; row < img.height; ++row) {
        for (int col = 0; col < img.width; ++col) {
            size_t n = 0;
            for (int s = -r; s <= r; ++s) {
                const int y = std::clamp(row + s, 0, img.height - 1);  // replicate border
                for (int t = -r; t <= r; ++t) {
                    const int x = std::clamp(col + t, 0, img.width - 1);
                    window[n++] = img.at(y, x);
                }
            }
            // Partial sort: places the median at `middle` in O(d^2) on average,
            // instead of sorting the whole window in O(d^2 log d^2).
            std::nth_element(window.begin(), window.begin() + static_cast<std::ptrdiff_t>(middle),
                             window.end());
            out.at(row, col) = window[middle];
        }
    }
    return out;
}

double impulseFraction(const Image& img) {
    if (img.empty()) return 0.0;
    size_t count = 0;
    for (float v : img.data) {
        const long level = std::lround(v);
        if (level <= 0 || level >= 255) ++count;
    }
    return 100.0 * static_cast<double>(count) / static_cast<double>(img.data.size());
}
