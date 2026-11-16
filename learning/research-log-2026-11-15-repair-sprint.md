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

[GitHub Actions run 43790591777](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43790591777), source `28163cbe`, **completed** preflight (train 1830; holdouts 1943,1944), compile, rule selftest PASS, one five-game-per-generator training epoch, both 30-game-per-generator paired holdouts, and evidence preservation. Training found **9/21 combo-safe candidates**, 12 combo rejections, 0 illegal; provisional selected weights 180,60,30,60. Both strict holdouts failed.

| Generator | 1943 mean survival / proxy / p10 ratio | 1944 mean survival / proxy / p10 ratio |
| --- | --- | --- |
| normal | 1.9438x / 2.1762x / 1.4706x | 1.1219x / 1.1438x / 0.9756x |
| heavy | 1.2748x / 1.5329x / 1.0000x | 1.1417x / 1.2134x / 1.0526x |
| shifted-heavy | **0.9404x / 0.8788x / 1.0000x** | **0.9793x / 0.9251x / 1.0000x** |

All reported placements legal. No additional avoidable combo deadlines. Shifted-heavy combo-break rate improved versus baseline, but shifted-heavy survival and score proxy were **worse** on both holdouts. Truncation instrumentation found approximately 0.7–2.1% of baseline decisions and 1.8–2.7% of candidate decisions exhausted the solver's search budget. This suggests first-root DFS starvation exists but is not, by itself, the dominant cause of the 10x gap: fair-root stratification did not fix shifted-heavy survival. Do not promote this experiment as a winning policy. The simulator's scores are estimates only.

The bounded fair-root design has now been experimentally evaluated and should be considered an **unverified, generator-sensitive hypothesis**, not the default recommendation for the phone. Target the shifted-heavy survival/tail weakness and generalization across independent holdouts next.

## Provenance and deployment

No full external historical seed-use audit is available; current epoch non-use is supported only by the accessible branch logs and recent Actions runs, so treat these epochs as provisional until independent verification. No code was deployed to a phone. No production or main changes, no dylib build, no saved checkpoint/candidate promotion: saved checkpoint remains `16 120 120 30 60`. The 10x target has **not** been verified. Do not imply a failed Actions workflow is a failure to execute when compilation and simulations actually completed; distinguish performance-gate failure from infrastructure failure.

## Post-experiment safety decision

Because iteration 30's fair-root traversal regressed shifted-heavy mean survival and proxy score in **both** holdouts, commit `cbadb897` gated the fair-root implementation behind compile-time `BL_RESEARCH_FAIR_ROOT=1`, default OFF. This preserves the hypothesis for explicit research A/B experiments without silently treating it as a promoted improvement. The default behavior has not itself been independently recompiled after this toggle commit; the upcoming bounded research run must compile and rerun tests.

All used iteration 28–30 IDs (1828/1939/1940; 1829/1941/1942; 1830/1943/1944) were entered in the central exclusion ledger. Do not rerun the workflow's currently configured 1830/1943/1944 IDs: the runner will correctly reject them. Do not launch a duplicate job merely to check whether the workflow is green.

## What still blocks 10x and the next useful hypothesis

1. **Generalization failure:** New coefficients and optional root search have not consistently beaten v1.7 across normal, heavy and shifted-heavy piece distributions, especially p10. Both 30-game holdouts can disagree even on mean improvement.
2. **Long-horizon planning:** Current afterstate heuristic mainly rewards one-tray footprints; four-weight neighborhood search cannot learn which boards survive unfavorable **future** trays. Next worthwhile research change is a bounded, distribution-aware rollout/feasibility evaluator that samples unknown future trays from each *documented synthetic generator* without peeking at actual future draws during deployment. Compare legal full-tray outcomes and combo deadlines. Do not treat this as a guarantee.
3. **Validation power:** Five paired training games per generator are insufficient to predict 30-game holdout gains reliably. Consider increasing independent training seeds or validation sampling within the 12-minute cap, using a cheaper candidate shortlisting stage and a larger final paired validation. Do not train on holdout epochs.
4. **Simulator mismatch:** 28 shape patterns and three synthetic generators are not the real game's proprietary piece distribution/scoring; no 10x claim can be transferred to phone results without observed real-tray evidence.
5. **Model promotion:** Keep current saved checkpoint and iPhone deployment unchanged until every per-generator survival, proxy score, p10, zero-invalid, combo-break and avoidable-deadline gate passes on independently replicated, audited untouched seeds.

Automated research remains active and should prioritize one new, substantive gameplay hypothesis rather than repeating unsuccessful four-weight searches or shallow audits.
