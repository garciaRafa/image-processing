#include "image.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <stdexcept>

#include "stb_image.h"
#include "stb_image_write.h"

namespace {

std::string lowercaseExtension(const std::string& path) {
    const auto dot = path.find_last_of('.');
    if (dot == std::string::npos) return "";
    std::string ext = path.substr(dot);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext;
}

// Reads the next header token of a PGM file, skipping whitespace and
// comments ('#' until end of line), which may appear anywhere in the header.
std::string nextPgmToken(std::istream& in) {
    std::string token;
    char c;
    while (in.get(c)) {
        if (c == '#') {
            std::string ignored;
            std::getline(in, ignored);
        } else if (std::isspace(static_cast<unsigned char>(c))) {
            if (!token.empty()) break;
        } else {
            token += c;
        }
    }
    if (token.empty()) throw std::runtime_error("Unexpected end of PGM file");
    return token;
}

int nextPgmInt(std::istream& in) {
    const std::string token = nextPgmToken(in);
    try {
        return std::stoi(token);
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid number in PGM file: '" + token + "'");
    }
}

Image loadPgm(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open file: " + path);

    const std::string magic = nextPgmToken(in);
    if (magic != "P2" && magic != "P5")
        throw std::runtime_error("Unsupported PGM type '" + magic + "' in " + path);

    const int width = nextPgmInt(in);
    const int height = nextPgmInt(in);
    const int maxVal = nextPgmInt(in);
    if (width <= 0 || height <= 0 || maxVal <= 0 || maxVal > 65535)
        throw std::runtime_error("Invalid PGM header in " + path);
    // nextPgmToken consumed exactly one whitespace character after maxVal,
    // which is where the binary raster of a P5 file starts.

    Image img(width, height);
    const float scale = 255.0f / static_cast<float>(maxVal);
    const size_t count = img.data.size();

    if (magic == "P2") {
        for (size_t i = 0; i < count; ++i)
            img.data[i] = static_cast<float>(nextPgmInt(in)) * scale;
    } else {
        const int bytesPerSample = (maxVal < 256) ? 1 : 2;
        std::vector<unsigned char> raw(count * bytesPerSample);
        in.read(reinterpret_cast<char*>(raw.data()), static_cast<std::streamsize>(raw.size()));
        if (in.gcount() != static_cast<std::streamsize>(raw.size()))
            throw std::runtime_error("Truncated PGM raster in " + path);
        for (size_t i = 0; i < count; ++i) {
            const int value = (bytesPerSample == 1)
                                  ? raw[i]
                                  : (raw[2 * i] << 8) | raw[2 * i + 1];  // big-endian
            img.data[i] = static_cast<float>(value) * scale;
        }
    }
    return img;
}

Image loadWithStb(const std::string& path) {
    int width, height, channelsInFile;
    // Last argument = 1 forces a single (luminance) channel, so RGB/RGBA
    // inputs are converted to grayscale and the alpha channel is dropped.
    unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channelsInFile, 1);
    if (!pixels)
        throw std::runtime_error("Cannot load " + path + ": " + stbi_failure_reason());

    Image img(width, height);
    for (size_t i = 0; i < img.data.size(); ++i) img.data[i] = pixels[i];
    stbi_image_free(pixels);
    return img;
}

bool hasPgmMagic(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    char magic[2] = {0, 0};
    in.read(magic, 2);
    return in.gcount() == 2 && magic[0] == 'P' && (magic[1] == '2' || magic[1] == '5');
}

std::vector<unsigned char> toBytes(const Image& img) {
    std::vector<unsigned char> bytes(img.data.size());
    for (size_t i = 0; i < img.data.size(); ++i) {
        const float v = std::round(img.data[i]);
        bytes[i] = static_cast<unsigned char>(std::clamp(v, 0.0f, 255.0f));
    }
    return bytes;
}

}  // namespace

Image loadImage(const std::string& path) {
    if (!std::ifstream(path)) throw std::runtime_error("Cannot open file: " + path);
    // Detect PGM by content rather than extension, since stb_image does not read P2.
    return hasPgmMagic(path) ? loadPgm(path) : loadWithStb(path);
}

void saveImage(const Image& img, const std::string& path) {
    if (img.empty()) throw std::runtime_error("Cannot save an empty image to " + path);
    const std::vector<unsigned char> bytes = toBytes(img);
    const std::string ext = lowercaseExtension(path);

    if (ext == ".pgm") {
        std::ofstream out(path, std::ios::binary);
        if (!out) throw std::runtime_error("Cannot write file: " + path);
        out << "P5\n" << img.width << ' ' << img.height << "\n255\n";
        out.write(reinterpret_cast<const char*>(bytes.data()),
                  static_cast<std::streamsize>(bytes.size()));
        if (!out) throw std::runtime_error("Error while writing " + path);
    } else if (ext == ".png") {
        if (!stbi_write_png(path.c_str(), img.width, img.height, 1, bytes.data(), img.width))
            throw std::runtime_error("Cannot write PNG file: " + path);
    } else {
        throw std::runtime_error("Unsupported output extension '" + ext + "' (use .pgm or .png)");
    }
}

Image normalizeToByteRange(const Image& img) {
    Image out(img.width, img.height);
    if (img.empty()) return out;
    const auto [minIt, maxIt] = std::minmax_element(img.data.begin(), img.data.end());
    const float lo = *minIt, range = *maxIt - *minIt;
    for (size_t i = 0; i < img.data.size(); ++i)
        out.data[i] = (range > 0.0f) ? (img.data[i] - lo) * 255.0f / range : 0.0f;
    return out;
}

Image normalizeSymmetric(const Image& img, float percentile) {
    Image out(img.width, img.height, 128.0f);
    if (img.empty()) return out;
    std::vector<float> magnitudes(img.data.size());
    for (size_t i = 0; i < img.data.size(); ++i) magnitudes[i] = std::fabs(img.data[i]);
    const float fraction = std::clamp(percentile, 0.0f, 100.0f) / 100.0f;
    const size_t k = static_cast<size_t>(fraction * static_cast<float>(magnitudes.size() - 1));
    std::nth_element(magnitudes.begin(), magnitudes.begin() + static_cast<std::ptrdiff_t>(k),
                     magnitudes.end());
    const float limit = magnitudes[k];
    if (limit <= 0.0f) return out;
    for (size_t i = 0; i < img.data.size(); ++i) {
        const float v = std::clamp(img.data[i] / limit, -1.0f, 1.0f);
        out.data[i] = 128.0f + 127.0f * v;
    }
    return out;
}
