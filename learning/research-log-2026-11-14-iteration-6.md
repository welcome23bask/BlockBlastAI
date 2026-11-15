# Research iteration 6 — 2026-11-14 (US/Central)

**10× target: NOT VERIFIED. No training or new simulation measurements in this iteration.**

## Inspected source and saved state
- Exact branch source: `learning/train_tenx.c`, `learning/run_tenx_reproducible.sh`, `.github/workflows/v22-tenx.yml`.
- Saved checkpoint: `16 120 120 30 60`; candidate weights: `120 120 30 60`.
- Most recently documented research Actions run: [43639191903](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43639191903), successful CI on an earlier revision; this does not prove any 10× gate. No new Actions run was launched.

## Reproducibility and safety correction
The prior standalone runner only asserted that `TEN_X_VERDICT=` appeared in the output. Thus it could print "Research run completed" even if both synthetic holdouts explicitly said `NOT REACHED - NO DEPLOY`. It also validated holdout arguments after running the training epoch and allowed untracked source files in the checkout.

Updated `learning/run_tenx_reproducible.sh` at commit `ecd464c03b50f935e6c200ba73b41504d0e582cd` to:
1. Reject missing, invalid, duplicate, or previously documented holdout epoch IDs before training.
2. Reject untracked files as well as staged/unstaged modifications in the checkout.
3. Require the exact `TEN_X_VERDICT=ALL HELDOUT GATES PASSED` for each of two independent holdouts, otherwise exit nonzero.

The script still relies on the simulator's internal gate for 10× mean survival, score proxy, p10 survival, zero illegal placements, combo-break rate and avoidable combo misses. Operator must independently verify that holdout epoch IDs are truly never-trained/never-tested; the built-in exclusions cannot establish that across all historical logs.

## Execution and next step
No local exact-source compilation or training was performed. A verified executable checkout of the research branch is still required. Run the corrected script on two genuinely unused holdout epochs, preserve full outputs and source SHA, and independently repeat successful results before any 10× claim. Simulator scores are not official Block Blast scores.

No checkpoint promotion, no production/main modification, no deployment, no iPhone dylib build, and no new Actions minutes.
