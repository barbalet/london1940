#!/usr/bin/env python3

import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

from jsonschema import Draft202012Validator

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("map_migrate", ROOT / "tools" / "map_migrate.py")
MAP_MIGRATE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MAP_MIGRATE)
VALIDATE_SPEC = importlib.util.spec_from_file_location("map_validate", ROOT / "tools" / "map_validate.py")
MAP_VALIDATE = importlib.util.module_from_spec(VALIDATE_SPEC)
assert VALIDATE_SPEC.loader is not None
VALIDATE_SPEC.loader.exec_module(MAP_VALIDATE)


def legacy_counts(document):
    counts = {}
    for category in ("terrain", "urban"):
        for layer in document[category]:
            identifier = MAP_MIGRATE.layer_id(category, layer["type"])
            if "polygons" in layer:
                polygons = layer["polygons"]
                if isinstance(polygons, list):
                    counts[identifier] = {"polygons": len(polygons), "rings": len(polygons)}
                else:
                    ring_count = sum(1 + (1 if "int" in polygon else 0) for polygon in polygons.values())
                    counts[identifier] = {"polygons": len(polygons), "rings": ring_count}
            elif "pointdirections" in layer:
                counts[identifier] = {"items": len(layer["pointdirections"])}
            elif "links" in layer:
                value = {"points": len(layer["points"]), "links": len(layer["links"])}
                if "widths" in layer:
                    value["widths"] = len(layer["widths"])
                counts[identifier] = value
            else:
                counts[identifier] = {"points": len(layer.get("points", []))}
    return counts


class MigrationTests(unittest.TestCase):
    def test_all_examples_validate_and_preserve_counts(self):
        schema = json.loads((ROOT / "schema" / "map-v1.schema.json").read_text())
        Draft202012Validator.check_schema(schema)
        schema_validator = Draft202012Validator(schema)
        for path in sorted((ROOT / "examples").glob("*.json")):
            with self.subTest(path=path.name):
                legacy = json.loads(path.read_text())
                migrated = MAP_MIGRATE.migrate_legacy(legacy, ROOT / "examples")
                MAP_MIGRATE.validate_v1(migrated)
                schema_validator.validate(migrated)
                self.assertEqual(legacy_counts(legacy), MAP_MIGRATE.feature_counts(migrated))
                first = MAP_MIGRATE.canonical_bytes(migrated)
                reloaded = json.loads(first)
                MAP_MIGRATE.validate_v1(reloaded)
                self.assertEqual(MAP_MIGRATE.feature_counts(migrated), MAP_MIGRATE.feature_counts(reloaded))
                self.assertEqual(first, MAP_MIGRATE.canonical_bytes(reloaded))
                self.assertRegex(migrated["source"]["sha256"], r"^[0-9a-f]{64}$")

    def test_real_keyed_writer_fixture(self):
        fixture = ROOT / "tests" / "fixtures" / "legacy-keyed-canterbury.json"
        self.assertEqual(
            "f52d83e15350eae3e133a9f7a8d4c50ae84cfdf26b35d56b7960229cb934ec37",
            MAP_MIGRATE.sha256_file(fixture),
        )
        legacy = json.loads(fixture.read_text())
        migrated = MAP_MIGRATE.migrate_legacy(legacy, ROOT / "examples")
        MAP_MIGRATE.validate_v1(migrated)
        schema = json.loads((ROOT / "schema" / "map-v1.schema.json").read_text())
        Draft202012Validator(schema).validate(migrated)
        self.assertEqual("keyed-object", migrated["source"]["legacyFormat"])
        self.assertEqual(legacy_counts(legacy), MAP_MIGRATE.feature_counts(migrated))
        buildings = next(layer for layer in migrated["layers"] if layer["id"] == "urban.buildings")
        self.assertTrue(any(len(polygon["rings"]) > 1 for polygon in buildings["polygons"]))
        topology = MAP_VALIDATE.semantic_invariants(migrated)["urban.buildings"]
        self.assertEqual(83, topology["interiorRings"])
        self.assertEqual(38, topology["misplacedInteriorRings"])
        self.assertEqual(45, topology["quarantinedRings"])
        first = MAP_MIGRATE.canonical_bytes(migrated)
        reloaded = json.loads(first)
        Draft202012Validator(schema).validate(reloaded)
        self.assertEqual(first, MAP_MIGRATE.canonical_bytes(reloaded))

    def test_invalid_network_index_is_rejected(self):
        legacy = {
            "sourceFilename": "missing.png", "tile": [0, 0], "offset": [0, 0], "resolution": [10, 10],
            "terrain": [{"type": "rivers", "points": [[1, 1]], "widths": [1], "links": [[2, -1]]}],
            "urban": [],
        }
        with self.assertRaises(MAP_MIGRATE.MapValidationError):
            MAP_MIGRATE.migrate_legacy(legacy)

    def test_schema_is_well_formed_json(self):
        schema = json.loads((ROOT / "schema" / "map-v1.schema.json").read_text())
        Draft202012Validator.check_schema(schema)
        self.assertEqual("https://json-schema.org/draft/2020-12/schema", schema["$schema"])
        self.assertEqual("london1940-map", schema["properties"]["format"]["const"])


