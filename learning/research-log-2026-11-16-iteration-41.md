# Iteration 41 — distributionally robust future-piece accessibility model (bounded research)

Date: 2026-11-16 UTC. ONLY branch `feature/v2.2-10x-research-gate`; unchanged live v3.4 iPhone, main, checkpoint, deployment.

## Prior evidence and novel hypothesis

The baseline-anchored backup on [run 43805340214](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43805340214) did not help worst-case shifted-heavy survival: 0.9278x survival / 0.9335x proxy score / 0.9412x p10 on holdout 1964, plus normal p10 0.8600x on holdout 1963. Zero of 21 initial weight candidates passed combo-safety. Reject the optional baseline-anchored shield by default; older failed bottom-three, dual finalist-pool, fixed future-tray rollout and proactive override all remain OFF.

The previous single-minimum hard-shape bonus also poorly captures the distribution of **all** possible synthetic future pieces. The new research-only feature `BL_RESEARCH_SHAPE_MIXTURE=1` in source commit `0659edc705cba8dea4cec3ccca05aab4fdd1804a` computes legal placement counts for all **28 documented synthetic shape templates** (matching `learning/learn_combo_guarded.c`) from a *completed current-tray* afterstate. A robust positive mixture weight covers every normal shape and gives additional mass to known synthetic heavy/shifted-heavy shape families, without peeking at the RNG or learning from withheld games. Candidate afterstate penalty for unavailable piece types and single-landing types is compared among the original four finalists; this is NOT the earlier rejected bottom-three risk bonus on all search leaves, nor the overfitted hypothetical next-tray rollout. Distributional model only influences research afterstate selection; original v1.7 solver, legality, combo safety and all threshold gates unchanged. Regression selftest commit `c3b6f31d104ef84c16b25f31a66912495f050be4` checks finite empty/full board accessibility and monotonicity with removing blocked cells. The fixed rarity weights are synthetic heuristic assumptions, NOT the actual proprietary piece probabilities or true score.

## Workflow experiment and cross-role epochs

[GitHub Actions run 43805816034](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43805816034) triggered by workflow-path commit `099d018e5095c848b365cf1f39fd4e7e0033489b`, after exact fetch and review of `.github/workflows/v22-tenx.yml`, runner `learning/run_tenx_reproducible.sh`, and cross-role ledger `learning/tenx_used_epochs.txt`; no other Actions run active at trigger time. Fresh training epoch **1841**, independent holdouts **1965** and **1966**, disjoint, not in accessible historical ledger or completed Actions. External/local undocumented use cannot be excluded. GitHub runner checkout and seed preflight completed successfully; only then reserved all three epochs in ledger commit `b0a3c35f01e602c0885429be906125f807e0e93b`, without changing the executing snapshot. Bound: 21 candidates x five paired games per generator, up to four stage-two finalists x eight *disjoint training* games per generator, two independent 30-game/generator paired holdouts. Job timeout 12 min. Evidence artifacts retained on any gate failure. No checkpoint promotion.

## No-relax 10x acceptance

All normal/heavy/shifted-heavy generators on both holdouts must independently achieve ≥10x mean survival, ≥10x **synthetic estimated score**, ≥10x p10 survival, zero illegal moves, nonworsening combo-break rates and avoidable deadlines, zero 4096-capped games, and independent replication before any success claim. Synthetic estimated score is **NOT actual Block Blast game score**. No main/phone/deployment/validated-checkpoint changes. Pending verdict at log creation.

## Completed result — no universal 10× and no candidate promotion

[Actions run 43805816034](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43805816034) passed source compilation, exact three-piece rule selftests, fresh seed preflight and completed two independent 30-game/generator holdouts. Twenty of twenty-one weight variants failed safety screening; the sole survivor failed the independent eight-game stage-two training split. The holdout artifact `blockblast-tenx-bounded-research-43805816034` is saved. None was promoted.

| Generator | 1965 mean survival / estimated score / p10 | 1966 mean survival / estimated score / p10 |
|---|---|---|
| Normal | 1.2359× / 1.3843× / **0.7321×** | 1.6377× / 1.9727× / 1.5610× |
| Heavy | 1.2856× / 1.5410× / 1.3333× | 1.4863× / 2.0417× / 1.2353× |
| Shifted-heavy | 1.1995× / 1.4405× / 1.1765× | 1.0710× / 1.2031× / 1.3571× |

All reported moves legal, no censored games or additional avoidable combo deadline misses. However 1966 shifted-heavy combo breaks per 100 moves were **2.772 vs v1.7's 2.757**, disqualifying under zero-regression gate; 1965 normal p10 collapsed to 41 from baseline 56 (0.7321×). Per-generator 10× gates failed twice. Broad future-shape landing coverage can aid heavy pieces but hurts normal lower-tail and a shifted-heavy combo comparison. **Reject as default**; retain opt-in research flag only. Next hypothesis should favor *immediate future clear opportunities*, rather than raw fit counts across shapes, while retaining exact combo-safety checks and unchanged gates. Synthetic estimated score is not game points. Main, v3.4 iPhone and saved checkpoint were never touched.
