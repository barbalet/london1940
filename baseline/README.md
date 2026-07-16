# Legacy baseline fixtures

This directory defines the Phase 0 inputs and the command used to preserve the
legacy `map2json` behaviour before its format and implementation are repaired.
The generated baseline is evidence of current behaviour, not a declaration that
the extraction is correct.

Run from any directory:

```sh
./baseline/capture.sh
```

The command verifies input hashes, builds the current extractor, converts all
three examples, records logs, elapsed time, sampled peak resident memory,
feature counts, JSON, semantic renders, diagnostic masks, and artifact hashes.
It writes to `build/phase0-baseline`, which is ignored by Git. The command
refuses to overwrite an existing capture; remove it deliberately or pass a new
output directory:

```sh
./baseline/capture.sh build/phase0-baseline-second-run
```

`fixtures.json` is the tracked manifest of source inputs, dimensions, depths,
and SHA-256 digests. Checked-in legacy JSON hashes are recorded for provenance,
but the capture regenerates JSON from the PNG rather than trusting those files.

The capture uses the examples' legacy `--apesdk` path. Without that flag,
`map2json` rejects these fixtures because its large-map path requires dimensions
of at least 10,000 by 10,000 pixels. The resulting format incompatibilities are
part of the behaviour Phase 0 is intended to preserve.

Peak RSS, elapsed wall time, user CPU time, and system CPU time are captured
through the operating system's child-process resource accounting. RSS is
normalised to KiB on macOS and Linux.
