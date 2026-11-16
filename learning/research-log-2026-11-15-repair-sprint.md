# BlockBlastAI research repair sprint — iterations 28–30

Date: November 15, 2026 America/Chicago (Actions timestamps cross into Nov 16 UTC).
Branch: `feature/v2.2-10x-research-gate`. Baseline is separately compiled, unchanged v1.7 SMART SURVIVAL. Synthetic simulator estimates are **not** actual Block Blast scores.

## Systemic failures identified and repaired

1. **Contradictory seed exclusions.** Workflow and two separate runner exclusion lists had repeatedly disagreed, causing false preflight failure or leaving cross-role gaps. Added `learning/tenx_used_epochs.txt` as the single cross-role documented-epoch exclusion source (commit `71547694`), and switched the runner to validate it (commit `9ed162cd`). Reserved epochs from completed runs; future experiments must advance IDs and audit the ledger before launching.
2. **Missing automated legality regressions.** Added `learning/tenx_rules_selftest.c` (commit `fa92cf03`), compiled and executed by the research runner before training (commit `9d00a5db`). Tests simultaneous row/column clearing, legally completed 3-piece plans from baseline and trained solvers, exhausted-board rejection, and used-piece masking.
3. **Missing evidence after failures.** Fixed doubly escaped `\\n` strings in the runner's directory/revision output (commit `25a3661a`), letting GitHub Actions identify and archive raw training/holdout evidence on failure.
4. **Training-game parameter silently ignored.** Added validated `TENX_TRAIN_GAMES` in the runner (commit `21a1d93d`); this applies from iteration 29. The iteration-28 workflow requested five training games but the script at that exact source revision used its hardcoded three; do not misstate that iteration as five-game training.
5. **Training objective did not match per-generator acceptance gates.** Research-only commit `9d72435d` replaces sum of generator fitness with worst-generator fitness. The independent 10x holdout gates are unchanged.
6. **Research solver search bias.** The depth-first search can consume all 180,000 nodes under early root branches, starving other piece orders. Research-only commit `e59493f` distributes the same total budget across legal first actions; the v1.7 baseline remains unchanged. Commit `cd35d7f` adds baseline/trained search-budget truncation diagnostics to help validate this hypothesis.
7. **Avoidable combo deadlines.** Research-only commit `d478d838` forces a discovered complete legal three-piece combo-saving path at the observed final allowed non-clear move, rather than rejecting that path solely on a board-utility gap. This can trade survival for combo safety; acceptance gates remain strict.

## Iteration 28 — fixed pipeline and first worst-generator test

[GitHub Actions run 43789646663](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43789646663), source `d97103cc`, completed compilation, documented seed preflight (train 1828; holdouts 1939,1940), rules selftest **PASS**, one 3-game-per-generator training epoch and two 30-game-per-generator paired holdouts. All 21 training candidates failed combo safety (0 illegal), so experimental weights stayed 120,120,30,60 inside the isolated runner. Holdout survival ratios:

| Generator | 1939 | 1940 |
| --- | ---: | ---: |
| normal | 1.3424x | 1.0002x |
| heavy | 1.0834x | 1.3259x |
| shifted-heavy | 1.0516x | 1.1158x |

Normal p10 fell below baseline on 1939 (0.7931x); heavy p10 fell on 1939 (0.7391x); shifted p10 fell on 1940 (0.8500x). Several combo breaks/deadlines regressed. Both strict verdicts failed.

## Iteration 29 — last-chance combo guard and correctly applied five training games

[GitHub Actions run 43790179104](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43790179104), source `2f377901`, completed compilation, preflight (train 1829; holdouts 1941,1942), selftest **PASS**, one epoch with five paired training games/generator and both 30-game/generator holdouts. **11/21 candidates passed initial combo-safety screening**, versus 0/21 in iteration 28. The provisional selected weights were 120,60,30,120. Both strict holdouts still failed.

| Generator | Mean survival ratio 1941 | Mean survival ratio 1942 |
| --- | ---: | ---: |
| normal | 0.9876x | 1.4072x |
| heavy | 1.0557x | 1.0537x |
| shifted-heavy | 0.9694x | 1.0904x |

No illegal placements were reported; candidate avoidable deadlines were **no worse than the baseline in all six holdout comparisons**. Other criteria still failed, including survival, p10, and some combo-break rates. This is progress on combo feasibility, **not** proven overall gameplay improvement.

## Iteration 30 — bounded fair-root search trial

[GitHub Actions run 43790591777](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43790591777), source `28163cbe`, was in progress at initial log drafting. Provisional training epoch 1830, holdout epochs 1943 and 1944, five paired training games/generator, 21 candidates, two 30-game/generator synthetic holdouts, same strict gates, and added search-truncation percentage instrumentation. Update this section once the run has completed.

## Provenance and deployment

No full external historical seed-use audit is available; current epoch non-use is supported only by the accessible branch logs and recent Actions runs, so treat these epochs as provisional until independent verification. No code was deployed to a phone. No production or main changes, no dylib build, no saved checkpoint/candidate promotion: saved checkpoint remains `16 120 120 30 60`. The 10x target has **not** been verified. Do not imply a failed Actions workflow is a failure to execute when compilation and simulations actually completed; distinguish performance-gate failure from infrastructure failure.
