#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "image.hpp"

constexpr int kGrayLevels = 256;  // L
using Histogram = std::array<std::int64_t, kGrayLevels>;
using LevelMapping = std::array<int, kGrayLevels>;

// Number of pixels at each grey level r_k, k = 0, ..., L-1. Pixel values are
// rounded and clamped to [0, L-1].
Histogram computeHistogram(const Image& img);

// Histogram equalization, implemented from scratch:
//
//   s_k = T(r_k) = round( (L - 1) * sum_{j=0}^{k} p_r(r_j) ),  p_r(r_j) = n_j / (M N)
//
// i.e. each level is mapped through the scaled cumulative distribution
// function (CDF) of the input. If `mapping` is not null, the transformation
// T is stored in it.
Image equalizeHistogram(const Image& img, LevelMapping* mapping = nullptr);

// Writes "level,count,normalized_cdf" lines (with a header) for plotting.
void saveHistogramCsv(const Histogram& hist, const std::string& path);
