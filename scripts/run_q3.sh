#!/usr/bin/env bash
# Question 3: histogram equalization of the low-contrast images.
# Run from the repository root after building: ./scripts/run_q3.sh
set -euo pipefail

PDI=./build/pdi
OUT=results/q3
mkdir -p "$OUT"

for in in data/woman_low_contrast.pgm data/Google_JAX_low_contrast.pgm \
          data/Hill.png data/RX_a.png data/RX_b.png data/RX_c.png data/RX_d.png; do
    name=$(basename "${in%.*}" | tr '[:upper:]' '[:lower:]')
    name=${name%_low_contrast}
    echo "== $name"
    "$PDI" convert "$in" "$OUT/${name}_original.png"
    "$PDI" histeq "$in" "$OUT/${name}_equalized.png" "$OUT/${name}_hist"
done

echo "Results written to $OUT/"
