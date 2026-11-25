#!/usr/bin/env python3
"""Research-only, bounded, deterministic lower-tail hazard calibration.

This is an OFFLINE training component; only separately validated models may feed the optional research policy.
Input CSV columns: epoch,scenario,game,tray,finalist,split,dead_within_3,censored,
occupied,square_fits,hbar_fits,vbar_fits,hard_zero,
largest_free,pockets,combo_streak,future_clear.
Each (epoch,scenario,game,tray,finalist) identifies ONE legal afterstate.
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
SCALES = (64., 36., 32., 32., 11., 64., 20., 10., 32.)

def load(path):
    rows = []
    seen = set()
    with open(path, newline="", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            key = (int(r["epoch"]), r["scenario"], int(r["game"]),
                   int(r["tray"]), int(r["finalist"]))
            if key[3] < 0 or key[4] < 0:
                raise ValueError("Negative tray/finalist index")
            if key in seen:
                raise ValueError("Duplicate episode key: " + str(key))
            seen.add(key)
            if r["split"] not in ("train", "validation"):
                raise ValueError("Only train/validation splits are permitted")
            if int(r["censored"]) != 0:
                raise ValueError("Censored continuation labels cannot be treated as known")
            if int(r["dead_within_3"]) not in (0, 1):
                raise ValueError("Death label must be binary")
            raw = {k: float(r[k]) for k in FEATURES}
            if not all(math.isfinite(v) for v in raw.values()):
                raise ValueError("Nonfinite feature")
            # Physical 8x8 afterstate contract: impossible feature vectors
            # must fail closed rather than contaminate a calibrated hazard.
            if any(v < 0 for v in raw.values()):
                raise ValueError("Negative afterstate feature")
            if (raw["occupied"] > 64 or raw["largest_free"] > 64 or
                    raw["occupied"] + raw["largest_free"] > 64):
                raise ValueError("Impossible 8x8 occupancy/free-region geometry")
            if ((raw["occupied"] == 64 and raw["largest_free"] != 0) or
                    (raw["occupied"] < 64 and raw["largest_free"] == 0) or
                    (raw["occupied"] == 0 and raw["largest_free"] != 64)):
                raise ValueError("Impossible empty/full board connectivity")
            if raw["hard_zero"] > 11:
                raise ValueError("hard_zero must count 0..11 blocked hard shapes")
            if any(raw[k] != int(raw[k]) for k in FEATURES):
                raise ValueError("Afterstate count features must be integers")
            x = [1.] + [raw[k] / s for k, s in zip(FEATURES, SCALES)]
            rows.append((int(r["epoch"]), r["scenario"], r["split"],
                         int(r["dead_within_3"]), x, key[:3]))
    if not rows:
        raise ValueError("Empty dataset")
    train = {r[0] for r in rows if r[2] == "train"}
    valid = {r[0] for r in rows if r[2] == "validation"}
    if not train or not valid or train & valid:
        raise ValueError("Training and validation epochs must be nonempty and disjoint")
    if not {"normal", "heavy", "shifted-heavy"}.issubset(
            {r[1] for r in rows if r[2] == "train"}):
        raise ValueError("Missing training generator")
    if any(r[1] not in ("normal", "heavy", "shifted-heavy") for r in rows):
        raise ValueError("Unknown generator")
    if any(not 17 <= r[0] <= 2000 for r in rows):
        raise ValueError("Epoch outside research range")
    return rows, train, valid

def sigmoid(z):
    return 1. / (1. + math.exp(-max(-35., min(35., z))))

# Fit the same scenario-blind logistic model against the actual calibration
# criterion, not just pooled cross-entropy. All weights, reference priors and
# optimizer state come from training rows only. The fresh validation gate is
# unchanged. Smooth worst-generator excess Brier gives struggling generators
# more gradient mass without reading their identities at inference.
def fit(rows, epochs=300, rate=0.25, regularization=0.001):
    data = [r for r in rows if r[2] == "train"]
    scenarios = ("normal", "heavy", "shifted-heavy")
    groups = {}
    for scenario in scenarios:
        group = [r for r in data if r[1] == scenario]
        counts = {}
        for r in group:
            counts[r[5]] = counts.get(r[5], 0) + 1
        if not counts:
            raise ValueError("Missing training generator: " + scenario)
        weighted = [(r[3], r[4], 1. / (len(counts)*counts[r[5]])) for r in group]
        prior = sum(y*mass for y,x,mass in weighted)
        reference = sum(mass*(prior-y)**2 for y,x,mass in weighted)
        groups[scenario] = (weighted, reference, prior)
    prior = max(.001, min(.999, sum(groups[s][2] for s in scenarios)/3.))
    w = [math.log(prior/(1.-prior))] + [0.] * len(FEATURES)
    m, v = [0.]*len(w), [0.]*len(w)
    # Fixed optimizer and smooth-max temperature: no validation-driven search.
    temperature = .025
    for step in range(1, epochs+1):
        excess, gradients = [], []
        for scenario in scenarios:
            group, reference, _ = groups[scenario]
            loss = 0.
            grad = [0.]*len(w)
            for y,x,mass in group:
                p = sigmoid(sum(a*b for a,b in zip(w,x)))
                loss += mass*(p-y)**2
                d = 2.*mass*(p-y)*p*(1.-p)
                for j in range(len(w)):
                    grad[j] += d*x[j]
            excess.append(loss-reference)
            gradients.append(grad)
        top = max(excess)
        importance = [math.exp((e-top)/temperature) for e in excess]
        denom = sum(importance)
        for j in range(len(w)):
            g = sum(q*grad[j] for q,grad in zip(importance,gradients))/denom
            if j:
                g += regularization*w[j]
            m[j] = .9*m[j]+.1*g
            v[j] = .999*v[j]+.001*g*g
            w[j] -= (rate/10.)*(m[j]/(1.-.9**step))/(math.sqrt(v[j]/(1.-.999**step))+1e-8)
    return w

def metrics(rows, w, split):
    out = {}
    for scenario in ("normal", "heavy", "shifted-heavy"):
        selected = [r for r in rows if r[2] == split and r[1] == scenario]
        if not selected:
            raise ValueError("Missing " + split + " generator " + scenario)
        predictions = [sigmoid(sum(a*b for a,b in zip(w,r[4])))
                       for r in selected]
        # Score independent source episodes equally, regardless of finalist count.
        episode_counts = {}
        for r in selected:
            episode_counts[r[5]] = episode_counts.get(r[5], 0) + 1
        weights = [1. / (len(episode_counts) * episode_counts[r[5]]) for r in selected]
        brier = sum(weight*(p-r[3])**2 for weight,p,r in zip(weights,predictions,selected))
        positives = sum(r[3] for r in selected)
        prevalence = sum(weight*r[3] for weight,r in zip(weights,selected))
        # Reference prevalence is fitted on TRAIN ONLY. Computing it from
        # validation labels leaks the holdout outcomes into the comparator.
        reference = [r for r in rows if r[2] == "train" and r[1] == scenario]
        reference_counts = {}
        for r in reference:
            reference_counts[r[5]] = reference_counts.get(r[5], 0) + 1
        train_prevalence = sum(r[3] / reference_counts[r[5]] for r in reference) / len(reference_counts)
        constant_brier = sum(weight*(train_prevalence-r[3])**2 for weight,r in zip(weights,selected))
        out[scenario] = {"n":len(selected), "deaths":positives,
                         "brier":round(brier, 6),
                         "constant_prevalence_brier":round(constant_brier, 6),
                         "training_reference_prevalence":round(train_prevalence, 6),
                         "calibration_gap":round(abs(sum(weight*p for weight,p in zip(weights,predictions))-prevalence), 6),
                         "mean_predicted_hazard":round(sum(weight*p for weight,p in zip(weights,predictions)),6)}
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
    # Do not let a strong validation epoch conceal a catastrophic weak epoch.
    # Use unrounded scores for the acceptance decision; rounded diagnostics
    # can otherwise turn a small regression into an apparent tie.
    epoch_validation = {}
    for scenario in ("normal", "heavy", "shifted-heavy"):
        epoch_validation[scenario] = {}
        for epoch in sorted(valid):
            subset = [r for r in rows if r[2] == "validation" and
                      r[1] == scenario and r[0] == epoch]
            if not subset:
                raise ValueError(f"Missing validation generator {scenario} epoch {epoch}")
            train_subset = [r for r in rows if r[2] == "train" and r[1] == scenario]
            train_counts = {}
            for r in train_subset:
                train_counts[r[5]] = train_counts.get(r[5], 0) + 1
            prior = sum(r[3] / train_counts[r[5]] for r in train_subset) / len(train_counts)
            counts = {}
            for r in subset:
                counts[r[5]] = counts.get(r[5], 0) + 1
            model_brier = sum((sigmoid(sum(a*b for a,b in zip(w,r[4]))) - r[3])**2 /
                              (len(counts) * counts[r[5]]) for r in subset)
            constant_brier = sum((prior-r[3])**2 /
                                 (len(counts) * counts[r[5]]) for r in subset)
            epoch_validation[scenario][str(epoch)] = {
                "brier": round(model_brier, 9),
                "constant_prevalence_brier": round(constant_brier, 9),
                "passes_unrounded": model_brier <= constant_brier,
            }
    safe = all(v["passes_unrounded"] for scenario in epoch_validation.values()
               for v in scenario.values())
    result = {"status":("research_only_validation_diagnostic_pass" if safe
                         else "research_only_validation_diagnostic_fail"),
              "feature_names":["intercept"]+list(FEATURES),
              "feature_scales":list(SCALES),
              "weights":[round(v, 9) for v in w],
              "train_epochs":sorted(train), "validation_epochs":sorted(valid),
              "train":metrics(rows,w,"train"),
              "validation":validation,
              "per_epoch_validation":epoch_validation,
              "all_generator_brier_not_worse_than_constant":safe,
              "fit_objective":"training-only smooth worst-generator excess Brier, episode-balanced Adam",
              "limitations":"Model calibration only; no holdout gameplay evidence or 10x claim"}
    a.output.write_text(json.dumps(result, indent=2)+"\n", encoding="utf-8")
    print(json.dumps(result, sort_keys=True))

if __name__ == "__main__":
    main()
