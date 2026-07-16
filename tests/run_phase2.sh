#!/usr/bin/env bash

set -euo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
python="${PYTHON:-python3}"
output_root="${repo_dir}/build/phase2"
expectations="${repo_dir}/tests/fixtures/mask-expectations.json"

"${python}" -c 'import jsonschema' 2>/dev/null || {
    echo "Install test dependencies with: ${python} -m pip install -r requirements-test.txt" >&2
    exit 1
}

rm -rf "${output_root}"
mkdir -p "${output_root}/run-a" "${output_root}/run-b"

for run in run-a run-b; do
    for input in "${repo_dir}"/examples/*.json; do
        name="$(basename "${input}" .json)"
        fixture_dir="${output_root}/${run}/${name}"
        v1_path="${output_root}/${run}/${name}.v1.json"
        "${python}" "${repo_dir}/tools/map_migrate.py" migrate "${input}" \
            --source-root "${repo_dir}/examples" --output "${v1_path}"
        "${python}" "${repo_dir}/tools/map_validate.py" validate-map "${v1_path}" \
            --output-dir "${fixture_dir}" --expectations "${expectations}" \
            > "${output_root}/${run}/${name}.report.stdout.json"
    done
done

for name in new_ashford new_canterbury new_maidstone; do
    cmp "${output_root}/run-a/${name}.v1.json" "${output_root}/run-b/${name}.v1.json"
    diff -qr "${output_root}/run-a/${name}" "${output_root}/run-b/${name}"
done

PYTHONPYCACHEPREFIX="${TMPDIR:-/tmp}/london1940-phase2-pycache" \
    "${python}" -m unittest discover -s "${repo_dir}/tests" -v

echo "Phase 2 validation passed; artifacts: ${output_root}"
