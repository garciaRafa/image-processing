#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "convolution.hpp"
#include "hfe.hpp"
#include "histogram.hpp"
#include "image.hpp"
#include "median.hpp"

namespace {

void printUsage(const char* program) {
    std::cerr
        << "Usage:\n"
        << "  " << program << " info <input>\n"
        << "  " << program << " convert <input> <output.pgm|output.png>\n"
        << "  " << program << " conv <input> <output> <kernel> [args]\n"
        << "      kernels: mean <d> | gaussian <d> <sigma> | laplacian4 | laplacian8 |\n"
        << "               sharpen | file <kernel.txt>\n"
        << "  " << program << " histeq <input> <output> [histogram_prefix]\n"
        << "      writes <prefix>_before.csv and <prefix>_after.csv if a prefix is given\n"
        << "  " << program << " median <input> <output> [d=3] [iterations=1]\n"
        << "  " << program << " hfe <input> <output> <k1> <k2> [d0=40] [gaussian|butterworth] [order=2]\n"
        << "  " << program << " spectrum <input> <output>\n";
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

int runHistEq(const std::vector<std::string>& args) {
    if (args.size() != 2 && args.size() != 3) return -1;
    const Image input = loadImage(args[0]);
    const Image output = equalizeHistogram(input);
    saveImage(output, args[1]);

    const auto describe = [](const char* label, const Image& img) {
        const auto [lo, hi] = std::minmax_element(img.data.begin(), img.data.end());
        const Histogram hist = computeHistogram(img);
        const auto used = std::count_if(hist.begin(), hist.end(), [](auto n) { return n > 0; });
        std::cout << label << ": range [" << *lo << ", " << *hi << "], " << used
                  << " distinct levels\n";
    };
    describe("Input ", input);
    describe("Output", output);

    if (args.size() == 3) {
        saveHistogramCsv(computeHistogram(input), args[2] + "_before.csv");
        saveHistogramCsv(computeHistogram(output), args[2] + "_after.csv");
    }
    return 0;
}

int runMedian(const std::vector<std::string>& args) {
    if (args.size() < 2 || args.size() > 4) return -1;
    const int d = args.size() >= 3 ? std::stoi(args[2]) : 3;
    const int iterations = args.size() == 4 ? std::stoi(args[3]) : 1;
    if (iterations < 1) throw std::invalid_argument("Number of iterations must be at least 1");

    const Image input = loadImage(args[0]);
    Image output = input;
    for (int i = 0; i < iterations; ++i) output = medianFilter(output, d);
    saveImage(output, args[1]);

    std::cout << "Median " << d << "x" << d << " (" << iterations
              << " pass(es)): impulse pixels (0 or 255) " << impulseFraction(input) << "% -> "
              << impulseFraction(output) << "%\n";
    return 0;
}

int runHfe(const std::vector<std::string>& args) {
    if (args.size() < 4 || args.size() > 7) return -1;
    HfeParams params;
    params.k1 = std::stod(args[2]);
    params.k2 = std::stod(args[3]);
    if (args.size() >= 5) params.d0 = std::stod(args[4]);
    if (args.size() >= 6) {
        if (args[5] == "gaussian") params.type = HighPassType::Gaussian;
        else if (args[5] == "butterworth") params.type = HighPassType::Butterworth;
        else throw std::invalid_argument("Unknown high-pass type '" + args[5] + "'");
    }
    if (args.size() == 7) params.order = std::stoi(args[6]);

    const Image input = loadImage(args[0]);
    const Image output = highFrequencyEmphasis(input, params);
    // With k1 = 0 the result is a pure high-pass response, signed and centred
    // on zero, so it is displayed like the Laplacian (zero at mid-grey).
    // Otherwise values outside [0, 255] are clipped.
    if (params.k1 == 0.0) saveImage(normalizeSymmetric(output), args[1]);
    else saveImage(output, args[1]);

    const auto [lo, hi] = std::minmax_element(output.data.begin(), output.data.end());
    const auto clipped = std::count_if(output.data.begin(), output.data.end(),
                                       [](float v) { return v < -0.5f || v > 255.5f; });
    std::cout << "HFE k1=" << params.k1 << " k2=" << params.k2 << " D0=" << params.d0 << ": range ["
              << *lo << ", " << *hi << "], ";
    if (params.k1 == 0.0)
        std::cout << "displayed with zero at mid-grey\n";
    else
        std::cout << 100.0 * static_cast<double>(clipped) / static_cast<double>(output.data.size())
                  << "% of pixels clipped\n";
    return 0;
}

int runSpectrum(const std::vector<std::string>& args) {
    if (args.size() != 2) return -1;
    saveImage(logSpectrum(loadImage(args[0])), args[1]);
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
        else if (command == "histeq") status = runHistEq(args);
        else if (command == "median") status = runMedian(args);
        else if (command == "hfe") status = runHfe(args);
        else if (command == "spectrum") status = runSpectrum(args);

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
