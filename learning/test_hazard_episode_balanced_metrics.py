#!/usr/bin/env python3
"""Research-only regression: duplicating one episode's finalists must not bias validation."""
import importlib.util
from pathlib import Path

spec = importlib.util.spec_from_file_location("hazard", Path(__file__).with_name("train_lower_tail_hazard.py"))
hazard = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hazard)

def row(split, scenario, episode, y, x):
    return (300 if split == "train" else 301, scenario, split, y, x, (300 if split == "train" else 301, scenario, episode))

rows = []
for scenario in ("normal", "heavy", "shifted-heavy"):
    for split in ("train", "validation"):
        rows.append(row(split, scenario, 0, 0, [1.0] + [0.0] * 9))
        rows.append(row(split, scenario, 1, 1, [1.0] + [0.0] * 9))
weights = [0.0] * 10
before = hazard.metrics(rows, weights, "validation")
# Many finalists from ONE source episode must not change its effective mass.
for scenario in ("normal", "heavy", "shifted-heavy"):
    for split in ("train", "validation"):
        rows.extend([row(split, scenario, 1, 1, [1.0] + [0.0] * 9) for _ in range(19)])
after = hazard.metrics(rows, weights, "validation")
for scenario in before:
    for field in ("brier", "constant_prevalence_brier", "training_reference_prevalence", "calibration_gap", "mean_predicted_hazard"):
        assert before[scenario][field] == after[scenario][field], (scenario, field, before[scenario], after[scenario])
print("EPISODE_BALANCED_VALIDATION=PASS")
