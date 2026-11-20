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
