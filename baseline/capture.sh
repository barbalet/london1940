#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd "${script_dir}/.." && pwd)"
fixture_manifest="${script_dir}/fixtures.json"
output_dir="${1:-${repo_dir}/build/phase0-baseline}"

for command_name in cc jq shasum python3; do
    if ! command -v "${command_name}" >/dev/null 2>&1; then
        echo "Required command not found: ${command_name}" >&2
        exit 1
    fi
done

if [ -e "${output_dir}" ]; then
    echo "Baseline output already exists: ${output_dir}" >&2
    echo "Remove it or pass a different output directory." >&2
    exit 1
fi

mkdir -p "${output_dir}/bin" "${output_dir}/fixtures"

git_revision="unknown"
if command -v git >/dev/null 2>&1; then
    git_revision="$(git -C "${repo_dir}" rev-parse HEAD 2>/dev/null || printf unknown)"
fi

{
    echo "revision=${git_revision}"
    echo "capturedUtc=$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
    echo "system=$(uname -a)"
    cc --version 2>&1 | head -n 1
} > "${output_dir}/environment.txt"

echo "Building legacy map2json"
cc -Wall -std=c11 -pedantic -O3 \
    -I"${repo_dir}/map/src" \
    "${repo_dir}"/map/src/*.c -lm \
    -o "${output_dir}/bin/map2json" \
    > "${output_dir}/build.log" 2>&1

measure_conversion() {
    local fixture_name="$1"
    local input_png="$2"
    local fixture_dir="${output_dir}/fixtures/${fixture_name}"
    local log_file="${fixture_dir}/conversion.log"
    local elapsed peak_rss status map_json_bytes map2_json_bytes render_bytes
    local diagnostic_count diagnostic_bytes

    mkdir -p "${fixture_dir}"
    set +e
    python3 "${script_dir}/run_measured.py" \
        --cwd "${fixture_dir}" \
        --log "${log_file}" \
        --metrics "${fixture_dir}/metrics.json" \
        "${output_dir}/bin/map2json" -f "${input_png}" --apesdk
    status=$?
    set -e

    if [ "${status}" -ne 0 ]; then
        echo "Conversion failed for ${fixture_name}; see ${log_file}" >&2
        exit "${status}"
    fi

    elapsed="$(jq '.elapsedSeconds' "${fixture_dir}/metrics.json")"
    peak_rss="$(jq '.peakRssKb' "${fixture_dir}/metrics.json")"

    (
        cd "${fixture_dir}"
        "${output_dir}/bin/map2json" -f map.json -o semantic-render.png
    ) >> "${log_file}" 2>&1

    map_json_bytes="$(wc -c < "${fixture_dir}/map.json" | tr -d ' ')"
    map2_json_bytes="$(wc -c < "${fixture_dir}/map2.json" | tr -d ' ')"
    render_bytes="$(wc -c < "${fixture_dir}/semantic-render.png" | tr -d ' ')"
    diagnostic_count="$(find "${fixture_dir}" -maxdepth 1 -type f -name '*.png' ! -name semantic-render.png | wc -l | tr -d ' ')"
    diagnostic_bytes="$(find "${fixture_dir}" -maxdepth 1 -type f -name '*.png' ! -name semantic-render.png -exec wc -c {} + | awk '{total += $1} END {print total + 0}')"

    jq -n \
        --arg name "${fixture_name}" \
        --argjson elapsedSeconds "${elapsed}" \
        --argjson peakRssKb "${peak_rss}" \
        --argjson mapJsonBytes "${map_json_bytes}" \
        --argjson map2JsonBytes "${map2_json_bytes}" \
        --argjson semanticRenderBytes "${render_bytes}" \
        --argjson diagnosticPngCount "${diagnostic_count}" \
        --argjson diagnosticPngBytes "${diagnostic_bytes}" \
        --argjson buildings "$(awk '/^Polygons [0-9]+$/ {print $2; exit}' "${log_file}")" \
        --argjson woodland "$(awk '/ woodland areas$/ {print $1; exit}' "${log_file}")" \
        --argjson orchards "$(awk '/ orchards detected$/ {print $1; exit}' "${log_file}")" \
        --argjson waterPoints "$(awk '/ water line points$/ {print $1; exit}' "${log_file}")" \
        --argjson stations "$(awk '/ station points$/ {print $1; exit}' "${log_file}")" \
        --argjson mainRoadPoints "$(awk '/ main road line points$/ {print $1; exit}' "${log_file}")" \
        --argjson minorRoadPoints "$(awk '/ minor road line points$/ {print $1; exit}' "${log_file}")" \
        --argjson bridges "$(awk '/ bridges$/ {print $1; exit}' "${log_file}")" \
        --argjson junctions "$(awk '/ junctions$/ {print $1; exit}' "${log_file}")" \
        '{name:$name,elapsedSeconds:$elapsedSeconds,peakRssKb:$peakRssKb,artifactSizes:{mapJsonBytes:$mapJsonBytes,map2JsonBytes:$map2JsonBytes,semanticRenderBytes:$semanticRenderBytes,diagnosticPngCount:$diagnosticPngCount,diagnosticPngBytes:$diagnosticPngBytes},observedFeatures:{buildings:$buildings,woodland:$woodland,orchards:$orchards,waterPoints:$waterPoints,stations:$stations,mainRoadPoints:$mainRoadPoints,minorRoadPoints:$minorRoadPoints,bridges:$bridges,junctions:$junctions}}' \
        > "${fixture_dir}/summary.json"

    (
        cd "${fixture_dir}"
        find . -type f ! -name artifact-sha256.txt ! -name .DS_Store -print0 |
            sort -z |
            xargs -0 shasum -a 256
    ) > "${fixture_dir}/artifact-sha256.txt"
}

fixture_count="$(jq '.fixtures | length' "${fixture_manifest}")"
for ((fixture_index = 0; fixture_index < fixture_count; fixture_index++)); do
    fixture_name="$(jq -r ".fixtures[${fixture_index}].name" "${fixture_manifest}")"
    relative_png="$(jq -r ".fixtures[${fixture_index}].png" "${fixture_manifest}")"
    expected_sha="$(jq -r ".fixtures[${fixture_index}].pngSha256" "${fixture_manifest}")"
    input_png="${repo_dir}/${relative_png}"
    actual_sha="$(shasum -a 256 "${input_png}" | awk '{print $1}')"

    if [ "${actual_sha}" != "${expected_sha}" ]; then
        echo "Fixture checksum mismatch: ${relative_png}" >&2
        exit 1
    fi

    echo "Capturing ${fixture_name}"
    measure_conversion "${fixture_name}" "${input_png}"
done

jq -s \
    --arg revision "${git_revision}" \
    '{format:"london1940-phase0-results",version:1,revision:$revision,fixtures:.}' \
    "${output_dir}"/fixtures/*/summary.json \
    > "${output_dir}/summary.json"

(
    cd "${output_dir}"
    find . -type f ! -name SHA256SUMS ! -name .DS_Store -print0 |
        sort -z |
        xargs -0 shasum -a 256
) > "${output_dir}/SHA256SUMS"

echo "Baseline captured in ${output_dir}"
jq . "${output_dir}/summary.json"
