#!/usr/bin/env bash

set -euo pipefail

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
python=${PYTHON:-python3}
output_root="${repo_dir}/build/phase4"
install_root="${output_root}/install root with spaces"

rm -rf "${output_root}"
mkdir -p "${output_root}"

make -C "${repo_dir}/map" clean all
PYTHONPYCACHEPREFIX="${TMPDIR:-/tmp}/london1940-phase4-pycache" \
    "${python}" -m unittest discover -s "${repo_dir}/tests" -p 'test_*.py' -v

make -C "${repo_dir}/map" PREFIX=/usr DESTDIR="${install_root}" install
"${install_root}/usr/bin/map2json" --help >/dev/null
"${python}" "${install_root}/usr/bin/heightmap" --help >/dev/null
test -x "${install_root}/usr/bin/mapoutdir"
test -f "${install_root}/usr/libexec/london1940/map2json-core"
make -C "${repo_dir}/map" PREFIX=/usr DESTDIR="${install_root}" uninstall
if find "${install_root}" -type f -print -quit | grep . >/dev/null; then
    echo "uninstall left installed files" >&2
    exit 1
fi

jq empty "${repo_dir}/dependencies/apesdk.lock.json" "${repo_dir}/data/source-maps.json"
test "$(jq -r '.revision' "${repo_dir}/dependencies/apesdk.lock.json")" = \
    a194b760d7f123e512de0207954d716b3d230fcb
test "$(jq '.files | length' "${repo_dir}/data/source-maps.json")" -eq 10

if rg -n '/Users/|~/' "${repo_dir}/map" "${repo_dir}/mapedit" \
    "${repo_dir}/londonmap" "${repo_dir}/urban" --glob '*.c' --glob '*.h' \
    --glob '*.sh' --glob '*.pbxproj'; then
    echo "author-specific path remains in source/build files" >&2
    exit 1
fi
if rg -n 'for +[^;]+ +in +\$\(find|for +[^;]+ +in +`find' "${repo_dir}" --glob '*.sh'; then
    echo "unsafe find iteration remains" >&2
    exit 1
fi
while IFS= read -r -d '' script; do
    bash -n "${script}"
done < <(find "${repo_dir}" -path "${repo_dir}/build" -prune -o -type f -name '*.sh' -print0)

echo "Phase 4 validation passed; artifacts: ${output_root}"
