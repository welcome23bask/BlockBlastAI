# BlockBlastAI research iteration 84 — 2026-11-17 US/Central

Scope: ONLY feature/v2.2-10x-research-gate. Exact workflow .github/workflows/v22-tenx.yml, runner learning/run_tenx_reproducible.sh, and cross-role ledger learning/tenx_used_epochs.txt fetched and inspected. Prior iteration 83 log inspected.

Actions run 44233782998 completed with conclusion failure, but seed preflight, compilation, selftest, bounded training and both independent holdouts succeeded. Preserved artifact: https://github.com/welcome23bask/BlockBlastAI/actions/runs/44233782998/artifacts/14395719262 . Training epoch 300, holdouts 301/302 (already reserved). Stage 2 had 0/4 safe finalists and did not promote any weights. Training checkpoint in isolated runner became logical epoch 17 with unchanged weights 120 120 30 60; validated repository checkpoint remains 16 120 120 30 60.

Holdout 301 mean survival normal/heavy/shifted-heavy = 2.2588/1.3758/1.0400x; synthetic estimated score = 2.6148/1.5422/1.0551x; p10 = 2.4571/1.1765/1.1111x.
Holdout 302 mean survival = 1.6107/1.1907/1.2678x; synthetic estimated score = 1.8009/1.3696/1.4919x; p10 = 1.1892/1.0000/1.2143x.
All six comparisons zero illegal moves and zero censored games, no worse combo-break rates and no worse avoidable combo misses. Strict 10x gates failed everywhere. Worst generator shifted-heavy 301, survival 1.0400x and synthetic estimated score 1.0551x, nearly baseline. Synthetic estimated score is NOT actual game score.

Targeted distinct UNTESTED hypothesis commit fea386497ef0c2d78b43db71b9f2c45d7767acf1: avoid stacking the 650 shifted-family penalty on top of no-pair floor when one shifted shape still fits. Apply extra penalty only when all three tested shifted shapes have zero legal landing and no exact two-piece escape family survives. Changes logical predicate rather than retuning weights; no hidden RNG at inference. This is NOT evidence of improvement and may regress.

No new Actions experiment triggered: repository-wide active/queued verification unavailable via connector; direct github.com DNS lookup failed. Do not reuse 300/301/302 or any ledger IDs. Next safe cycle: independently confirm zero active runs, audit 3 fresh train/holdout epochs against ledger, accessible Actions and raw-seed alias auditor, trigger via exact workflow-path push, verify preflight before compilation, preserve artifacts, assess strict per-generator gates twice. External/local history remains unverifiable. No live v3.4 iPhone code, main, deployment, unchanged v1.7 baseline, or validated checkpoint changed.
