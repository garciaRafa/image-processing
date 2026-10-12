#!/usr/bin/env bash
# Question 5: high-frequency emphasis filtering of the PET image.
# Run from the repository root after building: ./scripts/run_q5.sh
# Output names avoid dots in the parameters (k1 = 0.5 -> k1_050) so that
# LaTeX does not mistake them for file extensions.
set -euo pipefail

PDI=./build/pdi
OUT=results/q5
IN="data/full body PET - original-HSV.png"
D0=40
mkdir -p "$OUT"

tag() { awk -v k="$1" 'BEGIN { printf "%03d", k * 100 + 0.5 }'; }  # 0.5 -> 050, 2 -> 200

"$PDI" convert "$IN" "$OUT/pet_original.png"
"$PDI" spectrum "$IN" "$OUT/pet_spectrum.png"

# Grid of k1 (offset) and k2 (high-frequency weight), Gaussian high-pass.
for k1 in 0.25 0.5 1.0; do
    for k2 in 0.5 1.0 2.0; do
        "$PDI" hfe "$IN" "$OUT/pet_k1_$(tag $k1)_k2_$(tag $k2).png" "$k1" "$k2" "$D0" gaussian
    done
done

# Butterworth (n = 2) with the same parameters, for comparison.
"$PDI" hfe "$IN" "$OUT/pet_butterworth_k1_050_k2_200.png" 0.5 2.0 "$D0" butterworth 2

# Pure high-pass (k1 = 0), to visualise the emphasized component alone.
"$PDI" hfe "$IN" "$OUT/pet_highpass_only.png" 0 1.0 "$D0" gaussian

# High-frequency emphasis followed by histogram equalization (Question 3).
"$PDI" histeq "$OUT/pet_k1_050_k2_200.png" "$OUT/pet_k1_050_k2_200_histeq.png"

echo "Results written to $OUT/"
