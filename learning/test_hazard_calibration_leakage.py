#!/usr/bin/env python3
"""Bounded research-only leakage and split regression test; no gameplay claims."""
import csv
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT))
from train_lower_tail_hazard import load, fit, metrics, FEATURES

with tempfile.TemporaryDirectory() as tmp:
    path = Path(tmp) / "episodes.csv"
    fields = ["epoch", "scenario", "game", "tray", "finalist", "split", "dead_within_3",
              "censored", *FEATURES]
    with path.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        for scenario in ("normal", "heavy", "shifted-heavy"):
            for split, epoch, labels in (("train", 430, (0, 0, 0, 1)),
                                         ("validation", 431, (1, 1, 1, 0))):
                for game, label in enumerate(labels):
                    writer.writerow(dict(epoch=epoch, scenario=scenario,
                                         game=game, tray=0, finalist=0, split=split,
                                         dead_within_3=label, censored=0,
                                         **{feature: 0 for feature in FEATURES}))
    rows, train, valid = load(path)
    assert train == {430} and valid == {431}
    result = metrics(rows, fit(rows, epochs=2), "validation")
    for scenario, value in result.items():
        assert value["training_reference_prevalence"] == 0.25
        # 3 positives, 1 negative evaluated against training-only p=0.25:
        # (3*(.75**2)+(.25**2))/4 == .4375.
        assert value["constant_prevalence_brier"] == 0.4375
    with path.open("a", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writerow(dict(epoch=430, scenario="normal", game=0,
                             tray=0, finalist=1, split="train",
                             dead_within_3=0, censored=0,
                             **{feature: 0 for feature in FEATURES}))
    expanded, _, _ = load(path)
    assert len(expanded) == len(rows) + 1
    with path.open("a", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writerow(dict(epoch=430, scenario="normal", game=0,
                             tray=0, finalist=1, split="train",
                             dead_within_3=0, censored=0,
                             **{feature: 0 for feature in FEATURES}))
    try:
        load(path)
        raise AssertionError("duplicate legal afterstate was accepted")
    except ValueError as exc:
        assert "Duplicate episode key" in str(exc)
    print("PASS: held-out reference leakage and legal-afterstate uniqueness")
