#!/bin/sh

set -eu
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
"${repo_dir}/tests/run_phase2.sh"
"${repo_dir}/tests/run_phase5.sh"
"${repo_dir}/tests/run_phase4.sh"
