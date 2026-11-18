#!/usr/bin/env python3
"""Research-only, bounded, deterministic lower-tail hazard calibration.

This is an OFFLINE training component, not wired into the gameplay policy.
Input CSV columns: epoch,scenario,game,split,dead_within_3,censored,
occupied,square_fits,hbar_fits,vbar_fits,hard_zero,
largest_free,pockets,combo_streak,future_clear.
Rows MUST originate from completed legal tray afterstates, with labels
obtained using continuation simulations, not from withheld test epochs.
Splits are train/validation, with disjoint epoch IDs. No hidden RNG features.
"""
import argparse
import csv
import json
import math
from pathlib import Path

FEATURES = ("occupied", "square_fits", "hbar_fits", "vbar_fits",
            "hard_zero", "largest_free", "pockets", "combo_streak",
            "future_clear")
SCALES = (64., 36., 32., 32., 7., 64., 20., 10., 32.)

def load(path):
    rows = []
    seen = set()
    with open(path, newline="", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            key = (int(r["epoch"]), r["scenario"], int(r["game"]))
            if key in seen:
                raise ValueError("Duplicate episode key: " + str(key))
            seen.add(key)
            if r["split"] not in ("train", "validation"):
                raise ValueError("Only train/validation splits are permitted")
            if int(r["censored"]) != 0:
                raise ValueError("Censored continuation labels cannot be treated as known")
            if int(r["dead_within_3"]) not in (0, 1):
                raise ValueError("Death label must be binary")
            x = [1.] + [float(r[k]) / s for k, s in zip(FEATURES, SCALES)]
            if not all(math.isfinite(v) for v in x):
                raise ValueError("Nonfinite feature")
            rows.append((int(r["epoch"]), r["scenario"], r["split"],
                         int(r["dead_within_3"]), x))
    if not rows:
        raise ValueError("Empty dataset")
    train = {r[0] for r in rows if r[2] == "train"}
    valid = {r[0] for r in rows if r[2] == "validation"}
    if not train or not valid or train & valid:
        raise ValueError("Training and validation epochs must be nonempty and disjoint")
    if not {"normal", "heavy", "shifted-heavy"}.issubset(
            {r[1] for r in rows if r[2] == "train"}):
        raise ValueError("Missing training generator")
    return rows, train, valid

def sigmoid(z):
    return 1. / (1. + math.exp(-max(-35., min(35., z))))

def fit(rows, epochs=300, rate=0.25, regularization=0.05):
    data = [r for r in rows if r[2] == "train"]
    w = [0.] * (len(FEATURES) + 1)
    # Equalize generator contributions rather than fitting the easiest
    # scenario. The generator identity is NEVER an inference feature.
    counts = {s: sum(r[1] == s for r in data)
              for s in ("normal", "heavy", "shifted-heavy")}
    for _ in range(epochs):
        grad = [0.] * len(w)
        for _, scenario, _, y, x in data:
            error = sigmoid(sum(a*b for a, b in zip(w, x))) - y
            weight = 1. / (3. * counts[scenario])
            for j in range(len(w)):
                grad[j] += weight * error * x[j]
        for j in range(len(w)):
            if j:
                grad[j] += regularization * w[j]
            w[j] -= rate * grad[j]
    return w

def metrics(rows, w, split):
    out = {}
    for scenario in ("normal", "heavy", "shifted-heavy"):
        selected = [r for r in rows if r[2] == split and r[1] == scenario]
        if not selected:
            raise ValueError("Missing " + split + " generator " + scenario)
        predictions = [sigmoid(sum(a*b for a,b in zip(w,r[4])))
                       for r in selected]
        brier = sum((p-r[3])**2 for p,r in zip(predictions, selected))/len(selected)
        positives = sum(r[3] for r in selected)
        prevalence = positives / len(selected)
        constant_brier = sum((prevalence-r[3])**2 for r in selected)/len(selected)
        out[scenario] = {"n":len(selected), "deaths":positives,
                         "brier":round(brier, 6),
                         "constant_prevalence_brier":round(constant_brier, 6),
                         "calibration_gap":round(abs(sum(predictions)/len(predictions)-prevalence), 6),
                         "mean_predicted_hazard":round(sum(predictions)/len(predictions),6)}
    return out

def main():
    p = argparse.ArgumentParser()
    p.add_argument("dataset", type=Path)
    p.add_argument("--output", type=Path, required=True)
    a = p.parse_args()
    rows, train, valid = load(a.dataset)
    w = fit(rows)
    validation = metrics(rows,w,"validation")
    # A weakly calibrated hazard model must not silently become a policy.
    # This is a diagnostic gate only; the existing gameplay gates remain unchanged.
    safe = all(v["brier"] <= v["constant_prevalence_brier"]
               for v in validation.values())
    result = {"status":("research_only_validation_diagnostic_pass" if safe
                         else "research_only_validation_diagnostic_fail"),
              "feature_names":["intercept"]+list(FEATURES),
              "feature_scales":list(SCALES),
              "weights":[round(v, 9) for v in w],
              "train_epochs":sorted(train), "validation_epochs":sorted(valid),
              "train":metrics(rows,w,"train"),
              "validation":validation,
              "all_generator_brier_not_worse_than_constant":safe,
              "limitations":"No game policy integration, no holdout tests, no 10x claim"}
    a.output.write_text(json.dumps(result, indent=2)+"\n", encoding="utf-8")
    print(json.dumps(result, sort_keys=True))

if __name__ == "__main__":
    main()
