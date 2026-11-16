# Research-only diagnosis: unstable five-game early training decisions

Date: November 16, 2026 UTC. No production code/weights/checkpoint changes. This is a **model-selection diagnostic**, not a 10× claim or training result.

## Observed failure signal

The reproducible bounded runner's initial screen evaluates 21 nearby weight candidates using just five paired games per generator; the stage-two screen uses eight fresh games per generator for at most four finalists. Under five games, a naive empirical p10 essentially becomes one extreme sample, and a single combo break swings a per-move rate materially. This can make early candidate ranking and safety classification unstable; it does **not** make a rejected candidate safe, and cannot justify relaxing gates.

Concrete [run 43807467128](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43807467128) evidence: one training seed epoch 1845 showed isolated incumbent normal mean survival 0.6895×, p10 0.1217×, and heavy generator 2.155 candidate vs 0.556 original combo breaks per 100. Stage-one verified **0/21 safe**. Yet independent 30-game/generator holdouts 1973/1974 produced normal mean survival **3.2370× / 2.0571×**, normal p10 **1.3158× / 1.0857×**, and improved combo-break rates across all six generator/holdout comparisons. This is **not** evidence the strategy meets strict gates: the *worst* heavy holdout mean survival is only **1.0164×**, shifted-heavy 1974 p10 is only **1.0000×**, and no generator reaches 10×. Both holdouts still fail; no promotion. The discrepancies are empirical evidence of high sampling variance in miniature training screens, not a reason to re-use those settings or seeds.

## Evidence-based next bounded experiment to prioritize

For a *genuinely different* learned continuation-value or lower-tail policy candidate (not the same four static weights), compare a **smaller, predeclared policy shortlist** under more independent paired training games per generator, keeping the same approximate CI budget. An example budget-neutral design is eight candidates × twelve first-stage games each (96 candidate-games/generator) instead of 21 × five (105). Leave the final safety rejection rule and two strictly independent 30-game/generator holdouts completely unchanged. If staged training shows high variance, report paired per-game survival differences, their p10 and per-game combo breaks, not just ratios of unstable order statistics. Do not use any holdout result to retune the same model before a *new* independently seeded run. Repeat independently only after a full original strict 10× pass. Do not claim stable generalization from one unusual 30-game cohort.

## Conditions

All 8×8/three-piece exact legality/selftests, zero illegal, zero censored under cap4096, no worse combo deadlines or break rates, ≥10× p10/mean survival/synthetic estimated score across normal/heavy/shifted-heavy, independent two-holdout replication remain identical. The synthetic estimated score is not the real Block Blast score. No task may overlap another Actions research job or reuse used epochs; preserve failure artifacts. Keep main, validated checkpoint, live v3.4 iPhone code untouched.
