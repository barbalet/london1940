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
| `map/` | `map2json`, tiling, feature extraction, JSON assembly, elevation import, and JSON rendering | Core C extractor builds; pipeline and format need repair |
| `mapedit/` | Command-line renderer and macOS JSON/map editor | Requires an ApeSDK source tree that is absent here |
| `londonmap/` | Experimental processing of four large London sheets | Uses hard-coded local paths and fixed dimensions |
| `examples/` | Ashford, Canterbury, and Maidstone PNG/JSON examples | Useful fixtures, but no automated correctness checks |
| `urban/` | Older generic urban simulation/game prototype | Adjacent research code; also requires ApeSDK |
| `occupations/` | Early occupation taxonomy/data | Incomplete and not integrated with map conversion |
| `newlondmaps.sh` | Legacy download helper | Downloads an opaque ZIP into a user-specific directory |

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

## Proposed JSON format

The current JSON records useful concepts but has two incompatible polygon
representations. A versioned format should be defined before processing the
full-resolution sheets. At minimum, every document should contain:

```json
{
  "format": "london1940-map",
  "version": 1,
  "source": {
    "filename": "160.png",
    "sha256": "..."
  },
  "crs": "EPSG:27700",
  "tile": [0, 0],
  "offset": [0, 0],
  "resolution": [1557, 1113],
  "bounds": [495000, 225000, 535000, 180000],
  "terrain": [],
  "urban": []
}
```

The final schema should use one polygon representation everywhere and define
whether rings are closed, their winding order, how interior rings are stored,
and whether coordinates are tile-local or sheet-global. Linear features should
use explicit point arrays and link indices with defined widths and units.
Coordinates should be integer and delta-encoded in the compact representation;
repeated feature names and properties should use shared tables. The canonical
uncompressed representation should be minified UTF-8 JSON with stable key and
feature ordering. Distribution artifacts should be deterministic gzip files
named `*.json.gz`; Zstandard can be benchmarked as an additional artifact, but
must not be the only supported encoding.

Every generated map should carry the source PNG's SHA-256 digest, converter
version, extraction profile, coordinate reference system, and schema version.
This makes outputs reproducible when extraction thresholds change.

## Current status

### What works

- `map/map2json` compiles on macOS with the supplied `Makefile`.
- It reads the three example RGB PNGs and detects multiple feature classes.
- The three checked-in example JSON documents are syntactically valid.
- The Python map-combination utilities pass Python syntax compilation.
- JSON can be rendered as a simplified feature map.

### What is broken or unverified

- **The repository is not self-contained.** `mapedit/` and `urban/` reference
  `../apesdk`, but this GitHub checkout contains neither that directory nor
  submodule metadata. The related GitLab project has diverged and includes
  components absent here.
- **The two JSON dialects do not round-trip.** ApeSDK output writes polygon
  collections as arrays, while the loader expects the older keyed-object form.
  In a test conversion of `new_ashford.png`, extraction reported 116 building
  polygons and 201 woodland areas; the immediate reload reported zero of both.
- **The internal test can validate the wrong file.** After writing the requested
  output, `map2json` reloads the literal path `map.json`. An `-o` conversion can
  therefore read a stale file or fail to test the file it created.
- **A successful exit does not prove success.** Several error paths return zero,
  parse/load results can discard data silently, and feature counts are not
  asserted.
- **Memory use is excessive.** Default fixed maxima report approximately 1.1 GB
  of map buffers even for the 2174 by 1754 Ashford example. Some downstream
  routines still use compile-time maxima instead of configured values.
- **Conversion pollutes the working directory.** A normal run emits numerous
  intermediate PNG files plus `map2.json` without an isolated output directory
  or a diagnostic-output switch.
- **Command-line parsing is fragile.** Arguments are consumed in pairs even
  though `--apesdk` is a flag, values are accessed without consistent bounds
  checks, duplicated conditions exist, and there is no useful `--help` output.
- **Coordinate parsing depends on filenames.** Georeferencing is inferred from
  an undocumented underscore-delimited filename convention. Ordinary example
  names consequently receive zero coordinates.
- **Automation is not a test suite.** GitHub Actions only downloads an external
  ZIP. GitLab CI builds selected components but does not validate schemas,
  feature counts, determinism, compression, or rendered output.
- **Scripts are machine-specific.** Several scripts and C files contain
  `/Users/barbalet/...`, `~/london1940`, or sibling `mapblend` paths.
- **The large-sheet helper is unfinished.** The `shrinkmaps` program in the map
  data repository contains fixed local paths and its `main` only prints
  `Hello, World!`.
- **Elevation validation is defective.** `heightmap.py` iterates elevation values
  and then incorrectly uses each value as an array index.
- **Installation has a copy-and-paste error.** The `Makefile` installs
  `combinemaps.py` as `heightmap` instead of installing `heightmap.py`.
- **Shell file iteration is unsafe.** Several scripts split `find` output on
  whitespace and assume filenames contain no spaces.
- **There is no authoritative schema, fixture manifest, quality baseline, or
  documented release process.**

## Building the currently working component

Requirements: a C11 compiler, `make`, and the standard maths library.

```sh
cd map
make
```

The following demonstrates the legacy interface, not a verified production
workflow:

```sh
./map2json -f ../examples/new_ashford.png -o map.json
./map2json -f map.json -o rendered.png
```

Run it in a disposable working directory because the extractor currently emits
many intermediate files. Do not treat `Ended Successfully` as evidence of a
correct round-trip until the validation work in [ROADMAP.md](ROADMAP.md) is
complete.

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

The next work should stabilise the format and validation harness before adding
the full source sheets. Processing all ten large maps with the current binary
would make unreliable output expensive to review and regenerate. The ordered,
testable plan is maintained in [ROADMAP.md](ROADMAP.md).

Phase 0 is complete. Its tracked fixture manifest and capture command are in
[`baseline/`](baseline/), and repository/dependency findings are recorded in
[`docs/repository-provenance.md`](docs/repository-provenance.md).

## License and attribution

Source code is provided under the terms in [LICENSE](LICENSE). Historical map
imagery and derived map data have separate attribution requirements; consult
the [source-map repository](https://gitlab.com/apesdk/southeastengland1940maps)
and preserve the stated National Library of Scotland/Ordnance Survey attribution
and `CC BY 4.0` notice in generated datasets and releases.
