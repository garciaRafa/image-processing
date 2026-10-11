#!/usr/bin/env bash
# Question 2: low-pass and high-pass spatial filtering.
# Run from the repository root after building: ./scripts/run_q2.sh
set -euo pipefail

PDI=./build/pdi
OUT=results/q2
mkdir -p "$OUT"

for name in Hill RX_a; do
    in="data/${name}.png"
    base="$OUT/$(echo "$name" | tr '[:upper:]' '[:lower:]')"

    "$PDI" convert "$in" "${base}_original.png"

    # Low-pass
    "$PDI" conv "$in" "${base}_mean3.png" mean 3
    "$PDI" conv "$in" "${base}_mean9.png" mean 9
    "$PDI" conv "$in" "${base}_gaussian9.png" gaussian 9 2.0

    # High-pass
    "$PDI" conv "$in" "${base}_laplacian4.png" laplacian4
    "$PDI" conv "$in" "${base}_laplacian8.png" laplacian8
    "$PDI" conv "$in" "${base}_sharpen.png" sharpen
done

echo "Results written to $OUT/"
