# Phase 3 `map2json` pipeline

`map2json` is now a checked pipeline rather than direct access to the historical
C command line. `make -C map` builds the C feature extractor as the private
`map2json-core` engine and installs the Python driver as `map/map2json`. This
boundary retains the reviewed extraction algorithm while separating extraction,
legacy decoding, the v1 data model, canonical serialization, and Phase 2
validation.

## Build and convert

```sh
python3 -m pip install -r requirements-test.txt
make -C map
./map/map2json examples/new_ashford.png \
  --output build/new_ashford.v1.json \
  --work-dir build/map2json-work \
  --profile legacy-1 \
  --format-version 1 \
  --metadata sheet-160-metadata.json
```

The positional input and `--output` paths are resolved before the engine runs.
`-f`/`--filename` remains as an input alias, but supplying both forms is an
error. Unknown options, missing values, unsupported formats or versions,
missing and malformed PNGs, engine failures, missing legacy output, invalid
JSON, invalid v1 data, and failed writes return non-zero.

`--metadata` replaces filename-derived georeferencing with an explicit JSON
object containing `geographicBounds` and/or `tile`. Bounds use the v1
`EPSG:4326`, `topLeft`, and `bottomRight` structure. Metadata is schema-checked
with the final document; omitting it retains checked legacy filename parsing.

Normal conversion creates one requested artifact. The engine runs inside a
unique directory beneath `--work-dir`; intermediate legacy JSON, logs, and PNGs
are deleted after successful or failed normal runs. `--diagnostics` retains the
unique directory and prints its path to standard error. Atomic replacement
prevents a partially written requested output, and self-validation reloads that
exact temporary destination before replacement.

The C engine's ApeSDK-array JSON is now an explicitly private legacy boundary.
It is immediately decoded into the v1 model and is never exposed as the public
output. Generated documents identify `map2json` version `2.0.0`, extraction
profile `legacy-1`, and the source digest.

## Memory and safety limits

The driver derives polygon, road, and junction capacities from PNG dimensions,
with conservative minimums, instead of accepting the historical defaults of
10,485,760 polygon points and 2,621,440 points per network. The engine's
existing bounds remain the overflow safety mechanism. Normal runs also suppress
diagnostic PNG encoding and skip the incompatible legacy reload/render/write
self-test.

Measured peak RSS against the recorded Phase 0 baseline:

| Fixture | Phase 0 | Phase 3 | Reduction |
| --- | ---: | ---: | ---: |
| Ashford | 76,416 KiB | 46,784 KiB | 38.8% |
| Canterbury | 54,560 KiB | 31,792 KiB | 41.7% |
| Maidstone | 55,344 KiB | 36,896 KiB | 33.3% |

Run the complete gate with:

```sh
./tests/run_phase3.sh
```

It builds from clean sources, checks the CLI and negative paths, performs fresh
PNG extraction for all three towns, validates every result through Phase 2,
proves work-directory isolation and diagnostic retention with a controlled
engine, and requires at least a 20% peak-RSS reduction when the Phase 0 metrics
are present.
