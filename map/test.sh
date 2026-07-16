#!/bin/sh

set -eu
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
exec "${repo_dir}/tests/run_phase3.sh" "$@"
