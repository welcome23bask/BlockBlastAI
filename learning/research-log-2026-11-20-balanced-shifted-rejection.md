# 2026-11-20: shifted-irregular lower-tail trial rejected, first candidate restored

Branch: `feature/v2.2-10x-research-gate` exclusively. This log documents **synthetic research evidence**, not actual Block Blast gameplay or score. Never merge/deploy to main/live v3.4 or promote checkpoint from these failed gates.

## Tested hypothesis
After the disjoint-3×3 late rerank improved mean moves relative to unchanged v1.7 but shifted-heavy holdout 599 p10 was only 1.0526×, implemented `BLResearchBalancedShiftedRerank()` to reward the weaker exact landing count among two irregular 3×3 shape families (masks 0x40603 and 0x20303) **only** where two non-overlapping legal 3×3 placements remain and the full tray afterstate has 18–31 vacant cells. Added to normal and combo-preserving late rerank, not baseline or production. Experiment source cdd4a101, regression 835d48e1.

## Trial and evidence
- GitHub Actions run **44827375236**, immutable workflow `f138039f`; preflight and compiler regression passed, full simulator hazard pipeline ran. Artifacts **14674428567** retained.
- Five new cross-role epochs: hazard 675/676, policy train 677, independent 30-game/generator paired holdouts 678/679. Ledger reservation **07efbf5d**, after workflow snapshot. 570 unique observed raw PRNG starts against 307 documented epochs; external/local history incomplete.
- Hazard model calibration diagnostic: `research_only_validation_diagnostic_fail`. This is not a gameplay gate pass.
- Bounded training stage two: 3/4 weight candidates passed eight-game/generator internal safety evaluation; an **isolated** candidate `60 120 90 60` was selected in the temporary training workspace. It was **NOT promoted** to the repository or validated checkpoint because holdouts failed.
- Independent results versus unchanged v1.7:

| Synthetic generator | Holdout 678 survival / estimated score / p10 | Holdout 679 survival / estimated score / p10 |
| --- | --- | --- |
| Normal | 1.8920× / 2.2464× / 1.1277× | 2.0713× / 2.5015× / 2.2143× |
| Heavy | 1.2856× / 1.5819× / 1.2143× | **1.0629×** / 1.1551× / 1.0625× |
| Shifted-heavy | 1.3227× / 1.6826× / **0.9500×** | 1.1834× / 1.4478× / 1.0625× |

All six comparisons had zero illegal moves and zero censored games, improved combo breaks/move and no worse avoidable combo deadlines. **Strict 10× failed on all six comparisons**, and shifted-heavy p10 actually regressed below unchanged v1.7 in holdout 678 (19 vs 20 moves). This new policy is not a dependable lower-tail improvement; cross-run seed changes also prevent attributing differences directly to this feature.

## Safe action
The rejected shifted-irregular rerank and its test were **reverted to the exact source files of previous immutable workflow 3ee69e95** on this research branch: source commit ed6c526c, guard commit 60a2a81f. The earlier **disjoint-square late-rerank** candidate remains the current research working hypothesis, backed by two independent positive-v1.7 synthetic holdouts 598/599 in artifact 14674736849, but it still fails 10× and is not an independently proven improvement over earlier research variants. Failed code remains retrievable from immutable cdd4a101 and artifact; no evidence erased. No checkpoint, main, live v3.4, phone build or deployment changed.

Next research-only focus: investigate actual short-episode shifted-heavy failures and the simulator’s heavy-tray geometry before changing another coefficient. Avoid repeating any settings/seed combination. Maintain exact legal current-tray search, training-only hazard labels, unchanged v1.7 baseline, independent fresh normal/heavy/shifted-heavy holdouts, strict gates and censor/legality checks.
