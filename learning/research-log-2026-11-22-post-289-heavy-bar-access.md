# Post-289 research triage — 2026-11-22

Research branch only. No main, v3.4 iPhone, deployment, or validated checkpoint modifications.

## Verified bounded experiment
GitHub Actions run 45085777116, source snapshot 601baec2, completed with failed strict gate. Preflight and C regressions passed; hazard simulator exported 1122 records. Hazard calibration diagnostic status was research_only_validation_diagnostic_fail (no policy integration). Policy training epoch 285 rejected all four stage-two candidates as regressions, with no isolated weight promotion. Independent 30-game-per-generator paired holdouts 288 and 289 completed against unchanged v1.7. Synthetic estimated score is not actual game score.

Holdout 288 normal mean survival 1.8039x, estimated score 2.1200x, p10 1.3023x; heavy 1.2452x / 1.4931x / 0.9583x; shifted-heavy 1.1865x / 1.3139x / 1.3333x.
Holdout 289 normal 1.9794x / 2.3100x / 0.9302x; heavy 1.3259x / 1.6288x / 1.4706x; shifted-heavy 1.2160x / 1.4430x / 1.1429x.
Both holdouts had zero illegal moves, zero censored games, improved combo-break rates and no worse avoidable deadlines. All strict 10x gates failed; heavy and shifted-heavy mean survival remain near baseline and heavy p10 regressed on 288, normal p10 on 289. Evidence artifact 14778789272. No checkpoint promotion.

## New targeted research-only hypothesis, NOT yet benchmarked
Commit 34414166 requires at least one exact legal horizontal OR vertical five-cell bar landing before granting crowded-board irregular-finalist admission bonuses. Hypothesis: scarce heavy-bar access is displaced by irregular-only finalists and contributes to heavy-generator lower-tail deaths. This is a geometry-only admission gate; no hidden RNG, search budget increase, baseline change, or coefficient retuning. Commit c0c175f updates the compilation guard to require the active predicate. Neither source change has passed CI or a fresh paired benchmark.

Do not reuse epochs 283/284/285/288/289, which are reserved in the cross-role ledger. Before any new workflow trigger, verify repository-wide Actions non-overlap, audit five fresh nonaliasing epochs and historical runs, run preflight, and preserve both strict independent paired holdouts. External/local history remains incomplete. Do not promote unless every strict per-generator gate passes twice.
