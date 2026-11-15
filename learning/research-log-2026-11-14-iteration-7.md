# Research iteration 7 — 2026-11-14 (US/Central)

**10× performance target: NOT VERIFIED. No new training or simulator results claimed.**

## Exact-source inspection
- Research branch HEAD observed: `e548bc572398f1d3dbcb11b5e62c9e4d4a8386d5`.
- Read `learning/train_tenx.c`, `learning/run_tenx_reproducible.sh`, `learning/tenx_checkpoint.txt`, `learning/tenx_candidate_weights.txt`, and `.github/workflows/v22-tenx.yml` from the research branch.
- Saved checkpoint: `16 120 120 30 60`; candidate: `120 120 30 60`.
- Latest listed research Actions run: [43639191903](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43639191903), successful on older commit `9c4d80bda51e6b86c4ff66d20d71e47bb97541d4`; this is **not** verification of current branch or of any 10× performance gate.

## New reproducibility findings
1. The standalone runner compiles exact local sources and performs one bounded training epoch plus two 30-game-per-generator synthetic holdouts, but its `trap 'rm -rf "$work"' EXIT` deletes raw training and holdout files at exit. To preserve independent evidence, capture complete stdout/stderr and the source revision externally in a research-only log/artifact before cleanup, without promoting the checkpoint.
2. The Actions workflow is triggered on pushes only for a limited set of source and workflow paths. Changes to `learning/run_tenx_reproducible.sh` and research logs alone do **not** trigger it. The most recent successful workflow is consequently stale relative to the corrected runner.
3. The workflow tests a single previously documented epoch 93 and only asserts a verdict line exists; a successful CI status is explicitly not equivalent to a passing candidate. It does not run the corrected two-holdout runner.
4. `train_tenx.c` caps each episode at 1,400 moves, and its score is a synthetic proxy. Every claimed 10× result must state the cap and must not be described as real Block Blast scoring.
5. Holdout epoch IDs 17..2000 are not automatically checked against all past training/test records; before use, audit the historical logs to establish they are truly unused. Training at saved epoch 16 uses training seeds distinct from those holdouts, subject to that audit.

## Execution decision
The connected GitHub interface supports source reads and research-branch commits, but does not provide a workflow-dispatch action or an executable checkout in this run. Therefore **no exact-source compilation, new training, fresh-seed holdout, or independent replication occurred**. Do not report any new performance numbers. Next actionable step: run the corrected script in a verified executable checkout, preserve full logs and source SHA, and repeat any success independently. No main branch, production deployment, iPhone dylib, or saved checkpoint changes; no new Actions minutes consumed.
