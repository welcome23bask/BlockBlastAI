# 2026-11-20 — Hard-family signature-diversity experiment rejected

Research branch `feature/v2.2-10x-research-gate` only. Unchanged v1.7 SMART SURVIVAL baseline and strict 10× mean/synthetic-score/p10 per-generator gates were preserved. No live v3.4 iPhone, main, deployment, build, or validated-checkpoint mutation.

## Gameplay hypothesis
Following completed twelve-finalist trial 44836107763, test whether crowded legal three-piece afterstates would avoid low-tail heavy deaths when a near-quality candidate with a **new six-family exact legal fit signature** could displace a redundant shortlist candidate. Evaluated exact availability of square, horizontal/vertical 5-bar, two shifted irregular 3×3 geometries and an additional shifted irregular family; no hidden future-tray RNG. Candidate acceptance bounded to 240 baseline utility, with disjoint-square rerank still active. Source commit **3a5d9fd9**; regression **8158fa33**.

## Completed independent experiment

- [GitHub Actions 44837049501](https://github.com/welcome23bask/BlockBlastAI/actions/runs/44837049501), immutable workflow `105a502e`, evidence artifact **14679192465**.
- Fresh cross-role epochs: hazard train **794**, hazard validation **795**, policy train **796**, holdouts **797** and **798**. Passed seed consistency and audited 570 distinct raw PRNG starts against 327 documented reservations; reserved all five after verified queued snapshot in ledger commit `40e0dae6`. External/local historical use is still incompletely observable.
- Source compiler regression, exact row/column clearing legality selftest, and independent 30-game/generator paired holdouts completed. Hazard model calibration diagnostic `research_only_validation_diagnostic_fail`, not an improvement.
- All four candidate weight proposals failed secondary 8-game/generator safety-regression validation (`verified_safe=0`), so isolated weights stayed `120 120 30 60`; no checkpoint promotion.

| Scenario | Holdout 797 mean survival / synthetic estimated score / p10 | Holdout 798 mean survival / synthetic estimated score / p10 |
| --- | --- | --- |
| Normal | 1.7656× / 1.9897× / 1.5862× | 1.5871× / 1.8481× / **0.9737×** |
| Heavy | 1.2320× / 1.5040× / **0.8500×** | 1.3939× / 1.8132× / 1.1765× |
| Shifted-heavy | 1.3275× / 1.6857× / 1.1429× | 1.1484× / 1.4151× / 1.0000× |

Zero illegal moves and capped games in all six comparisons, no worse avoidable combo deadlines, improved combo breaks per move. **Strict 10× fails all six comparisons; requested 2× mean survival fails every comparison in this candidate.** Normal p10 regressed below v1.7 in 798, and heavy p10 in 797. Estimated score is synthetic, NOT actual Block Blast score, and independent seeds differ from prior run so apparent differences cannot be attributed causally without same-seed ablation (forbidden reusing official holdout seeds; can use a separate fresh paired A/B experiment).

## Safe rollback and follow-up
Rejected signature-diversity code without promoting it. Restored exact earlier tested source of workflow `b491613c` on research branch in commit **8ec6af14** and its matching Python regression guard in **62389cf3**. The previously measured 12-legal-finalist hypothesis remains the current research candidate: >2× mean survival on normal generator in both fresh holdouts 770/771, but only 1.39–1.41× heavy and 1.08–1.17× shifted-heavy. No independently reproduced full 2× across all generators, no strict 10× and no live gameplay validation. Previous checkpoint remains `16 120 120 30 60`, candidate weights `120 120 30 60`.

**Next targeted investigation:** shorten failure analysis to actual synthetic short episodes and compare accessible heavy/shifted 3×3 placements around the terminal board; study the unintegrated trained hazard model and calibration failures before applying further heuristic weights. Any next bounded trial must use fresh five-role epochs, verify no overlapping Actions, audit raw seed aliases, and preserve full per-generator mean/p10/estimated score, combo, legality and censor gates.
