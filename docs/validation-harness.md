# Phase 2 validation harness

The validation harness treats format correctness, compression correctness, and
extraction regression as separate checks. It does not claim that the legacy
extractor identified every historical feature correctly.

## Run everything

```sh
python3 -m pip install -r requirements-test.txt
./tests/run_phase2.sh
```

The script performs two clean runs for Ashford, Canterbury, and Maidstone. Each
run migrates legacy JSON, independently validates the Draft 2020-12 schema,
checks semantic invariants, creates canonical JSON and deterministic gzip,
renders every layer mask, compares reviewed expectations, and writes a report.
It then compares both output trees byte-for-byte and runs all unit and negative
tests. Outputs are isolated under ignored `build/phase2`.

## Validation layers

1. `jsonschema` independently validates the normative schema and document.
2. `map_migrate.validate_v1` checks the same format contract without relying on
   the external validator.
3. The harness checks coordinate bounds, unique canonical layer identifiers,
   polygon ring structure and interior-ring ownership, link ranges, one link
   pair per network point, and one river width per river point.
4. Canonical JSON is decoded and compared deeply with its source data model,
   then encoded again and compared byte-for-byte.
5. Gzip uses compression level 9, timestamp zero, and no source filename. Both
   decompression equality and repeated compressed-byte equality are required.
6. PNG carriers store deterministic gzipped canonical JSON in a private `loNg`
   chunk with byte counts and SHA-256 checksums; extraction must reproduce the
   canonical JSON bytes exactly.

## PNG carriers

`tools/json2png.py` writes a viewable semantic RGB preview of a v1 map and
embeds the canonical JSON payload. The visible PNG is a map preview, not a
bit-perfect reconstruction of the source scan. `tools/png2json.py` verifies the
PNG structure, carrier metadata, gzip stream, byte counts, and checksums before
writing JSON.

```sh
python3 tools/json2png.py build/map.v1.json --output build/map.carrier.png
python3 tools/png2json.py build/map.carrier.png --output build/map.roundtrip.v1.json
cmp build/map.v1.json build/map.roundtrip.v1.json
```

The carrier code is covered by unit tests for exact JSON round-trip, preview
pixel stability, CLI wrappers, non-canonical JSON rejection, and corrupt or
missing carrier chunks.

## Masks and expectations

Each v1 layer is rendered to a deterministic 8-bit grayscale PNG. Polygon
exteriors are filled and interior rings erased; indexed network edges are
rasterised once; point and bridge layers use fixed-radius marks. The renderer is
a regression reference, not the simulation renderer.

`tests/fixtures/mask-expectations.json` records reviewed raw-pixel mask hashes and
minimum feature counts for every layer in all three examples. All 27 masks were
visually inspected as per-town contact sheets on 2026-07-15. Independently
stored reference PNGs are decoded and compared to newly rendered raw pixels;
the expected hash is a second pinned check. A missing layer, zeroed feature
class, reduced count, or changed raster fails validation.

Legacy polygon data contains rings with fewer than three distinct points,
zero area, and interior rings outside their declared exterior. Migration
preserves those records for Phase 1 feature-count fidelity; Phase 2 explicitly
quarantines them, records per-layer topology counts, excludes them from masks,
and pins the reviewed counts. They are therefore visible data defects rather
than silently accepted geometry.

Raw pixels rather than compressed PNG bytes are hashed so expectations remain
stable across zlib implementations. Exact hashes are used for stable fixtures.
`compare-raw-masks` also reports true
positive, false positive, false negative, precision, recall, and intersection
over union for future reviewed changes where exact equality is inappropriate.

## Individual commands

```sh
python3 tools/map_validate.py validate-map build/map.v1.json \
  --output-dir build/validation \
  --expectations tests/fixtures/mask-expectations.json
python3 tools/map_validate.py validate-png examples/new_ashford.png
python3 tools/map_validate.py compare-raw-masks actual.raw expected.raw
```

The test suite includes truncated JSON, incompatible schema version, malformed
PNG, truncated PNG, invalid network indices, misplaced legacy interior rings,
non-exact mask metrics, and a deliberately empty building layer. Every failure
must produce a non-zero CLI exit or a harness exception.
