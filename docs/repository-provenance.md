# Repository and dependency provenance

## Authoritative working repository

For this implementation effort, the GitHub repository at
`https://github.com/barbalet/london1940.git` is authoritative. The Phase 0
baseline was established from commit
`a15b9f338fc970e6ca128bbf35a555908725d261` on 2026-07-15. Future baseline
reports record the exact checked-out revision automatically.

The related GitLab project at `https://gitlab.com/apesdk/london1940` is useful
historical and upstream context, but it is not presently interchangeable with
this checkout. The GitLab tree includes an `apesdk` submodule and a `londonium`
component that are absent from the GitHub tree, while the checked-out GitHub
repository contains no tracked `.gitmodules` file.

Changes should not be copied blindly between the two repositories. Before any
reconciliation, compare commit ancestry, component ownership, licenses, build
entry points, and the expected ApeSDK revision. Record the chosen source commit
in the resulting merge or dependency update.

## ApeSDK dependency

The archived components use a repository-local ApeSDK checkout:

- `mapedit/` includes files from `../apesdk/toolkit` and `../apesdk/shared.h`;
- `urban/` includes toolkit, script, render, simulation, entity, CLI, and
  universe sources from `../apesdk`; and
- the macOS project files also refer to relative ApeSDK source paths.

`dependencies/apesdk.lock.json` pins commit
`a194b760d7f123e512de0207954d716b3d230fcb`, the gitlink recorded by the related
GitLab London 1940 repository. `python3 tools/fetch_apesdk.py` clones that exact
detached revision into ignored `apesdk/`; `--verify-only` rejects any other
checkout. The maintained ingestion pipeline does not require ApeSDK. See
`docs/component-status.md` for why the consumers remain archived.

## Source-map dependency

The ten large PNG sheets live in
`https://gitlab.com/apesdk/southeastengland1940maps`. `data/source-maps.json`
pins repository commit `cb31a79468ad43a04967c0ab3c50b2be157b0562`, the
individual Git blob checksum for every PNG, license, and attribution.
`newlondmaps.sh --output-dir DIRECTORY [--sheet 160]` streams only requested
files and verifies their Git content checksum; `--verify-only` performs no
download. Phase 6 will extend this acquisition manifest with decoded SHA-256,
dimensions, geographic bounds, and extraction profiles before ingestion.
