#!/usr/bin/env bash

set -euo pipefail

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
python=${PYTHON:-python3}
output_root="${repo_dir}/build/phase5"

rm -rf "${output_root}"
mkdir -p "${output_root}/work" "${output_root}/metrics" "${output_root}/sensitivity-validation"

PYTHON="${python}" "${repo_dir}/tests/run_phase3.sh"

"${python}" "${repo_dir}/tools/map_quality.py" assess \
    "${repo_dir}/build/phase3/validation/new_ashford/report.json" \
    "${repo_dir}/build/phase3/validation/new_canterbury/report.json" \
    "${repo_dir}/build/phase3/validation/new_maidstone/report.json" \
    --policy "${repo_dir}/tests/fixtures/quality-thresholds.json" \
    > "${output_root}/quality-assessment.json"

"${python}" "${repo_dir}/baseline/run_measured.py" \
    --cwd "${repo_dir}" --log "${output_root}/metrics/sensitivity.log" \
    --metrics "${output_root}/metrics/sensitivity.json" \
    "${repo_dir}/map/map2json" "${repo_dir}/examples/new_ashford.png" \
    --output "${output_root}/sensitivity.v1.json" --work-dir "${output_root}/work" \
    --profile sensitivity-woodland-1

"${python}" "${repo_dir}/tools/map_validate.py" validate-map \
    "${output_root}/sensitivity.v1.json" --output-dir "${output_root}/sensitivity-validation" >/dev/null
"${python}" "${repo_dir}/tools/map_quality.py" compare-profiles \
    "${repo_dir}/build/phase3/new_ashford.v1.json" "${output_root}/sensitivity.v1.json" \
    --output "${output_root}/profile-comparison.json" >/dev/null
test "$(jq -r '.differences | keys | join(",")' "${output_root}/profile-comparison.json")" = "terrain.woodland"

PYTHONPYCACHEPREFIX="${TMPDIR:-/tmp}/london1940-phase5-pycache" \
    "${python}" -m unittest discover -s "${repo_dir}/tests" -p 'test_quality.py' -v

jq -n \
    --slurpfile ashford "${repo_dir}/build/phase3/metrics/ashford.json" \
    --slurpfile canterbury "${repo_dir}/build/phase3/metrics/canterbury.json" \
    --slurpfile maidstone "${repo_dir}/build/phase3/metrics/maidstone.json" \
    --slurpfile sensitivity "${output_root}/metrics/sensitivity.json" \
    '{format:"london1940-phase5-performance",version:1,legacy1:{ashford:$ashford[0],canterbury:$canterbury[0],maidstone:$maidstone[0]},sensitivityWoodland1:$sensitivity[0]}' \
    > "${output_root}/performance.json"

echo "Phase 5 validation passed; artifacts: ${output_root}"
