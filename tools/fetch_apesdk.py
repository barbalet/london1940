#!/usr/bin/env python3

"""Fetch the exact optional ApeSDK revision recorded by the dependency lock."""

import argparse
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LOCK = ROOT / "dependencies" / "apesdk.lock.json"


def run(command: list[str], cwd: Path | None = None) -> str:
    completed = subprocess.run(command, cwd=cwd, text=True, capture_output=True)
    if completed.returncode:
        raise RuntimeError(completed.stderr.strip() or completed.stdout.strip())
    return completed.stdout.strip()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--destination", type=Path, default=ROOT / "apesdk")
    parser.add_argument("--verify-only", action="store_true")
    args = parser.parse_args()
    lock = json.loads(LOCK.read_text())
    destination = args.destination.resolve()
    try:
        if not destination.exists():
            if args.verify_only:
                raise RuntimeError(f"dependency is absent: {destination}")
            destination.parent.mkdir(parents=True, exist_ok=True)
            run(["git", "clone", "--no-checkout", lock["url"], str(destination)])
        if not (destination / ".git").exists():
            raise RuntimeError(f"destination is not a Git checkout: {destination}")
        if not args.verify_only:
            run(["git", "fetch", "origin", lock["revision"]], destination)
            run(["git", "checkout", "--detach", lock["revision"]], destination)
        actual = run(["git", "rev-parse", "HEAD"], destination)
        if actual != lock["revision"]:
            raise RuntimeError(f"ApeSDK revision mismatch: expected {lock['revision']}, found {actual}")
    except (OSError, RuntimeError) as error:
        print(f"fetch_apesdk: error: {error}", file=sys.stderr)
        return 1
    print(f"verified {lock['name']} {actual} at {destination}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
