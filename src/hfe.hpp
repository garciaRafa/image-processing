#pragma once

#include "image.hpp"

enum class HighPassType { Gaussian, Butterworth };

struct HfeParams {
    double k1 = 0.5;      // offset: keeps a fraction of the low frequencies (and the DC term)
    double k2 = 1.0;      // weight of the high-frequency contribution
    double d0 = 40.0;     // cutoff distance, in samples of the padded frequency grid
    HighPassType type = HighPassType::Gaussian;
    int order = 2;        // Butterworth order n (ignored for the Gaussian filter)
};

// High-frequency emphasis filtering in the frequency domain:
//
//   g(x, y) = F^{-1}{ [k1 + k2 H_HP(u, v)] F(u, v) }
//
// with H_HP the Gaussian high-pass  1 - exp(-D^2 / (2 D0^2))  or the
// Butterworth high-pass  1 / (1 + (D0 / D)^(2n)),  where D(u, v) is the
// distance to the centre of the frequency rectangle. Steps:
//   1. pad the M x N image to P x Q = 2M x 2N (avoids wraparound error) by
//      replicating its border pixels;
//   2. multiply by (-1)^(x+y) to centre the spectrum (Question 6);
//   3. compute the DFT with FFTW, multiply by the transfer function;
//   4. inverse DFT, keep the real part, undo the centring and crop M x N.
// The result is returned in floating point and is not clipped.
Image highFrequencyEmphasis(const Image& img, const HfeParams& params);

// Centred log-magnitude spectrum log(1 + |F(u, v)|) of the padded 2M x 2N
// image, scaled to [0, 255].
Image logSpectrum(const Image& img);
