#!/usr/bin/env python3

import array
import importlib.util
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("heightmap", ROOT / "map" / "heightmap.py")
HEIGHTMAP = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(HEIGHTMAP)


class HeightmapTests(unittest.TestCase):
    def fixture(self, directory: Path) -> Path:
        text = (ROOT / "tests" / "fixtures" / "elevation-small.dem.hex").read_text().splitlines()[-1]
        path = directory / "small.dem"
        path.write_bytes(bytes.fromhex(text))
        return path

    def test_fixture_validation_and_extraction(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = self.fixture(Path(temporary))
            values = HEIGHTMAP.load_dem(path, 4, 3)
            result = HEIGHTMAP.extract(values, 4, 3, (1, 0, 4, 2), 160)
        self.assertEqual(3, result["width"])
        self.assertEqual([[20, None, 40], [60, 70, 80]], result["elevation"])

    def test_all_nodata_and_wrong_size_fail(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            wrong = directory / "wrong.dem"
            wrong.write_bytes(b"\0")
            with self.assertRaisesRegex(HEIGHTMAP.HeightmapError, "expected 24"):
                HEIGHTMAP.load_dem(wrong, 4, 3)
            nodata = directory / "nodata.dem"
            values = array.array("h", [HEIGHTMAP.NO_DATA] * 12)
            if sys.byteorder == "little":
                values.byteswap()
            nodata.write_bytes(values.tobytes())
            with self.assertRaisesRegex(HEIGHTMAP.HeightmapError, "only the NODATA"):
                HEIGHTMAP.load_dem(nodata, 4, 3)

    def test_cli_emits_valid_json_and_nonzero_errors(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            self.fixture(directory)
            base = [sys.executable, str(ROOT / "map" / "heightmap.py"), "--dir", str(directory),
                       "--dem-filename", "small.dem", "--image-width", "4", "--image-height", "3",
                       "--pixel-bounds"]
            command = base + ["0", "0", "4", "3"]
            completed = subprocess.run(command, text=True, capture_output=True)
            self.assertEqual(0, completed.returncode, completed.stderr)
            self.assertEqual(12, sum(len(row) for row in json.loads(completed.stdout)["elevation"]))
            failed = subprocess.run(base + ["0", "0", "5", "3"],
                                    text=True, capture_output=True)
            self.assertNotEqual(0, failed.returncode)


if __name__ == "__main__":
    unittest.main()
