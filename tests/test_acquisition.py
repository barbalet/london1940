#!/usr/bin/env python3

import hashlib
import importlib.util
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("fetch_source_maps", ROOT / "tools" / "fetch_source_maps.py")
FETCH = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(FETCH)


class AcquisitionTests(unittest.TestCase):
    def test_git_blob_checksum_and_mismatch(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "sheet.png"
            path.write_bytes(b"fixture bytes")
            expected = hashlib.sha1(b"blob 13\0fixture bytes", usedforsecurity=False).hexdigest()
            self.assertEqual(expected, FETCH.git_blob_sha1(path))
            FETCH.verify(path, expected)
            with self.assertRaisesRegex(FETCH.AcquisitionError, "checksum mismatch"):
                FETCH.verify(path, "0" * 40)

    def test_urls_are_pinned_to_manifest_revision(self):
        manifest = {"repository": "https://example.test/maps", "revision": "abc123"}
        url = FETCH.source_url(manifest, {"path": "160.png"})
        self.assertIn("/abc123/160.png", url)


if __name__ == "__main__":
    unittest.main()
