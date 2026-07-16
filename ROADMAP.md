# London 1940 implementation roadmap

This roadmap prioritises a trustworthy PNG-to-semantic-JSON data pipeline. The
simulation goals depend on that data, so map-format and validation work comes
before scaling to all ten source sheets or millions of agents.

Each phase has an explicit completion gate. A phase is complete only when its
gate is automated in CI or recorded as a reviewed data decision.

## Phase 0: preserve a baseline — complete

1. Record clean conversion logs, runtime, peak memory, output sizes, and feature
   counts for Ashford, Canterbury, and Maidstone.
2. Collect the current generated JSON, semantic render, and individual feature
   masks as legacy fixtures without declaring them correct.
3. Add a fixture manifest containing input SHA-256 hashes and expected image
   dimensions.
4. Document which GitHub or GitLab repository is authoritative and reconcile
   the missing ApeSDK submodule/component history.

**Completion gate:** one command captures the legacy baseline in an ignored
build directory without modifying tracked files.

**Completion evidence:** `baseline/capture.sh` performs the clean capture,
`baseline/fixtures.json` pins the three inputs, and
`docs/repository-provenance.md` records repository and dependency status. The
reference capture is written to `build/phase0-baseline` with per-fixture logs,
metrics, artifacts, and checksums.

## Phase 1: define one map format

1. Write JSON Schema for `london1940-map` version 1.
2. Choose a single representation for exterior and interior polygon rings.
3. Define line points, link indices, widths, units, and invalid-link behaviour.
4. Define coordinate spaces: tile-local pixels, sheet-global pixels, OSGB36
   (`EPSG:27700`) bounds, and optional WGS84 metadata.
5. Define stable ordering, integer/delta coordinate encoding, shared feature
   tables, missing values, and forward-compatible extension fields.
6. Record source digest, converter version, extraction profile, schema version,
   and attribution in every document.
7. Provide a migration command for both legacy keyed-object JSON and current
   ApeSDK array JSON.

**Completion gate:** all three example documents migrate to version 1, validate
against the schema, and retain identical feature counts after load/save.

## Phase 2: build the validation harness

1. Add schema and semantic-invariant validation:
   - coordinates within declared bounds;
   - valid polygon rings and interior-ring ownership;
   - valid point/link indices;
   - widths aligned with line points;
   - unique identifiers where required; and
   - non-negative, plausible resolutions and offsets.
2. Add canonical JSON serialisation and deep data-model equality checks.
3. Add deterministic gzip generation with fixed metadata and round-trip byte
   equality. Benchmark Zstandard separately before making it optional.
4. Render a separate binary or indexed mask for each feature class.
5. Compare reviewed expected masks using exact equality where possible and
   per-class precision, recall, and intersection-over-union where simplification
   requires tolerance.
6. Make zero-feature regressions fail when a fixture expects features.
7. Add malformed PNG, malformed JSON, truncated JSON, and incompatible-version
   tests.

**Completion gate:** CI proves JSON and gzip round-trips, reports every feature
class independently, and reproduces identical outputs on two clean runs.

## Phase 3: repair and isolate `map2json`

1. Replace pair-stepping argument parsing with a checked parser and add
   `--help`, `--version`, `--output`, `--work-dir`, `--diagnostics`,
   `--profile`, and `--format-version`.
2. Make every failure return non-zero and propagate allocation, PNG, JSON,
   schema, and file-write errors.
3. Reload the actual requested output during self-validation, never a hard-coded
   filename.
4. Remove the incompatible ApeSDK serializer or make it a named legacy export
   generated from the versioned internal model.
5. Put intermediates in an explicit work directory and emit them only when
   diagnostics are enabled.
6. Replace large fixed allocations with image/feature-sized growing buffers and
   enforce configurable safety limits consistently.
7. Separate extraction, data model, serialisation, and rendering so each can be
   tested independently.
