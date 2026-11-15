# Research check 5 — 2026-11-14 19:33 US/Central

**Gate status: NOT VERIFIED. No new training, simulation, or 10x claim.**

## Verified research branch state
- Branch HEAD at inspection: `5de565d63617e98b28800d58b39e34a09942da9d`.
- Saved checkpoint: `16 120 120 30 60`; candidate weights: `120 120 30 60`. No promotion.
- Inspected `learning/train_tenx.c`, `learning/run_tenx_reproducible.sh`, and `.github/workflows/v22-tenx.yml` via the connected GitHub API.
- Latest listed research Actions run: [43639191903](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43639191903), success at 2026-11-14 20:46 UTC, on earlier commit `9c4d80bda51e6b86c4ff66d20d71e47bb97541d4`. Successful CI does **not** establish 10x performance.
- The research simulator checks the 8x8 board, 3-piece tray legality, three generator scenarios, synthetic survival and score proxy, p10 survival, invalid moves, combo-break rate, and avoidable combo deadlines. Simulator scores are not real Block Blast scores.

## Execution
- Retried `git ls-remote https://github.com/welcome23bask/BlockBlastAI.git refs/heads/feature/v2.2-10x-research-gate` in the local execution container.
- Failed: `Could not resolve host: github.com`.
- The GitHub connector can inspect source but cannot execute the C benchmark. Therefore no exact-branch build, training epoch, fresh never-trained seeds, or independent holdout was performed in this check.
- Avoided triggering Actions to conserve minutes.

## Next step
Use a network-enabled clean checkout of the exact research branch and run `bash learning/run_tenx_reproducible.sh <two distinct unused holdout epochs>`. Record source revision, complete training log, and paired normal/heavy/shifted-heavy holdouts, and repeat independently before accepting all strict gates. Do not promote a checkpoint unless every gate passes.

No changes to main, production deployment, or iPhone dylib.
