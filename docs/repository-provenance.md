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

The current GitHub checkout is intentionally documented as incomplete:

- `mapedit/` includes files from `../apesdk/toolkit` and `../apesdk/shared.h`;
- `urban/` includes toolkit, script, render, simulation, entity, CLI, and
  universe sources from `../apesdk`; and
- the macOS project files also refer to relative ApeSDK source paths.

No ApeSDK directory, vendored snapshot, submodule entry, lock file, or fetch
script in this checkout pins the dependency. Consequently `mapedit` and `urban`
cannot be reproduced from a fresh clone. Only `map/map2json`, which carries its
own PNG and JSON code, is included in the Phase 0 executable baseline.

Phase 4 will restore ApeSDK through a pinned and documented dependency. Until
then, a developer's unversioned sibling `apesdk` directory must not be treated
as a verified build.

## Source-map dependency

The ten large PNG sheets live in
`https://gitlab.com/apesdk/southeastengland1940maps`. They are external dataset
inputs rather than source dependencies and are not required for Phase 0. Phase
6 will add a manifest containing the URL, SHA-256 digest, dimensions, bounds,
license attribution, and extraction profile for every sheet before ingesting
them.
