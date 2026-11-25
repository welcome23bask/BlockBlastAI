#!/usr/bin/env python3
"""Export only a separately calibrated research model; never a checkpoint."""
import argparse
import json
import math
from pathlib import Path

NAMES = ["intercept", "occupied", "square_fits", "hbar_fits", "vbar_fits",
         "hard_zero", "largest_free", "pockets", "combo_streak", "future_clear"]
SCALES = [64., 36., 32., 32., 11., 64., 20., 10., 32.]
SCENARIOS = {"normal", "heavy", "shifted-heavy"}

def render(model, train, valid):
    if train == valid or not (17 <= train <= 2000 and 17 <= valid <= 2000):
        raise ValueError("invalid hazard epochs")
    if model.get("train_epochs") != [train] or model.get("validation_epochs") != [valid]:
        raise ValueError("hazard provenance mismatch")
    if (model.get("status") != "research_only_validation_diagnostic_pass" or
            model.get("all_generator_brier_not_worse_than_constant") is not True):
        raise ValueError("hazard calibration failed; no policy experiment")
    checks = model["per_epoch_validation"]
    if set(checks) != SCENARIOS:
        raise ValueError("missing generator calibration")
    for scenario in SCENARIOS:
        if set(checks[scenario]) != {str(valid)}:
            raise ValueError("validation epoch mismatch")
        row = checks[scenario][str(valid)]
        a, b = row["brier"], row["constant_prevalence_brier"]
        if (row.get("passes_unrounded") is not True or
                not all(type(x) in (int, float) and math.isfinite(x) and 0 <= x <= 1
                        for x in (a,b)) or a > b):
            raise ValueError("generator calibration failed")
    if model.get("feature_names") != NAMES or model.get("feature_scales") != SCALES:
        raise ValueError("hazard feature schema mismatch")
    weights = model["weights"]
    if len(weights) != len(NAMES) or not all(
            type(x) in (int,float) and math.isfinite(x) and abs(x) <= 1000
            for x in weights):
        raise ValueError("invalid model coefficients")
    return ("/* Generated research-only model, never commit. */\n"
            "static const double bl_hazard_weights[10] = {"
            + ", ".join(format(float(x), ".17g") for x in weights) + "};\n"
            "static const double bl_hazard_scales[9] = {"
            + ", ".join(format(x, ".17g") for x in SCALES) + "};\n")

def main():
    p = argparse.ArgumentParser()
    p.add_argument("model", type=Path)
    p.add_argument("output", type=Path)
    p.add_argument("train", type=int)
    p.add_argument("validation", type=int)
    a = p.parse_args()
    header = render(json.loads(a.model.read_text()), a.train, a.validation)
    a.output.write_text(header)
    print("HAZARD_POLICY_EXPORT=PASS")

if __name__ == "__main__":
    main()
