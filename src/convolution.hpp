#pragma once

#include <string>
#include <vector>

#include "image.hpp"

// Square kernel of odd size d, stored in row-major order (d * d weights).
struct Kernel {
    int size = 0;
    std::vector<float> weights;
};

// 2-D spatial convolution, implemented from scratch:
//
//   g(x, y) = sum_{s=-r}^{r} sum_{t=-r}^{r} w(s, t) f(x - s, y - t),  r = floor(d / 2)
//
// The kernel is flipped (true convolution, not correlation). Pixels closer
// than r to the border, where the kernel does not fit entirely inside the
// image, are ignored: the output has size (width - 2r) x (height - 2r).
// Throws std::invalid_argument if d is not odd, if the number of weights is
// not d * d, or if the image is smaller than the kernel.
Image convolve(const Image& img, const std::vector<float>& kernel, int d);
Image convolve(const Image& img, const Kernel& kernel);

// Predefined kernels.
Kernel meanKernel(int d);                    // low-pass: box filter, weights 1/d^2
Kernel gaussianKernel(int d, float sigma);   // low-pass: normalized Gaussian
Kernel laplacianKernel4();                   // high-pass: [0 -1 0; -1 4 -1; 0 -1 0]
Kernel laplacianKernel8();                   // high-pass: [-1 -1 -1; -1 8 -1; -1 -1 -1]
Kernel sharpenKernel();                      // identity + 4-neighbour Laplacian

// Reads a kernel from a text file: the size d followed by d * d weights,
// separated by whitespace. Lines starting with '#' are comments.
Kernel loadKernel(const std::string& path);
