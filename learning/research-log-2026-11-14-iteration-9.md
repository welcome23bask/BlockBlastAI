# Research iteration 9 — 2026-11-14 (US/Central)

**10× challenge: NOT VERIFIED. No new training or simulation was executed.**

## Source and CI inspection
- Branch source HEAD before changes: `aef6fca7576516811b39aff9affea0abc5485c6f`.
- Checkpoint `learning/tenx_checkpoint.txt`: `16 120 120 30 60`; candidate `learning/tenx_candidate_weights.txt`: `120 120 30 60`.
- Latest research Actions success: run 43639191903, executed commit `9c4d80bda51e6b86c4ff66d20d71e47bb97541d4` on 2026-11-14 UTC; older than current runner and not independent two-holdout verification.
- Current workflow `.github/workflows/v22-tenx.yml` tests epoch 93 and merely checks for a verdict, so a green run is **not** a pass of all requested gates.

## Work performed
- Attempted exact Git checkout via `git ls-remote`; failed with `Could not resolve host: github.com`. GitHub connector can inspect and edit files but cannot execute C simulator. No alternate binary was substituted.
- Updated `learning/run_tenx_reproducible.sh` in commit `e32c5838934184b43f2bc5001e010728c18f3fc8` to preserve training and holdout transcripts plus checkpoint and candidate previews in a separately named temporary evidence directory **even when a gate fails**. Source revision and evidence directory are printed. No production files or CI workflow were changed.
- This evidence-retention edit has not been compiled or exercised in an exact-source checkout; its correctness remains to be tested.

## Remaining gates
Audit unused holdout seeds, compile exact research branch, run bounded training and two fresh independent holdouts across normal/heavy/shifted-heavy, verify 8×8 three-piece legality, per-generator average survival and synthetic score >=10× unchanged v1.7, p10 >=10×, zero invalid placements, no worse combo-break rate and no more avoidable deadlines, and independently repeat on disjoint seeds. Preserve full evidence, including failures. Simulator score estimates are **not** actual Block Blast scores. The 1,400-move cap can conceal ratios.

No checkpoint promotion, CI dispatch, extra Actions minutes, main change, merge, deployment, or dylib build. Do not claim any 10× gain.
