#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "image.hpp"

namespace {

void printUsage(const char* program) {
    std::cerr << "Usage:\n"
              << "  " << program << " info <input>\n"
              << "  " << program << " convert <input> <output.pgm|output.png>\n";
}

int runInfo(const std::vector<std::string>& args) {
    if (args.size() != 1) return -1;
    const Image img = loadImage(args[0]);
    float lo = img.data[0], hi = img.data[0];
    double sum = 0.0;
    for (float v : img.data) {
        lo = std::min(lo, v);
        hi = std::max(hi, v);
        sum += v;
    }
    std::cout << args[0] << ": " << img.width << " x " << img.height
              << ", min " << lo << ", max " << hi
              << ", mean " << sum / static_cast<double>(img.data.size()) << '\n';
    return 0;
}

int runConvert(const std::vector<std::string>& args) {
    if (args.size() != 2) return -1;
    saveImage(loadImage(args[0]), args[1]);
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
