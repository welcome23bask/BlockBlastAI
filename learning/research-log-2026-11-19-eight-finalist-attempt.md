# Research attempt — 2026-11-19, eight exactly legal finalists

Branch: `feature/v2.2-10x-research-gate` only. No main, live v3.4, iPhone build, deployment, or validated checkpoint edits.

## Prior verified failure: Actions 44556407939 (workflow f2537b75)

Seed preflight passed (strategy train 77, holdouts 78/79; hazard train/validation 75/76), Python hazard pipeline passed, two independent 30-game-per-generator paired holdouts completed. Synthetic estimated score is NOT the official Block Blast score.

| Holdout | Generator | Mean survival × | Synthetic estimated score × | p10 survival × | Candidate combo breaks/100 vs v1.7 | Illegal/censored |
|---|---|---:|---:|---:|---|---|
| 78 | normal | 2.1233 | 2.4648 | 1.6207 | 1.215 vs 1.957 | 0/0 |
| 78 | heavy | 1.7265 | 2.4798 | 1.0625 | 2.391 vs 3.509 | 0/0 |
| 78 | shifted-heavy | 1.2716 | 1.5631 | 1.0625 | 1.744 vs 2.550 | 0/0 |
| 79 | normal | 1.7790 | 2.0211 | 1.8864 | 1.319 vs 2.346 | 0/0 |
| 79 | heavy | 1.3961 | 1.7906 | 1.2500 | 1.985 vs 3.464 | 0/0 |
| 79 | shifted-heavy | 1.0890 | 1.1618 | 1.1765 | 1.782 vs 2.740 | 0/0 |

Avoidable combo misses: zero for all candidate scenarios in both holdouts; v1.7 normal holdout 79 had 3. Every strict 10× gate failed. Weakest mean survival and score: shifted-heavy holdout 79. Training-stage incumbent normal p10 also regressed (0.4082×), and heavy combo breaks were worse in training, so no candidate/checkpoint promotion. All evidence preserved by workflow artifact upload. Epochs 75–79 reserved in cross-role ledger commit 8875e694. External/local history remains incomplete.

## New bounded hypothesis and execution

Change ONLY research strategy `learning/BLTrainableStrategy.c`: increase complete-tray exactly legal finalists from six to eight (source commit 52110b8f). Hypothesis: shifted-heavy lower-tail failures reflect rare legal irregular-shape escape afterstates being pruned before the deterministic geometry reranker; no hidden future RNG. This is a search-space ablation, not coefficient retuning, hazard-only diagnostics, or a production change.

Workflow commit d9ad9bdb uses hazard train/validation epochs 900/901, strategy training epoch 902, paired holdouts 903/904. All five are distinct, 17..2000, absent from the accessible ledger before execution, and do not have 20/40-epoch spacing aliases against the accessible historical ledger or each other. The existing runner checks cross-role ledger and raw PRNG starts before compilation. No overlapping workflow runs were present when the experiment was triggered. Actions run [44572445973](https://github.com/welcome23bask/BlockBlastAI/actions/runs/44572445973) was observed **in progress**, not yet a verified training or holdout result. Epochs 900–904 were conservatively reserved in ledger commit f31ed649 after verifying the immutable executing workflow snapshot; this later ledger update cannot change the running checkout's seed set.

Strict unchanged-v1.7 per-generator survival, p10, synthetic score, combo, deadline, legality and censor gates remain unchanged, with independent holdout replication required. No improvement or 10× result is claimed. Inspect the Actions logs/artifacts after completion, diagnose worst generator, and implement a different targeted research-only hypothesis on fresh seeds if the strict gates fail.
