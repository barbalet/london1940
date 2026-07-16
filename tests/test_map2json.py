#!/usr/bin/env python3

import importlib.util
import tempfile
import unittest
from argparse import Namespace
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("map2json_driver", ROOT / "tools" / "map2json.py")
DRIVER = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(DRIVER)


class DriverTests(unittest.TestCase):
    def fake_engine(self, directory: Path, exit_code: int = 0, produce_output: bool = True) -> Path:
        engine = directory / "fake-engine"
        legacy = ROOT / "examples" / "new_ashford.json"
        lines = ["#!/bin/sh", "printf diagnostic > engine-diagnostic.txt"]
        if produce_output:
            lines.append(f"cp '{legacy}' map.json")
        lines.append(f"exit {exit_code}")
        engine.write_text("\n".join(lines) + "\n")
        engine.chmod(0o755)
        return engine

    def arguments(self, directory: Path, engine: Path, diagnostics: bool) -> Namespace:
        return Namespace(
            input=ROOT / "examples" / "new_ashford.png", filename=None,
            output=directory / "requested.json", work_dir=directory / "work",
            diagnostics=diagnostics, profile="legacy-1", format_version=1,
            metadata=None, engine=engine,
        )

    def test_exact_requested_output_and_clean_normal_work_directory(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            args = self.arguments(directory, self.fake_engine(directory), False)
            output = DRIVER.run_pipeline(args)
            self.assertEqual(args.output.resolve(), output)
            self.assertTrue(output.is_file())
            self.assertEqual([], list(args.work_dir.iterdir()))
            self.assertFalse((directory / "map.json").exists())

    def test_diagnostics_retain_isolated_engine_artifacts(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            args = self.arguments(directory, self.fake_engine(directory), True)
            DRIVER.run_pipeline(args)
            runs = list(args.work_dir.iterdir())
            self.assertEqual(1, len(runs))
            self.assertTrue((runs[0] / "extractor.log").is_file())
            self.assertTrue((runs[0] / "engine-diagnostic.txt").is_file())
            self.assertTrue((runs[0] / "map.json").is_file())

    def test_engine_failure_and_missing_output_are_nonzero_conditions(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            with self.assertRaisesRegex(DRIVER.PipelineError, "status 7"):
                DRIVER.run_pipeline(self.arguments(directory, self.fake_engine(directory, 7), False))
            missing = self.fake_engine(directory, 0, False)
            with self.assertRaisesRegex(DRIVER.PipelineError, "without producing map.json"):
                DRIVER.run_pipeline(self.arguments(directory, missing, False))

    def test_checked_inputs_and_scaled_limits(self):
        args = Namespace(input=None, filename=None)
        with self.assertRaisesRegex(DRIVER.PipelineError, "required"):
            DRIVER.checked_input(args)
        limits = DRIVER.extraction_limits(2174, 1754)
        self.assertLess(int(limits[1]), 10_485_760)
        self.assertLess(int(limits[3]), 2_621_440)
        release, release_arguments = DRIVER.load_profile("legacy-1")
        sensitivity, _ = DRIVER.load_profile("sensitivity-woodland-1")
        changed = {key for key in release["parameters"] if release["parameters"][key] != sensitivity["parameters"][key]}
        self.assertEqual({"woodlandThreshold"}, changed)
        self.assertEqual(len(DRIVER.PROFILE_FLAGS) * 2, len(release_arguments))

    def test_explicit_metadata_overrides_filename_georeferencing(self):
        document = {"tile": [0, 0], "coordinates": {"crs": None}}
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "metadata.json"
            path.write_text('{"tile":[160,0],"geographicBounds":{"crs":"EPSG:4326",'
                            '"topLeft":{"longitude":0.0,"latitude":52.0},'
                            '"bottomRight":{"longitude":1.0,"latitude":51.0}}}')
            DRIVER.apply_metadata(document, path)
        self.assertEqual([160, 0], document["tile"])
        self.assertEqual("EPSG:4326", document["coordinates"]["crs"])


if __name__ == "__main__":
    unittest.main()
