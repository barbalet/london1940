# London 1940

London 1940 is a historical-map ingestion project for agent-based simulations of
London and southeast England in 1940. Its immediate scope is Ashford, Maidstone,
and Canterbury; its longer-term scope is the wider London and Operation Sea Lion
simulation area.

The repository's principal job is to extract simulation-relevant features from
scanned Ordnance Survey PNG maps and store them as compact semantic JSON. The
features include buildings, roads, railways, stations, waterways, woodland,
orchards, sand, sea, bridges, junctions, map coordinates, and elevation data.
The JSON is intended to be small and quick for ApeSDK-based urban and war
simulations to load. It is not intended to preserve paper texture, printed
labels, scan noise, or every source pixel.

The project is currently a prototype and is not yet a reproducible end-to-end
pipeline. See [Current status](#current-status) and [ROADMAP.md](ROADMAP.md).

## Project context

- [London1940.org](https://london1940.org/) describes the initial three-town
  simulation and the wider London objective.
- [Sim Sea Lion](https://simsealion.com/) describes the related southeast
  England and Operation Sea Lion simulation.
- [southeastengland1940maps](https://gitlab.com/apesdk/southeastengland1940maps)
  contains the ten full-resolution source sheets: `160`, `161`, `162`, `170`,
  `171`, `172`, `173`, `182`, `183`, and `184`.
- The source imagery derives from historical Ordnance Survey material supplied
  by the National Library of Scotland. Consult the source-map repository and
  its `CC BY 4.0` attribution before redistributing derived map data.

The source sheets are deliberately not duplicated in this repository. They are
large (sheet `160.png` alone is approximately 90 MB) and should be supplied to
the conversion pipeline as external inputs.

## Repository layout

| Path | Purpose | Present state |
| --- | --- | --- |
| `map/` | `map2json`, tiling, feature extraction, JSON assembly, elevation import, and JSON rendering | Checked v1 extraction pipeline; legacy C engine is private |
| `mapedit/` | Command-line renderer and macOS JSON/map editor | Archived pre-v1 research component |
| `londonmap/` | Experimental processing of four large London sheets | Archived; paths replaced by explicit arguments |
| `examples/` | Ashford, Canterbury, and Maidstone PNG/JSON examples | Automated extraction and validation fixtures |
| `urban/` | Older generic urban simulation/game prototype | Archived adjacent research component |
| `occupations/` | Early occupation taxonomy/data | Retained source data; not integrated |
| `newlondmaps.sh` | Verified source-sheet acquisition | Downloads pinned sheets and checks Git blob digests |

## Intended data flow

```text
source PNG sheets
       |
       v
normalise and tile images
       |
       v
extract typed feature geometry  ---> diagnostic feature masks
       |
       v
validate and canonicalise JSON
       |
       +---> minified .json
       +---> compressed .json.gz
       |
       v
combine tiles and sheets
       |
       v
ApeSDK simulation / reference renderer
```

There are two distinct validation goals:

1. **Data round-trip:** canonical JSON must survive encode, compression,
   decompression, and decode without changing its data model.
2. **Extraction quality:** rendering the semantic JSON must reproduce the
   expected feature masks within documented tolerances.

Pixel-identical reconstruction of the original scan is not an appropriate test
of semantic extraction. If exact raster preservation is required, it should be
implemented as a separate optional indexed-raster archive mode. Embedding the
original PNG bytes in JSON would add Base64 overhead without improving on PNG's
existing lossless compression.

## Version 1 JSON format

The legacy JSON records useful concepts but has two incompatible polygon
representations. Phase 1 defines one canonical interchange format in
[`schema/map-v1.schema.json`](schema/map-v1.schema.json), with its geometry and
coordinate semantics documented in
[`docs/map-format-v1.md`](docs/map-format-v1.md). A document begins:

```json
{
  "format": "london1940-map",
  "version": 1,
  "source": {
    "filename": "160.png",
    "sha256": "...",
    "legacyFormat": "apesdk-array"
  },
  "generator": {
    "name": "legacy-map2json",
    "version": null,
    "extractionProfile": "legacy-default-unknown"
  },
  "attribution": {
    "license": "CC-BY-4.0",
    "text": "Contains derived data from historical Ordnance Survey maps supplied by the National Library of Scotland."
  },
  "encoding": {
    "coordinates": "absolute-integer",
    "sharedTables": false
  },
  "coordinates": {
    "space": "sheet-pixel",
    "units": "pixel",
    "origin": "top-left",
    "yAxis": "down",
    "crs": null
  },
  "tile": [0, 0],
  "offset": [0, 0],
  "resolution": [1557, 1113],
  "layers": []
}
```

Version 1 uses `polygons[].rings` everywhere, explicit indexed networks, and
absolute sheet-pixel integer coordinates. It deliberately does not use delta
coordinates or shared tables: readable geometry makes legacy migration easier
to audit, while gzip will provide transport compression in Phase 2. Canonical
JSON is minified UTF-8 with sorted object keys, preserved layer/feature order,
and one trailing newline.

Every generated map should carry the source PNG's SHA-256 digest, converter
version, extraction profile, coordinate reference system, and schema version.
This makes outputs reproducible when extraction thresholds change.

Legacy files can be migrated and validated now:

```sh
python3 tools/map_migrate.py migrate examples/new_ashford.json \
  --source-root examples --output build/new_ashford.v1.json
python3 tools/map_migrate.py validate build/new_ashford.v1.json
```

The test suite validates migrated documents twice: with the repository's
semantic validator and independently with the `jsonschema` Draft 2020-12
implementation. It also migrates a checked-in keyed-object fixture emitted by
the legacy C writer itself.

```sh
python3 -m pip install -r requirements-test.txt
python3 -m unittest discover -s tests -v
```

The complete Phase 2 validation—including two clean deterministic runs,
canonical and gzip round-trips, every per-layer mask, reviewed expectations,
and negative inputs—is run with:

```sh
./tests/run_phase2.sh
```

See [docs/validation-harness.md](docs/validation-harness.md) for artifact and
metric details.

## Current status

### What works

- `map/map2json` builds on macOS and Linux with the supplied `Makefile`.
- It converts all three example RGB PNGs directly to canonical format-v1 JSON.
- Fresh extraction passes schema, semantic, compression, and all 27 reviewed
  per-layer mask checks.
- Normal extraction emits only the requested output; diagnostics are isolated
  and opt-in.
- Checked arguments and pipeline failures produce non-zero exits.
- The Python map-combination utilities pass Python syntax compilation.
- JSON can be rendered as a simplified feature map.

### What is broken or unverified

- **The private C engine remains legacy code.** Its pair-stepping CLI and
  incompatible serializer are deliberately hidden behind the checked driver;
  direct use of `map2json-core` is unsupported.
- **The large-sheet helper is unfinished.** The `shrinkmaps` program in the map
  data repository contains fixed local paths and its `main` only prints
  `Hello, World!`.
- **Full-sheet ingestion is not implemented.** Acquisition is reproducible, but
  tiling, overlap deduplication, georeferencing manifests, and sheet-level
  validation remain Phase 6 work.
- **Archived components are not supported builds.** `urban/`, `londonmap/`, and
  `mapedit/` remain for research provenance; see
  [docs/component-status.md](docs/component-status.md).

## Building the currently working component

Requirements: Python 3, a C11 compiler, `make`, and the standard maths library.

```sh
python3 -m pip install -r requirements-test.txt
make -C map
./tests/run_supported.sh
```

```sh
./map/map2json examples/new_ashford.png \
  --output build/new_ashford.v1.json \
  --work-dir build/map2json-work
```

See [docs/map2json.md](docs/map2json.md) for profiles, diagnostics, failure
semantics, validation, and measured memory use.

## Definition of a valid conversion

A source sheet is converted successfully only when all of the following hold:

- the source digest and extraction configuration are recorded;
- output validates against the versioned schema;
- all coordinates, indices, ring rules, and feature properties satisfy semantic
  invariants;
- canonical generation is byte-for-byte deterministic across two clean runs;
- `gunzip(gzip(canonical JSON))` is byte-identical to the canonical JSON;
- decoded data is deeply equal to the pre-compression data model;
- the semantic render meets per-class mask thresholds against reviewed fixtures;
- no feature class disappears during a JSON load/save cycle;
- malformed input produces a non-zero exit and an actionable message; and
- peak memory, runtime, and artifact sizes are recorded.

Exact comparison should be used for JSON and binary masks. Reviewed tolerances
should be used for vector-to-raster feature masks where polygon simplification,
line width, or antialiasing makes exact pixels inappropriate. Metrics should be
reported separately for each class so that a good road result cannot hide lost
buildings or waterways.

## Development priorities

The next work is Phase 4 repository reproducibility before adding the full
source sheets. The ordered, testable plan is maintained in
[ROADMAP.md](ROADMAP.md).

Phases 0 through 5 are complete. The tracked baseline fixture manifest and capture command are in
[`baseline/`](baseline/), and repository/dependency findings are recorded in
[`docs/repository-provenance.md`](docs/repository-provenance.md). The v1 schema,
format semantics, migration command, and tests are in `schema/`, `docs/`,
`tools/`, and `tests/` respectively.

## License and attribution

Source code is provided under the terms in [LICENSE](LICENSE). Historical map
imagery and derived map data have separate attribution requirements; consult
the [source-map repository](https://gitlab.com/apesdk/southeastengland1940maps)
and preserve the stated National Library of Scotland/Ordnance Survey attribution
and `CC BY 4.0` notice in generated datasets and releases.
