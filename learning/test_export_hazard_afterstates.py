#!/usr/bin/env python3
"""Research-only fail-closed hazard exporter contract tests."""
import copy
import json
import unittest
from export_hazard_afterstates import convert

BASE = {
    "epoch": 500, "scenario": "heavy", "game": 0, "tray": 0,
    "finalist": 0, "split": "train", "legal": True,
    "features": {
        "occupied": 20, "square_fits": 3, "hbar_fits": 4,
        "vbar_fits": 5, "hard_zero": 0, "largest_free": 30,
        "pockets": 1, "combo_streak": 2, "future_clear": 3,
    },
    "continuation": [{"completed": True}] * 3,
}

class ExportContract(unittest.TestCase):
    def test_valid_survival(self):
        self.assertEqual(convert([json.dumps(BASE)])[0]["dead_within_3"], 0)

    def test_valid_terminal_death(self):
        r = copy.deepcopy(BASE)
        r["continuation"] = [{"completed": True}, {"completed": False}]
        self.assertEqual(convert([json.dumps(r)])[0]["dead_within_3"], 1)

    def test_reject_malformed_or_censored(self):
        cases = []
        for field, value in [
            ("epoch", True), ("epoch", 2001), ("scenario", "unknown"),
            ("split", "test"), ("game", -1), ("finalist", 0.5),
        ]:
            r = copy.deepcopy(BASE)
            r[field] = value
            cases.append(r)
        for field, value in [
            ("occupied", 65), ("occupied", True), ("occupied", -1),
            ("largest_free", 50), ("hard_zero", 2), ("square_fits", 1.2),
        ]:
            r = copy.deepcopy(BASE)
            r["features"][field] = value
            cases.append(r)
        r = copy.deepcopy(BASE)
        r["continuation"] = [{"completed": True}]
        cases.append(r)
        r = copy.deepcopy(BASE)
        r["legal"] = False
        cases.append(r)
        for r in cases:
            with self.subTest(r=r), self.assertRaises(ValueError):
                convert([json.dumps(r)])

    def test_reject_cross_split_leakage(self):
        for same_epoch in (True, False):
            a = copy.deepcopy(BASE)
            b = copy.deepcopy(BASE)
            b["finalist"] = 1
            b["split"] = "validation"
            if not same_epoch:
                b["game"] = 1
            with self.subTest(same_epoch=same_epoch), self.assertRaises(ValueError):
                convert([json.dumps(a), json.dumps(b)])

    def test_reject_incomplete_feature_map(self):
        for bad in (None, [], {"occupied": 20}, {**BASE["features"], "hidden_rng": 123}):
            r = copy.deepcopy(BASE)
            r["features"] = bad
            with self.subTest(bad=bad), self.assertRaises(ValueError):
                convert([json.dumps(r)])

    def test_duplicate_finalist(self):
        with self.assertRaises(ValueError):
            convert([json.dumps(BASE), json.dumps(BASE)])

if __name__ == "__main__":
    unittest.main()
