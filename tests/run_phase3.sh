#!/bin/sh

set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
python=${PYTHON:-python3}
output_root="${repo_dir}/build/phase3"
baseline="${repo_dir}/build/phase0-baseline/summary.json"

rm -rf "${output_root}"
mkdir -p "${output_root}/work" "${output_root}/validation" "${output_root}/metrics"
make -C "${repo_dir}/map" clean all

"${repo_dir}/map/map2json" --help >/dev/null
"${repo_dir}/map/map2json" --version | grep 'map2json 2.0.0' >/dev/null
if "${repo_dir}/map/map2json" --unknown-option >/dev/null 2>&1; then
    echo "unknown option unexpectedly succeeded" >&2
    exit 1
fi
if "${repo_dir}/map/map2json" "${repo_dir}/tests/fixtures/malformed.png" >/dev/null 2>&1; then
    echo "missing/malformed PNG unexpectedly succeeded" >&2
    exit 1
fi

for name in new_ashford new_canterbury new_maidstone; do
    short=${name#new_}
    destination="${output_root}/${name}.v1.json"
    "${python}" "${repo_dir}/baseline/run_measured.py" \
        --cwd "${repo_dir}" \
        --log "${output_root}/metrics/${short}.log" \
        --metrics "${output_root}/metrics/${short}.json" \
        "${repo_dir}/map/map2json" "${repo_dir}/examples/${name}.png" \
        --output "${destination}" --work-dir "${output_root}/work"
    test -f "${destination}"
    "${python}" "${repo_dir}/tools/map_validate.py" validate-map "${destination}" \
        --output-dir "${output_root}/validation/${name}" \
        --expectations "${repo_dir}/tests/fixtures/mask-expectations.json" >/dev/null
done

test -z "$(find "${output_root}/work" -mindepth 1 -print -quit)"

"${python}" -m unittest discover -s "${repo_dir}/tests" -p 'test_map2json.py' -v

if [ -f "${baseline}" ]; then
    "${python}" - "${baseline}" "${output_root}/metrics" <<'PY'
import json, sys
from pathlib import Path
baseline = {item["name"]: item for item in json.loads(Path(sys.argv[1]).read_text())["fixtures"]}
for path in sorted(Path(sys.argv[2]).glob("*.json")):
    current = json.loads(path.read_text())
    name = path.stem
    old = baseline[name]["peakRssKb"]
    new = current["peakRssKb"]
    reduction = (old - new) / old
    if reduction < 0.20:
        raise SystemExit(f"{name}: peak RSS reduction {reduction:.1%} is below 20%")
    print(f"{name}: peak RSS {old} -> {new} KiB ({reduction:.1%} reduction)")
PY
else
    echo "warning: Phase 0 metrics absent; memory comparison skipped" >&2
fi

echo "Phase 3 validation passed; artifacts: ${output_root}"
