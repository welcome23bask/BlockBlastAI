# Research-only bounded hypothesis — 2026-11-21, occupancy-gated shifted-cross bonus

Branch: feature/v2.2-10x-research-gate. Source commit 68a527e3d57864ee431b347f9a3c369b96fd8aee; regression guard e4dac75b0234117c666fdbe07b6467dcda72b018.

## Diagnosis and new change
Prior Actions 44837049501 failed strict gates, including heavy p10 0.8500x on holdout 797 and normal p10 0.9737x on 798. Latest inspected workflow still references reserved hazard/train/holdout epochs 794/795/796/797/798. The additional +75 admission reward for a third shifted-cross landing was unconditional even on spacious afterstates; it could crowd out normal-score or combo-safe finalists before the late rerank. New targeted hypothesis: apply the third-landing bonus only when fewer than 32 free cells remain, preserving the pre-existing three-family maximin and exact disjoint-square feasibility predicate. This is observable geometry only; no hidden next-piece RNG. Existing 20-finalist limit, search budget, unchanged v1.7 baseline, and strict 10x gates remain unchanged.

## Evidence and status
Fetched the exact workflow, reproducible runner, cross-role ledger, iteration-44 log, provisional frontier, hazard protocol, strategy and regression guard before changing source. Added a regression assertion for the occupancy gate. No CI compilation, seed preflight, fresh simulation, or independent holdout was executed in this iteration. No new epochs consumed. Current workflow seed IDs are reserved; do not trigger it unchanged. Repository-wide Actions non-overlap cannot be certified with available connector run-listing functions. The accessible ledger and historical runs may omit external/local use.

## Next bounded experiment
Verify no active overlapping Actions jobs through a complete run list; audit five distinct unused hazard train, validation, policy train, holdout A and B epochs in 17..2000, including raw-start collisions with all ledger epochs; run preflight before training. Only then replace every embedded workflow epoch reference consistently, preserving the immutable executing snapshot and reserving epochs after the run starts. Compare unchanged v1.7 on independent paired normal/heavy/shifted-heavy 8x8 three-piece games with 30 games/generator/holdout and report mean, p10, synthetic estimated score, illegals, combo breaks/move, avoidable deadlines, and censoring. No promotion unless all >=10x gates pass on both independent holdouts. Synthetic estimated score is not actual Block Blast points.

No changes to main, live v3.4 iPhone/Automatic Play, deployment, or validated checkpoints. This is an untested hypothesis, not a gameplay improvement or 10x result.
