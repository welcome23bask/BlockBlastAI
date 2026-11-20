# 2026-11-20 — Twelve legal complete-tray finalists: strict paired evaluation

Research branch `feature/v2.2-10x-research-gate` only. No phone build, main edit, merge, deployment, or validated checkpoint change.

## Hypothesis
Increase `BL_FUTURE_TOP` from 8 to 12 distinct **legally reached** complete-tray afterstates in `learning/BLTrainableStrategy.c`, with existing crowded disjoint-3×3-square late rerank enabled. More legal move alternatives might preserve heavy/shifted-heavy escape routes otherwise excluded by pure immediate-board shortlist. This does not peek at unseen game pieces or RNG and does not modify unchanged v1.7 baseline.

Source commit dde34a5d, regression correction 8ebee65c. First CI attempt **44835988126** (hazard 735/736, train 737, holdouts 738/739) passed consistent five-role seed/raw-start audit but **failed before training** because the new Python guard had an unterminated string literal. Never replay these reserved epochs. Correction uses an exact macro-line assertion.

## Completed trial
- [GitHub Actions 44836107763](https://github.com/welcome23bask/BlockBlastAI/actions/runs/44836107763), immutable workflow b491613c, artifact **14679296378**. Five roles hazard 767/768, policy train 769, independent paired holdouts 770/771. Seed preflight and 570 raw starts passed against 322 documented reservations; these five epochs reserved **after** executing snapshot in ledger commit 33f49c0c. External/local historical seed coverage remains incomplete.
- C regression and simulator legal-rules tests passed. Simulator-derived hazard pipeline completed, but **hazard calibration diagnostic failed** (`research_only_validation_diagnostic_fail`).
- Training incumbent on 5 games/generator (not validation): normal mean survival 2.8062×; heavy 1.8041×; shifted-heavy 1.0124×. All four candidate weight proposals failed the independent training-stage regression gate (`verified_safe=0`); isolated weights remained `120 120 30 60`, no promotion.

| Scenario | Holdout 770 survival / synthetic estimated score / p10 | Holdout 771 survival / synthetic estimated score / p10 |
| --- | --- | --- |
| Normal | **2.2962× / 2.8381× / 1.8788×** | **2.0109× / 2.4307× / 1.9429×** |
| Heavy | 1.3924× / 1.7554× / 1.3158× | 1.4121× / 1.8076× / 1.1765× |
| Shifted-heavy | 1.1697× / 1.4219× / 1.2000× | 1.0843× / 1.1536× / **1.0000×** |

All six comparisons had **zero illegal moves, zero right-censored games, no worse avoidable combo deadlines, and lower combo breaks/move**. These are *synthetic* simulations, not actual Block Blast scores. **The requested 2× mean survival is replicated on normal only, NOT on heavy or shifted-heavy; no strict 10× mean/score/p10 gate passed across all generators.** No checkpoint promotion.

## Diagnosis and next bounded hypothesis
Simply widening the existing quality-ordered shortlist lets normal moves exceed 2× v1.7, but does not preserve rare heavy and shifted-piece escape opportunities in the **lower tail**, especially holdout 771 shifted-heavy p10 19 versus baseline 19. Next research-only hypothesis: explicitly diversify crowded **legal finalist admission** by hard-piece family feasibility signatures, allowing a near-quality alternate with genuinely different square/bar/irregular landing accessibility to displace a redundant top-list finalist. Unlike a raw all-shape score or obsolete dual risk pool, this changes candidate *coverage* using only current-board legal geometry. Must test on fresh five-role historically audited epochs and independent paired normal/heavy/shifted-heavy holdouts; strict gates unchanged.

Checkpoint still `16 120 120 30 60`; saved candidate weights `120 120 30 60`. No guaranteed hourly gains.
