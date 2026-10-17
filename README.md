# Image Processing — Homework 1 (DIM0888)

C++ implementations of the practical questions of Homework 1 of the Image
Processing course (DIM0888). The report answering all questions is in
`report/report.pdf`.

| Question | Topic | Source |
|---|---|---|
| Q2 | Spatial convolution (mean, Gaussian, Laplacian, sharpening) | `src/convolution.cpp` |
| Q3 | Histogram equalization | `src/histogram.cpp` |
| Q4 | Median filter | `src/median.cpp` |
| Q5 | High-frequency emphasis filter (FFT) | `src/hfe.cpp` |

## Requirements

- C++17 compiler and CMake ≥ 3.16
- FFTW3: `sudo apt install libfftw3-dev`

PNG reading and writing uses [stb_image](https://github.com/nothings/stb)
(included in `external/`).

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Usage

Each question has a script that generates its results in `results/qN/`.
Run them from the repository root:

```bash
./scripts/run_q2.sh
./scripts/run_q3.sh
./scripts/run_q4.sh
./scripts/run_q5.sh
```

The `pdi` executable can also be used directly. Run `./build/pdi` without
arguments to list the available commands, for example:

```bash
./build/pdi conv data/bear_s_and_p.pgm out.png gaussian 9 2
./build/pdi median data/boat_s_and_p.pgm out.png 3
./build/pdi hfe data/RX_a.png out.png 1.0 1.0
```

## Repository structure

```
src/        C++ source code
scripts/    one script per question (Q2–Q5)
data/       input images
external/   third-party headers (stb)
report/     homework report (PDF)
```