8. Replace filename-only georeferencing with explicit metadata or a sheet
   manifest; retain legacy filename parsing only as a checked compatibility
   mode.

**Completion gate:** all example conversions pass Phase 2, `-o` is honoured,
failed conversions are non-zero, normal runs produce only requested artifacts,
and peak memory is measured and substantially below the legacy baseline.

## Phase 4: make the repository reproducible

1. Restore and pin ApeSDK using a documented submodule or dependency-fetch
   mechanism; do not rely on an unversioned sibling directory.
2. Remove hard-coded home-directory paths from C, shell, and Xcode projects.
3. Replace the opaque `newlondmaps.zip` workflow with documented source-map
   acquisition and checksum verification.
4. Fix `heightmap.py` validation and add a small elevation fixture.
5. Correct the `heightmap.py` install target and verify install/uninstall in a
   temporary prefix.
6. Make shell scripts quote paths, handle spaces, fail fast, and use temporary
   or caller-specified output directories.
7. Decide whether `urban/`, `occupations/`, and the experimental `londonmap/`
   belong in this repository; document, repair, or archive each deliberately.
8. Align GitHub Actions and GitLab CI around the same build and test command.

**Completion gate:** a fresh clone can build and run all supported tests from
documented commands without author-specific paths or unverified downloads.

## Phase 5: calibrate semantic extraction

1. Review reference masks for buildings, main/minor roads, railway lines and
   tunnels, stations, rivers, lakes/sea, woodland, orchards, sand, bridges, and
   junctions.
2. Move threshold constants into named, versioned extraction profiles.
3. Establish class-specific quality thresholds and document known ambiguity in
   the historical map symbology.
4. Add tile-boundary fixtures so features crossing overlaps remain connected
   and are not duplicated.
5. Verify polygon simplification does not erase small buildings or corrupt
   interior rings.
6. Profile runtime and memory and optimise only after correctness metrics exist.

**Completion gate:** reviewed fixtures meet agreed per-class thresholds and
threshold/profile changes create explicit, reviewable output differences.

## Phase 6: ingest the ten source sheets

1. Add a manifest for sheets `160`, `161`, `162`, `170`, `171`, `172`, `173`,
   `182`, `183`, and `184`, including download location, digest, dimensions,
   bounds, attribution, and extraction profile.
2. Process one representative sheet end-to-end before starting the batch.
3. Tile large sheets with declared overlap and deterministic names.
4. Validate each tile, combine the sheet, deduplicate overlap features, and
   validate again at sheet level.
5. Render and review per-class sheet masks.
6. Combine sheets only after boundary links and georeferencing pass checks.
7. Publish canonical `.json`, deterministic `.json.gz`, manifests, validation
   reports, checksums, and attribution; keep diagnostics out of releases.

**Completion gate:** all ten sheets are reproducible from the manifest, every
artifact passes schema/semantic/compression tests, and boundary-quality reports
contain no unexplained feature loss.

## Phase 7: integrate the simulation

1. Load version 1 maps directly into the maintained ApeSDK urban/war model.
2. Define simulation-scale spatial indexing and streaming so the whole dataset
   need not be resident for local activity.
3. Validate doors, building accessibility, road/rail/water routing, and movement
   between tiles before increasing population.
4. Integrate occupation data with provenance and explicit day/night behaviour.
5. Establish memory and update-time budgets per agent and per active map region.
6. Scale through measured population tiers rather than jumping directly to the
   historical London population target.

**Completion gate:** a documented three-town scenario loads released map data,
runs deterministically, routes agents across feature and tile boundaries, and
meets defined memory/performance budgets.

## Deferred decisions

These are valid project questions but should not block the map pipeline:

- the final population scale and level of agent cognition;
- implicit versus explicit currency and economic value;
- class and occupational hierarchies;
- detailed building interiors, doors, and windows;
- expansion from the initial towns to London and the full Sea Lion area; and
- gameplay and visual-interface direction.

They should be revisited after Phase 6 provides a stable, measured dataset.
