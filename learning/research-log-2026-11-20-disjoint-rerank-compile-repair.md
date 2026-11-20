# 2026-11-20: research-only disjoint-square rerank and compiler guard repair

User requested a real gameplay improvement and removal of experiment blockers. Research branch `feature/v2.2-10x-research-gate` only; live iPhone v3.4, src/ unchanged-v1.7 baseline, main, checkpoints and deployments untouched.

## Gameplay hypothesis

On crowded completed three-piece afterstates, prioritize TWO *disjoint* legal 3×3 square landing pockets in the late reranker, for both general and combo-preserving finalist pools. This differs from the earlier *admission-only* square gate: a candidate with multiple overlapping fits can still strand the next large piece. No future piece RNG, hidden tray, or illegal-placement shortcut. Added as a bounded strategy candidate in `learning/BLTrainableStrategy.c` (first commit 42c8f336, corrected 4e1752dd).

## Actions 44825843500: failure before gameplay

- Workflow snapshot 8b8095dd; hazard 509/510, policy train 511, independent heldouts 512/513; epochs reserved after immutable workflow snapshot in ledger commit 4bc4ea60.
- Workflow epoch consistency passed, independent raw-seed audit passed (570 distinct starts vs 292 documented reservations), and first three prior regression checks passed.
- C compilation regression FAILED because `BL_RESEARCH_DISJOINT_SQUARE_PAIR` was defined as both 1 (new) and 0 (older existing flag), which `-Werror` rejected. Training and holdouts were **not run**. Preserve artifact 14673452953 and failed run evidence. This is not evidence of gameplay success or failure.
- Fixed the duplicate definition, leaving one enabled flag, in commit 4e1752dd. Added an explicit unique-enabled rerank regression in `learning/test_irregular_admission_compilation_guard.py`, commit 05b4a0e2.

## Next bounded experiment

Verify repository-wide no active Actions, audit fresh five-role epochs against documented ledger and raw PRNG starts (external/local history incomplete), update ALL workflow epoch references, and trigger only by a workflow-file edit. Run compiler regression then simulator-derived hazard model and policy training, followed by two fresh 30-game/generator normal/heavy/shifted-heavy paired holdouts against unchanged v1.7. Preserve both positive and negative metrics; strict >=10× mean survival, synthetic estimated score and p10, zero illegal/censor, no worse combo/deadline, two independent replications before any success claim. No checkpoint promotion.

## Complete run chronology and measured result

- Actions 44826084272 / workflow 022b6357: five-role seed audit (515–519) and C compile regression passed, but a newly added Python regex guard had accidentally double-escaped its raw-string pattern, giving false failure. No training/holdouts. Artifact 14673623133. Fixed that guard in c42417b5. Epochs 515–519 remain reserved.
- Actions 44826256817 / workflow 3ee69e95: fresh hazard 595/596, policy train 597, independent paired holdouts 598/599. Full seed preflight, C and Python regressions, simulator-derived hazard training/validation (calibration diagnostic PASS), legality selftest, one policy training epoch, and two 30-game/generator paired holdouts all **executed**. Artifact 14674736849 contains training and holdout logs/metrics. Epochs 595–599 reserved AFTER immutable snapshot in c59dc5bf.

| Scenario | Holdout 598 mean survival / synthetic estimated score / p10 | Holdout 599 mean survival / synthetic estimated score / p10 |
| --- | --- | --- |
| Normal | 1.8455× / 2.1791× / 1.4151× | 1.6570× / 1.8931× / 1.6207× |
| Heavy | 1.2218× / 1.6106× / 1.2105× | 1.6062× / 2.0964× / 1.5333× |
| Shifted-heavy | 1.2191× / 1.4751× / 1.2500× | 1.2537× / 1.5064× / 1.0526× |

Both holdouts had zero illegal and capped games; no worse avoidable combo deadline misses (candidate zero in all six comparisons); fewer combo breaks/move for each generator. **Strict 10× failed on every generator/holdout**. No candidate weights promoted; source checkpoint 16 120 120 30 60 unchanged. The candidate is better than unchanged v1.7 on these fresh simulator seeds, but seed variation means this is NOT a controlled attribution against the prior disjoint-square admission-only candidate and is NOT actual game score evidence. Stage-two four weight candidates failed internal regression safety; isolated run left weights 120 120 30 60.

Worst gate: shifted-heavy holdout 599 p10 only 1.0526× (20 vs v1.7 19 moves), and heavy holdout 598 mean only 1.2218× (43.333 vs 35.467 moves). Next hypothesis should specifically protect alternate shifted-irregular exit families in the *late rerank* while retaining two disjoint 3×3 landing pockets, rather than globally increasing square preference. Test only with FRESH five-role epochs, do not reuse any failed seeds. External/local historical run coverage incomplete.