class ValidationHarnessTests(unittest.TestCase):
    schema_path = ROOT / "schema" / "map-v1.schema.json"
    expectations_path = ROOT / "tests" / "fixtures" / "mask-expectations.json"

    def migrate_example(self, name):
        path = ROOT / "examples" / f"{name}.json"
        return MAP_MIGRATE.migrate_legacy(json.loads(path.read_text()), ROOT / "examples")

    def test_examples_match_reviewed_masks_and_round_trip(self):
        with tempfile.TemporaryDirectory() as temporary:
            temporary_path = Path(temporary)
            for name in ("new_ashford", "new_canterbury", "new_maidstone"):
                with self.subTest(name=name):
                    document = self.migrate_example(name)
                    input_path = temporary_path / f"{name}.v1.json"
                    input_path.write_bytes(MAP_MIGRATE.canonical_bytes(document))
                    report = MAP_VALIDATE.validate_map(
                        input_path,
                        self.schema_path,
                        temporary_path / name,
                        self.expectations_path,
                    )
                    self.assertTrue(report["valid"])
                    self.assertEqual(len(document["layers"]), len(report["layers"]))
                    self.assertTrue(all(layer["metrics"]["exact"] for layer in report["layers"].values()))
                    compressed = (temporary_path / name / "map.canonical.json.gz").read_bytes()
                    self.assertEqual(compressed, MAP_VALIDATE.deterministic_gzip(MAP_MIGRATE.canonical_bytes(document)))

    def test_zero_feature_regression_fails(self):
        document = self.migrate_example("new_ashford")
        buildings = next(layer for layer in document["layers"] if layer["id"] == "urban.buildings")
        buildings["polygons"] = []
        with tempfile.TemporaryDirectory() as temporary:
            input_path = Path(temporary) / "new_ashford.v1.json"
            input_path.write_bytes(MAP_MIGRATE.canonical_bytes(document))
            with self.assertRaisesRegex(MAP_VALIDATE.HarnessError, "fell below expected minimum"):
                MAP_VALIDATE.validate_map(input_path, self.schema_path, Path(temporary) / "out", self.expectations_path)

    def test_malformed_and_incompatible_json_fail(self):
        with tempfile.TemporaryDirectory() as temporary:
            temporary_path = Path(temporary)
            truncated = temporary_path / "truncated.v1.json"
            truncated.write_text('{"format":"london1940-map"')
            with self.assertRaisesRegex(MAP_VALIDATE.HarnessError, "invalid JSON"):
                MAP_VALIDATE.validate_map(truncated, self.schema_path, temporary_path / "truncated", None)
            incompatible = self.migrate_example("new_ashford")
            incompatible["version"] = 99
            incompatible_path = temporary_path / "incompatible.v1.json"
            incompatible_path.write_bytes(MAP_MIGRATE.canonical_bytes(incompatible))
            with self.assertRaisesRegex(MAP_VALIDATE.HarnessError, "schema validation failed"):
                MAP_VALIDATE.validate_map(incompatible_path, self.schema_path, temporary_path / "incompatible", None)

    def test_png_validation_rejects_malformed_and_truncated_files(self):
        self.assertEqual((2174, 1754), MAP_VALIDATE.validate_png(ROOT / "examples" / "new_ashford.png"))
        with tempfile.TemporaryDirectory() as temporary:
            malformed = Path(temporary) / "bad.png"
            malformed.write_bytes(b"not a png")
            with self.assertRaises(MAP_VALIDATE.HarnessError):
                MAP_VALIDATE.validate_png(malformed)
            truncated = Path(temporary) / "truncated.png"
            truncated.write_bytes((ROOT / "examples" / "new_ashford.png").read_bytes()[:40])
            with self.assertRaises(MAP_VALIDATE.HarnessError):
                MAP_VALIDATE.validate_png(truncated)

    def test_mask_metrics_report_tolerant_comparison(self):
        metrics = MAP_VALIDATE.mask_metrics(bytes([0, 255, 255, 0]), bytes([0, 255, 0, 255]))
        self.assertFalse(metrics["exact"])
        self.assertEqual(1, metrics["truePositive"])
        self.assertEqual(0.5, metrics["precision"])
        self.assertEqual(0.5, metrics["recall"])
        self.assertAlmostEqual(1 / 3, metrics["intersectionOverUnion"])


if __name__ == "__main__":
    unittest.main()
