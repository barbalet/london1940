# Component support status

The maintained product in this repository is the historical-map ingestion
pipeline: `map/`, `tools/`, `schema/`, `baseline/`, and their tests and
documentation. These paths form the supported fresh-clone build.

The following research material is deliberately retained but archived:

- `urban/` is an unrelated earlier “Mushroom Boy” simulation prototype. It is
  not loaded by London 1940 and is not part of supported builds or CI.
- `londonmap/` is a fixed-dimension four-sheet colour experiment. Its local
  paths were removed and its command now accepts input/output arguments, but its
  extraction method is not the maintained map pipeline.
- `occupations/` is historical taxonomy source material. It has no supported
  executable or validated integration and is retained as data for possible
  later provenance work.
- `mapedit/` is a legacy renderer/editor. It is not a supported build because
  its data model predates format v1.

The archived C prototypes reference ApeSDK. `tools/fetch_apesdk.py` can obtain
the exact historical revision in `dependencies/apesdk.lock.json` at the
repository-local `apesdk/` path expected by those sources. Fetching it does not
promote those components into the supported build; any revival requires its own
compatibility tests and an explicit status change here.

Legacy orchestration commands `map/allmaps`, `map/generate.sh`, and
`map/allyouneed.sh` now fail with an archival explanation rather than silently
running incompatible formats. `map/mapoutdir` remains supported as a safe batch
front end for independent PNG-to-v1 conversions.
