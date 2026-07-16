#!/usr/bin/env python3

"""Download source sheets from a pinned GitLab commit and verify Git blob IDs."""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_MANIFEST = ROOT / "data" / "source-maps.json"


class AcquisitionError(RuntimeError):
    pass


def git_blob_sha1(path: Path) -> str:
    size = path.stat().st_size
    digest = hashlib.sha1(f"blob {size}\0".encode(), usedforsecurity=False)
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def verify(path: Path, expected: str) -> None:
    actual = git_blob_sha1(path)
    if actual != expected:
        raise AcquisitionError(f"checksum mismatch for {path}: expected {expected}, found {actual}")


def source_url(manifest: dict, entry: dict) -> str:
    repository = manifest["repository"]
    revision = manifest["revision"]
    encoded_path = urllib.parse.quote(entry["path"], safe="/")
    return f"{repository}/-/raw/{revision}/{encoded_path}?inline=false"


def download(url: str, destination: Path) -> None:
    temporary = destination.with_name(f".{destination.name}.part")
    temporary.unlink(missing_ok=True)
    try:
        request = urllib.request.Request(url, headers={"User-Agent": "london1940-source-fetch/1"})
        with urllib.request.urlopen(request) as response, temporary.open("wb") as output:
            while block := response.read(1024 * 1024):
                output.write(block)
        temporary.replace(destination)
    finally:
        temporary.unlink(missing_ok=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--sheet", action="append", help="sheet number; repeat or omit for all")
    parser.add_argument("--verify-only", action="store_true")
    args = parser.parse_args()
    try:
        manifest = json.loads(args.manifest.read_text())
        selected = args.sheet or sorted(manifest["files"])
        unknown = set(selected) - set(manifest["files"])
        if unknown:
            raise AcquisitionError(f"unknown sheets: {sorted(unknown)}")
        args.output_dir.mkdir(parents=True, exist_ok=True)
        for sheet in selected:
            entry = manifest["files"][sheet]
            destination = args.output_dir / entry["path"]
            if not destination.is_file():
                if args.verify_only:
                    raise AcquisitionError(f"missing source sheet: {destination}")
                print(f"downloading sheet {sheet} from pinned revision {manifest['revision']}")
                download(source_url(manifest, entry), destination)
            verify(destination, entry["gitBlobSha1"])
            print(f"verified sheet {sheet}: {entry['gitBlobSha1']}")
    except (OSError, KeyError, json.JSONDecodeError, urllib.error.URLError, AcquisitionError) as error:
        print(f"fetch_source_maps: error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
