#!/usr/bin/env python3

import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("map_quality", ROOT / "tools" / "map_quality.py")
QUALITY = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(QUALITY)


class QualityTests(unittest.TestCase):
    def test_overlap_networks_remain_connected_without_duplicate_points(self):
        base = {"id": "urban.main-roads", "category": "urban", "type": "main roads", "geometry": "network"}
        left = {**base, "points": [[0, 5], [5, 5], [10, 5]], "links": [[1, -1], [0, 2], [1, -1]]}
        right = {**base, "points": [[5, 5], [10, 5], [15, 5]], "links": [[1, -1], [0, 2], [1, -1]]}
        merged = QUALITY.merge_networks([left, right])
        self.assertEqual([[0, 5], [5, 5], [10, 5], [15, 5]], merged["points"])
        self.assertEqual([[1, -1], [0, 2], [1, 3], [2, -1]], merged["links"])

    def test_simplification_preserves_small_building_and_interior_ring(self):
        exterior = [[1, 1], [5, 1], [10, 1], [10, 10], [1, 10], [1, 5]]
        interior = [[4, 4], [7, 4], [7, 7], [4, 7]]
        simplified = QUALITY.simplify_ring(exterior)
        self.assertEqual(4, len(simplified))
        self.assertIsNone(QUALITY.MAP_VALIDATE.ring_issue(simplified))
        self.assertTrue(QUALITY.MAP_VALIDATE.point_in_ring(interior[0], simplified))
        original_layer = {"geometry": "polygons", "polygons": [{"rings": [exterior, interior]}]}
        simple_layer = {"geometry": "polygons", "polygons": [{"rings": [simplified, interior]}]}
        original = QUALITY.MAP_VALIDATE.render_layer(original_layer, 12, 12, [0, 0])
        reduced = QUALITY.MAP_VALIDATE.render_layer(simple_layer, 12, 12, [0, 0])
        self.assertEqual(original, reduced)
        self.assertEqual(0, reduced[5 * 12 + 5])
        self.assertGreater(sum(value != 0 for value in reduced), 0)

    def test_classes_absent_from_town_sheets_have_synthetic_mask_coverage(self):
        polygon = {"geometry": "polygons", "polygons": [{"rings": [[[1, 1], [8, 1], [8, 8], [1, 8]]]}]}
        network = {"geometry": "network", "points": [[1, 1], [8, 8]], "links": [[1, -1], [0, -1]]}
        fixtures = {
            "terrain.lakes": polygon, "terrain.sands": polygon, "terrain.sea": polygon,
            "urban.harbours": network, "urban.railway-lines": network,
            "urban.railway-tunnels": network,
        }
        policy = json.loads((ROOT / "tests/fixtures/quality-thresholds.json").read_text())
        self.assertEqual(set(policy["syntheticOnlyClasses"]), set(fixtures))
        for identifier, layer in fixtures.items():
            with self.subTest(identifier=identifier):
                mask = QUALITY.MAP_VALIDATE.render_layer(layer, 10, 10, [0, 0])
                self.assertGreater(sum(value != 0 for value in mask), 0)

    def test_quality_policy_requires_every_reviewed_real_class(self):
        policy_path = ROOT / "tests/fixtures/quality-thresholds.json"
        policy = json.loads(policy_path.read_text())
        layers = {}
        for identifier in policy["realMapClasses"]:
            layers[identifier] = {"metrics": {"precision": 1.0, "recall": 1.0, "intersectionOverUnion": 1.0}}
        with tempfile.TemporaryDirectory() as temporary:
            report = Path(temporary) / "report.json"
            report.write_text(json.dumps({"layers": layers}))
            result = QUALITY.assess_reports([report], policy_path)
        self.assertTrue(result["valid"])


if __name__ == "__main__":
    unittest.main()
