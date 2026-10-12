#include "hfe.hpp"

#include <fftw3.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>

namespace {

// RAII wrappers so FFTW buffers and plans are always released.
struct FftwBuffer {
    explicit FftwBuffer(size_t n) : data(fftw_alloc_complex(n)), size(n) {
        if (!data) throw std::bad_alloc();
    }
    ~FftwBuffer() { fftw_free(data); }
    FftwBuffer(const FftwBuffer&) = delete;
    FftwBuffer& operator=(const FftwBuffer&) = delete;

    fftw_complex* data;
    size_t size;
};

struct FftwPlan {
    explicit FftwPlan(fftw_plan p) : plan(p) {
        if (!plan) throw std::runtime_error("FFTW could not create a plan");
    }
    ~FftwPlan() { fftw_destroy_plan(plan); }
    FftwPlan(const FftwPlan&) = delete;
    FftwPlan& operator=(const FftwPlan&) = delete;
    void execute() const { fftw_execute(plan); }

    fftw_plan plan;
};

inline double centeringSign(int x, int y) { return ((x + y) % 2 == 0) ? 1.0 : -1.0; }

// Copies the image into a P x Q complex buffer, replicating border pixels
// into the padded region, and multiplies by (-1)^(x+y) to centre the spectrum.
void loadPaddedCentered(const Image& img, int P, int Q, fftw_complex* buffer) {
    for (int x = 0; x < P; ++x) {        // x: row index
        const int srcRow = std::min(x, img.height - 1);
        for (int y = 0; y < Q; ++y) {    // y: column index
            const int srcCol = std::min(y, img.width - 1);
            const size_t i = static_cast<size_t>(x) * Q + y;
            buffer[i][0] = img.at(srcRow, srcCol) * centeringSign(x, y);
            buffer[i][1] = 0.0;
        }
    }
}

double highPass(const HfeParams& p, double distance) {
    if (p.type == HighPassType::Gaussian)
        return 1.0 - std::exp(-(distance * distance) / (2.0 * p.d0 * p.d0));
    if (distance == 0.0) return 0.0;  // limit of the Butterworth high-pass at D = 0
    return 1.0 / (1.0 + std::pow(p.d0 / distance, 2.0 * p.order));
}

}  // namespace

Image highFrequencyEmphasis(const Image& img, const HfeParams& params) {
    if (img.empty()) throw std::invalid_argument("Cannot filter an empty image");
    if (params.k1 < 0.0) throw std::invalid_argument("k1 must be >= 0");
    if (params.k2 <= 0.0) throw std::invalid_argument("k2 must be > 0");
    if (params.d0 <= 0.0) throw std::invalid_argument("D0 must be > 0");
    if (params.order < 1) throw std::invalid_argument("Butterworth order must be >= 1");

    const int M = img.height, N = img.width;
    const int P = 2 * M, Q = 2 * N;  // even, so (P/2, Q/2) is an exact frequency index
    FftwBuffer buffer(static_cast<size_t>(P) * Q);

    // Plans are created before filling the buffer: FFTW_ESTIMATE does not
    // overwrite the data, but planning flags like FFTW_MEASURE would.
    FftwPlan forward(fftw_plan_dft_2d(P, Q, buffer.data, buffer.data, FFTW_FORWARD, FFTW_ESTIMATE));
    FftwPlan inverse(fftw_plan_dft_2d(P, Q, buffer.data, buffer.data, FFTW_BACKWARD, FFTW_ESTIMATE));

    loadPaddedCentered(img, P, Q, buffer.data);
    forward.execute();

    // Transfer function H(u, v) = k1 + k2 H_HP(u, v), real and centred at (P/2, Q/2).
    for (int u = 0; u < P; ++u) {
        for (int v = 0; v < Q; ++v) {
            const double du = u - P / 2.0, dv = v - Q / 2.0;
            const double H = params.k1 + params.k2 * highPass(params, std::sqrt(du * du + dv * dv));
            const size_t i = static_cast<size_t>(u) * Q + v;
            buffer.data[i][0] *= H;
            buffer.data[i][1] *= H;
        }
    }

    inverse.execute();

    // FFTW's inverse is unnormalized: divide by P*Q. Undo the centring and
    // keep the real part of the top-left M x N region (the original image).
    const double scale = 1.0 / (static_cast<double>(P) * Q);
    Image out(N, M);
    for (int x = 0; x < M; ++x) {
        for (int y = 0; y < N; ++y) {
            const size_t i = static_cast<size_t>(x) * Q + y;
            out.at(x, y) = static_cast<float>(buffer.data[i][0] * scale * centeringSign(x, y));
        }
    }
    return out;
}

Image logSpectrum(const Image& img) {
    if (img.empty()) throw std::invalid_argument("Cannot transform an empty image");
    // Same padded, centred P x Q grid used by the filter, so the spectrum
    // shows exactly the frequencies the transfer function is applied to.
    const int P = 2 * img.height, Q = 2 * img.width;
    FftwBuffer buffer(static_cast<size_t>(P) * Q);
    FftwPlan forward(fftw_plan_dft_2d(P, Q, buffer.data, buffer.data, FFTW_FORWARD, FFTW_ESTIMATE));
    loadPaddedCentered(img, P, Q, buffer.data);
    forward.execute();

    Image spectrum(Q, P);
    for (size_t i = 0; i < spectrum.data.size(); ++i)
        spectrum.data[i] =
            static_cast<float>(std::log1p(std::hypot(buffer.data[i][0], buffer.data[i][1])));
    return normalizeToByteRange(spectrum);
}
