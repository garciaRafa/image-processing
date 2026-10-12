#!/usr/bin/env bash
# Question 4: median filtering of the salt-and-pepper images.
# Run from the repository root after building: ./scripts/run_q4.sh
# Besides the images, writes results/q4/q4_impulses.csv (used by the report table).
set -euo pipefail

PDI=./build/pdi
OUT=results/q4
mkdir -p "$OUT"
SUMMARY="$OUT/q4_impulses.csv"
echo "image,filter,before,after" > "$SUMMARY"

# Runs the median filter and appends the impulse percentages to the summary.
median() {  # median <input> <name> <d> <iterations> <image label> <filter label>
    local line
    line=$("$PDI" median "$1" "$OUT/${2}_median${3}x${4}.png" "$3" "$4")
    echo "$line"
    local before after
    before=$(echo "$line" | sed -E 's/.* ([0-9.e+-]+)% -> .*/\1/')
    after=$(echo "$line" | sed -E 's/.*-> ([0-9.e+-]+)%.*/\1/')
    printf "%s,%s,%.2f,%.2f\n" "$5" "$6" "$before" "$after" >> "$SUMMARY"
}

# Salt-and-pepper images with unknown noise level: 3x3 median.
for name in bear_s_and_p boat_s_and_p glass_s_and_p; do
    echo "== $name"
    "$PDI" convert "data/${name}.pgm" "$OUT/${name}_original.png"
    median "data/${name}.pgm" "$name" 3 1 "${name%%_*}" '$3\times 3$'
done

# Lungs with 10%, 30% and 50% noise: window size and repeated passes.
for p in 10 30 50; do
    name="lungs_s_and_p_${p}"
    echo "== $name"
    "$PDI" convert "data/${name}.pgm" "$OUT/${name}_original.png"
    label="lungs (${p}\\%)"
    median "data/${name}.pgm" "$name" 3 1 "$label" '$3\times 3$'
    median "data/${name}.pgm" "$name" 5 1 "$label" '$5\times 5$'
    median "data/${name}.pgm" "$name" 7 1 "$label" '$7\times 7$'
    median "data/${name}.pgm" "$name" 3 3 "$label" '$3\times 3$ (3 passes)'
done

# Comparison with a linear low-pass filter of the same size (Question 2).
"$PDI" conv data/lungs_s_and_p_30.pgm "$OUT/lungs_s_and_p_30_mean3.png" mean 3

echo "Results written to $OUT/ (summary in $SUMMARY)"
