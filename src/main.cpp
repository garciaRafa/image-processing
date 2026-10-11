#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "convolution.hpp"
#include "image.hpp"

namespace {

void printUsage(const char* program) {
    std::cerr
        << "Usage:\n"
        << "  " << program << " info <input>\n"
        << "  " << program << " convert <input> <output.pgm|output.png>\n"
        << "  " << program << " conv <input> <output> <kernel> [args]\n"
        << "      kernels: mean <d> | gaussian <d> <sigma> | laplacian4 | laplacian8 |\n"
        << "               sharpen | file <kernel.txt>\n";
}

// Saves a filter result. Kernels whose weights sum to zero (pure high-pass,
// e.g. Laplacian) produce signed outputs centered on zero, so they are
// displayed with zero at mid-grey and scaled by the 99th percentile of the
// absolute response. Kernels whose weights sum to one
// (low-pass, sharpening) preserve the intensity range, so out-of-range values
// are simply clipped when saving.
void saveResult(const Image& img, const Kernel& kernel, const std::string& path) {
    float weightSum = 0.0f;
    for (float w : kernel.weights) weightSum += w;
    const auto [lo, hi] = std::minmax_element(img.data.begin(), img.data.end());
    if (std::abs(weightSum) < 1e-6f) {
        std::cout << "Signed output in [" << *lo << ", " << *hi
                  << "], displayed with zero at mid-grey (99th percentile scaling)\n";
        saveImage(normalizeSymmetric(img), path);
    } else {
        saveImage(img, path);
    }
}

int runInfo(const std::vector<std::string>& args) {
    if (args.size() != 1) return -1;
    const Image img = loadImage(args[0]);
    const auto [lo, hi] = std::minmax_element(img.data.begin(), img.data.end());
    double sum = 0.0;
    for (float v : img.data) sum += v;
    std::cout << args[0] << ": " << img.width << " x " << img.height << ", min " << *lo
              << ", max " << *hi << ", mean " << sum / static_cast<double>(img.data.size())
              << '\n';
    return 0;
}

int runConvert(const std::vector<std::string>& args) {
    if (args.size() != 2) return -1;
    saveImage(loadImage(args[0]), args[1]);
    return 0;
}

int runConv(const std::vector<std::string>& args) {
    if (args.size() < 3) return -1;
    const std::string& name = args[2];
    const size_t extra = args.size() - 3;

    Kernel kernel;
    if (name == "mean" && extra == 1) kernel = meanKernel(std::stoi(args[3]));
    else if (name == "gaussian" && extra == 2)
        kernel = gaussianKernel(std::stoi(args[3]), std::stof(args[4]));
    else if (name == "laplacian4" && extra == 0) kernel = laplacianKernel4();
    else if (name == "laplacian8" && extra == 0) kernel = laplacianKernel8();
    else if (name == "sharpen" && extra == 0) kernel = sharpenKernel();
    else if (name == "file" && extra == 1) kernel = loadKernel(args[3]);
    else return -1;

    const Image input = loadImage(args[0]);
    const Image output = convolve(input, kernel);
    std::cout << "Kernel " << kernel.size << "x" << kernel.size << ": " << input.width << "x"
              << input.height << " -> " << output.width << "x" << output.height << '\n';
    saveResult(output, kernel, args[1]);
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    const std::string command = argv[1];
    const std::vector<std::string> args(argv + 2, argv + argc);

    try {
        int status = -1;
        if (command == "info") status = runInfo(args);
        else if (command == "convert") status = runConvert(args);
        else if (command == "conv") status = runConv(args);

        if (status == -1) {
            printUsage(argv[0]);
            return 1;
        }
        return status;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
