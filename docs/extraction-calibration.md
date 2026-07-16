# Phase 5 extraction calibration

## Acceptance scope

The release extraction profile is `profiles/legacy-1.json`. It names the
calibration values that affect the maintained three-town path: buildings,
woodland, rivers, main and minor roads, orchards, stations, and sea-area
classification. `map2json` loads this tracked profile for every conversion and
records its name in v1 provenance. Missing, extra, non-integer, or malformed
profile parameters fail before extraction.

Ashford, Canterbury, and Maidstone contain nine observable output classes and
27 reviewed masks. `tests/fixtures/quality-thresholds.json` requires precision,
recall, and intersection-over-union of 1.0 against those accepted masks. This
is deliberately a strict release-regression threshold: it means the reviewed
output did not change. It is not a statistical estimate against independently
surveyed 1940 ground truth.

The town sheets do not exercise lakes, sea, sand, harbours, railway lines, or
railway tunnels. Phase 5 provides synthetic geometry/mask coverage for those
six types so serialization and rendering cannot disappear unnoticed. Real-map
quality acceptance for them requires the full sheets in Phase 6 and is not
silently inferred from synthetic shapes.

## Known symbol ambiguity

- Building ink merges on dense blocks; courtyards, labels, and hatching can be
  mistaken for interiors or split one footprint into several polygons.
- Woodland and orchard stippling varies with print density, ageing, scan colour,
  and overprinted text. Orchard points are especially sparse classes.
- Main/minor road colours overlap faded background and boundary ink. Junctions
  are derived from network topology, so one broken segment affects both classes.
- Rivers share blue-green tones with wash, ponds, and scan discolouration;
  inferred widths are not surveyed channel widths.
- Bridge and station marks are tiny and can be confused with lettering.
- Railway tunnels, harbours, sand, lakes, and sea require full-sheet reference
  review because the three town crops do not contain accepted examples.

Quality reports remain per class so abundant roads cannot hide lost buildings
or sparse point features. Exact hashes are appropriate for the stable reviewed
fixtures; future deliberate simplification may lower an individual threshold
only with a newly reviewed reference and a documented rationale.

## Boundary and polygon rules

The Phase 5 overlap fixture describes the same road in two overlapping tiles.
Merging uses sheet-global point coordinates, removes duplicate overlap points
and edges, and must retain one connected four-point chain. Polygon regression
tests remove only collinear vertices, reject invalid rings, retain small
buildings, preserve interior ownership, and require identical raster coverage
before and after simplification.

## Sensitivity profile

`profiles/sensitivity-woodland-1.json` is not a release profile. It changes only
the woodland threshold from 30 to 35. On Ashford it changes only the woodland
mask: precision 0.9366, recall 0.9657, and IoU 0.9064 relative to `legacy-1`.
`tools/map_quality.py compare-profiles` produces the review artifact rather than
allowing a threshold change to overwrite accepted fixtures implicitly.

## Run the gate

```sh
./tests/run_phase5.sh
```

The gate performs fresh release-profile extraction and Phase 2 validation for
all three towns, measures runtime and peak RSS, evaluates all real-map classes
against the policy, extracts Ashford with the sensitivity profile, requires a
woodland-only difference, and runs boundary, missing-class, and polygon tests.
No optimisation is justified by this phase; the measurements are retained so a
future optimisation can be assessed after correctness.
