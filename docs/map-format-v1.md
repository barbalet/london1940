# London 1940 map format version 1

The normative structural definition is
[`schema/map-v1.schema.json`](../schema/map-v1.schema.json). This document
defines semantics that JSON Schema cannot express completely.

## Design decisions

- Version 1 is a semantic interchange format, not a raster archive.
- Coordinates are signed integers stored as absolute sheet-pixel positions.
  They include the legacy offset exactly once during migration.
- Pixel origin is top-left; positive x points right and positive y points down.
- `tile` identifies the source tile and `offset` locates its local origin in the
  sheet coordinate space. `resolution` remains the tile's pixel dimensions.
- Version 1 deliberately has no shared string or coordinate tables. Readability
  and reviewability are more important while legacy data is being repaired.
  Deterministic gzip supplies transport compression. A later schema version may
  add delta coordinates or tables only with measured benefit and a migration.
- Layer order is preserved from legacy input. Object keys are emitted in sorted
  order and JSON is serialised without insignificant whitespace, followed by
  one newline. This is the canonical byte representation.

## Polygon rules

All polygon layers use `polygons[].rings`. The first ring is the exterior and
subsequent rings are holes. A ring contains at least three coordinate pairs.
Rings are implicitly closed: consumers connect the last point to the first and
writers must not append a closing duplicate solely for closure. Existing
consecutive duplicates are retained by migration because Phase 1 must not alter
legacy geometry. Ring winding is not normative in version 1; consumers must use
ring position rather than winding to distinguish exteriors and holes.

The ApeSDK array dialect has no hole ownership, so every legacy array entry
becomes one polygon with one exterior ring. The keyed-object dialect maps `ext`
to the exterior and `int` to a hole; `perimeter` maps to an exterior. A keyed
legacy identifier is retained as `legacyId` but has no spatial meaning.

## Networks and points

Point layers contain `points`. Network layers contain equally indexed `points`
and `links`; rivers additionally contain `widths`, with one width per point.
Each link is a pair of zero-based point indices. `-1` is the only sentinel and
means no connection at that endpoint. Other negative values are invalid, as
are indices beyond the layer's point array. Legacy data is validated against
these rules during migration.

Bridge `pointdirections` become objects with a `point` and a direction vector.
Direction values are fixed-point legacy vectors and are intentionally not
normalised during migration.

## Coordinates and georeferencing

The examples contain no reliable geographic coordinates, so their `crs` is
`null`. A non-null CRS describes the sheet-pixel mapping, not the pixel values
themselves. Legacy `geocoords` are preserved as optional WGS84
`geographicBounds`; zero-valued legacy bounds are omitted because they mean
unknown rather than a real location in the Gulf of Guinea.

## Source identity

`source.filename` is required. `source.sha256` is the digest of the source PNG
when it can be resolved through `--source-root`; otherwise it is `null`.
`source.legacyFormat` records whether the input used ApeSDK arrays, keyed
objects, or was already v1.

`generator` records the converter name, version, and extraction profile. Legacy
documents do not contain these values, so migration records a null version and
the explicit profile `legacy-default-unknown` rather than inventing provenance.
`attribution` is mandatory and carries the dataset license and human-readable
historical-map credit into derived artifacts.

## Canonical command

```sh
python3 tools/map_migrate.py migrate examples/new_ashford.json \
  --source-root examples --output build/new_ashford.v1.json
python3 tools/map_migrate.py validate build/new_ashford.v1.json
```

Running `canonicalize` on a valid v1 document performs validation and rewrites
the canonical bytes. Repeating it must be byte-identical.

The repository's semantic validator is checked against an independent
implementation of JSON Schema Draft 2020-12 in the test suite. Install the test
dependency and run:

```sh
python3 -m pip install -r requirements-test.txt
python3 -m unittest discover -s tests -v
```
