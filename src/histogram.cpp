#include "histogram.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>

namespace {

int toLevel(float value) {
    const int level = static_cast<int>(std::lround(value));
    return std::clamp(level, 0, kGrayLevels - 1);
}

}  // namespace

Histogram computeHistogram(const Image& img) {
    Histogram hist{};
    for (float v : img.data) ++hist[toLevel(v)];
    return hist;
}

Image equalizeHistogram(const Image& img, LevelMapping* mapping) {
    if (img.empty()) throw std::invalid_argument("Cannot equalize an empty image");

    const Histogram hist = computeHistogram(img);
    const double totalPixels = static_cast<double>(img.data.size());

    // Transformation T(r_k): scaled cumulative distribution of the input levels.
    LevelMapping transform{};
    std::int64_t cumulative = 0;
    for (int k = 0; k < kGrayLevels; ++k) {
        cumulative += hist[k];
        const double cdf = static_cast<double>(cumulative) / totalPixels;
        transform[k] = static_cast<int>(std::lround((kGrayLevels - 1) * cdf));
    }

    Image out(img.width, img.height);
    for (size_t i = 0; i < img.data.size(); ++i)
        out.data[i] = static_cast<float>(transform[toLevel(img.data[i])]);

    if (mapping) *mapping = transform;
    return out;
}

void saveHistogramCsv(const Histogram& hist, const std::string& path) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("Cannot write file: " + path);

    std::int64_t total = 0;
    for (auto count : hist) total += count;

    out << "level,count,cdf\n";
    std::int64_t cumulative = 0;
    for (int k = 0; k < kGrayLevels; ++k) {
        cumulative += hist[k];
        const double cdf = total > 0 ? static_cast<double>(cumulative) / total : 0.0;
        out << k << ',' << hist[k] << ',' << cdf << '\n';
    }
    if (!out) throw std::runtime_error("Error while writing " + path);
}
