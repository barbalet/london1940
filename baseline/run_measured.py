#!/usr/bin/env python3

"""Run one command and record portable elapsed-time and peak-RSS metrics."""

import argparse
import json
import resource
import subprocess
import sys
import time


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cwd", required=True)
    parser.add_argument("--log", required=True)
    parser.add_argument("--metrics", required=True)
    parser.add_argument("command", nargs=argparse.REMAINDER)
    args = parser.parse_args()

    if not args.command:
        parser.error("a command is required")

    before = resource.getrusage(resource.RUSAGE_CHILDREN)
    started = time.monotonic()
    with open(args.log, "w", encoding="utf-8") as log_file:
        completed = subprocess.run(
            args.command,
            cwd=args.cwd,
            stdout=log_file,
            stderr=subprocess.STDOUT,
            check=False,
        )
    elapsed = time.monotonic() - started
    after = resource.getrusage(resource.RUSAGE_CHILDREN)

    # macOS reports bytes; Linux and the BSDs report KiB.
    peak_rss_kb = after.ru_maxrss
    if sys.platform == "darwin":
        peak_rss_kb //= 1024

    metrics = {
        "elapsedSeconds": round(elapsed, 3),
        "peakRssKb": int(peak_rss_kb),
        "userCpuSeconds": round(after.ru_utime - before.ru_utime, 3),
        "systemCpuSeconds": round(after.ru_stime - before.ru_stime, 3),
        "exitCode": completed.returncode,
    }
    with open(args.metrics, "w", encoding="utf-8") as metrics_file:
        json.dump(metrics, metrics_file, sort_keys=True, indent=2)
        metrics_file.write("\n")

    return completed.returncode


if __name__ == "__main__":
    raise SystemExit(main())
