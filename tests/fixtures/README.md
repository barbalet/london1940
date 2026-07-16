# Migration fixtures

`legacy-keyed-canterbury.json` is genuine output from the repository's legacy C
serializer (`save_map_json`), not a hand-written approximation. It was generated
from `examples/new_canterbury.png` using the current source with one test-only
change: `MAX_TILE_WIDTH` and `MAX_TILE_HEIGHT` were raised to 3000 in a temporary
copy of `map2json.h`. This routes the 1648 by 1539 example through the ordinary
non-ApeSDK extractor instead of the large-sheet tiler; it does not change
extraction or JSON serialization.

Generation used Git revision
`a15b9f338fc970e6ca128bbf35a555908725d261`. The fixture SHA-256 is
`f52d83e15350eae3e133a9f7a8d4c50ae84cfdf26b35d56b7960229cb934ec37`.
It contains the writer's keyed `perimeter` terrain polygons and keyed `ext`/`int`
building polygons, plus every legacy point and network layer. It is intentionally
retained exactly as emitted, including legacy formatting and quirks.
