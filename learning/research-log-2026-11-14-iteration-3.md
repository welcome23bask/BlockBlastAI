# Research run 3 — 2026-11-14 17:30 US/Central

**Verdict: 10× NOT VERIFIED. No deployment or checkpoint promotion.**

## Source and checkpoint verified
- Exact research trainer `learning/train_tenx.c` blob `5e4390d90592fb4d3149d6316416bfdb5cad072b`.
- Baseline: separately compiled unchanged `src/BLDualStrategy.c` under symbol `BLV17FindTwoStrategies`; candidate: `learning/BLTrainableStrategy.c`.
- Current saved checkpoint: `16 120 120 30 60`; candidate weights: `120 120 30 60`. Neither modified.
- Most recent v2.2 GitHub Actions run `43639191903` completed successfully on November 14 at 20:46 UTC. This was an earlier CI run, not a fresh holdout in this iteration.
- Confirmed exact `test` CLI from source: `test [games] [weights.txt] [withheld_epoch]`, `10 <= games <= 1000`, `0 <= epoch <= 2000`.
- The trainer uses an 8×8 board and three visible pieces, checks the selected piece index, bounds, occupancy, before/after board and cleared-line count, and has distinct normal, heavy and shifted-heavy generation paths. Simulator scoring is approximate, not official game scoring.

## Work done this iteration
Created `learning/run_tenx_reproducible.sh`, a standalone fail-closed runner that:
1. Requires a clean checkout of this research branch and records the exact commit.
2. Copies the branch source and checkpoint into an isolated temporary directory.
3. Compiles the unchanged v1.7 and research strategy independently, then links the exact trainer.
4. Performs one bounded epoch with one seed per generator, saving its provisional candidate only in the temporary directory.
5. Requires two distinct, previously unused holdout epochs supplied by the operator; each tests 30 paired games per generator.
6. Verifies three scenario ratios and six zero-illegal counts in each holdout. The simulator's built-in verdict also checks 10× mean survival, score proxy, p10 survival, combo-break rate and avoidable combo deadlines.
7. Does not change tracked checkpoints, production code, iPhone dylib or main.

## Execution limitation
The GitHub connector can read/write source but does not run C. This environment cannot resolve github.com for checkout, and there is no available GitHub workflow dispatch action. Therefore the exact-source runner could not be executed here, and **no fresh training results or new measured ratios are claimed**. The earlier five-game exploratory comparisons remain diagnostic only.

## Next step
Run `bash learning/run_tenx_reproducible.sh 1307 1308` from a clean checkout, after checking these two epoch IDs against all existing research logs. If either was used before, choose fresh IDs. Do not accept a candidate until both independent holdouts meet all required thresholds. Avoid a new GitHub Actions run unless necessary.

**No 10× gate passed; retain v1.7 and continue research.**
