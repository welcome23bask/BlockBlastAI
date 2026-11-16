# Cross-iteration evidence audit: strict 10x research 36–44

Date: November 16, 2026 UTC. Research branch `feature/v2.2-10x-research-gate` ONLY. This is a descriptive aggregation of **existing independent paired synthetic holdouts**, not a new head-to-head comparison of experimental policies. Different iteration cohorts use different seeds and must not be compared as if an A/B pair. All metrics are relative to the unchanged v1.7 SMART SURVIVAL **within their own** held-out epoch. Simulator-estimated scores are NOT the real game's score.

## Audited holdout mean survival ratios (holdout A / holdout B)

| Iteration / Actions run | Normal | Heavy | Shifted-heavy | Strict reason rejected |
| --- | --- | --- | --- | --- |
| 36 / [43802814921](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43802814921) | 1.6940× / 1.2613× | 1.1534× / 1.1180× | 1.2811× / 1.1303× | All means and scores far below 10×; shifted p10 1.0000× in A |
| 37 / [43803697188](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43803697188) | 1.4868× / 2.0006× | 1.1243× / 1.2941× | 1.1425× / 1.0489× | Shifted combo breaks 2.894 vs v1.7 2.166 per 100 in A; p10 1.0000× both |
| 38 / [43804331999](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43804331999) | 1.3823× / 1.1328× | 1.0628× / **0.9970×** | 1.0724× / 1.1797× | Heavy mean and estimated score below baseline in B; normal/shifted p10 regress in A; heavy combo regression in B |
| 39 / [43804845920](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43804845920) | **0.9917×** / 1.4496× | 1.1610× / 1.0969× | 1.0405× / 1.0221× | Normal mean regress in A, normal p10 0.8200× in B; shifted near 1× |
| 40 / [43805340214](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43805340214) | 1.3404× / 1.8203× | 1.0834× / 1.1610× | 1.0299× / **0.9278×** | Normal p10 0.8600× A; shifted mean/score/p10 regress B |
| 41 / [43805816034](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43805816034) | 1.2359× / 1.6377× | 1.2856× / 1.4863× | 1.1995× / 1.0710× | Normal p10 0.7321× A; shifted combo break slight regression in B |
| 42 / [43806245696](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43806245696) | 2.0630× / 2.2664× | 1.3939× / 1.2118× | 1.2319× / 1.2032× | Shifted p10 **0.9412×** B; all means and scores <10× |
| 43 / [43806703532](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43806703532) | No games | No games | No games | Source array indexing did not compile, artifact preserved; seeds 1843/1969/1970 reserved, **never reused** |
| 43b / [43806781521](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43806781521) | 2.2364× / 1.6652× | 1.6257× / 1.1870× | 1.1971× / 1.1250× | Shifted p10 **0.8333×** A; all 10× gates failed |
| 44 / [43807467128](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43807467128) | 3.2370× / 2.0571× | **1.0164× / 1.0665×** | 1.2894× / 1.2402× | All means and scores <10×; no stage-one training candidate passed combo screening |

## What the evidence does and does not show

The normal generator can show strong *synthetic* mean survival gains on some seed cohorts, but heavy and shifted-heavy remain the critical bottleneck. Hard-shape and near-future-clear bonuses can trade high average survival for poor p10 or combo break regressions. No iteration has achieved 10× mean survival, 10× estimated score, and 10× p10 simultaneously in *every* generator on *both* holdouts with nonworsening combo safety. Independent 10× replication is therefore NOT established. Training self-play progress, a favorable holdout cohort or a missing illegal move is not success.

Notably, run 44's training stage used only five paired games/generator and screened **0/21** candidates as combo safe even though its two 30-game/generator holdouts recorded improved combo-break rates. This is a **hypothesis** of high small-sample selection variance, not proof that failed candidates were safe. Better follow-up: compare a small predeclared learned continuation-value or worst-generator lower-tail policy set with more training games per candidate under the same total CI budget; continue to reject on unchanged strict heldout thresholds. See `learning/research-note-2026-11-16-model-selection-seed-variance.md`.

The synthetic generator itself has **different known source-level marginals** for normal, heavy and shifted-heavy; see `learning/research-audit-2026-11-16-synthetic-generator-priors.md` and the non-promoted `learning/audit_synthetic_future_risk.py` diagnostic. A replay-augmentation proposal must pass the actual shape-catalog symmetry audit: `learning/audit_shape_symmetry.py` shows the 28-shape set is not D4-closed, so naive 8-way augmentation would change the synthetic training distribution. The next distinct long-horizon research hypothesis is outlined in `learning/research-plan-2026-11-16-learned-survival-value.md`.

## Provenance and invariants

All above runs had artifact retention, including the compile failure; 9 produced two complete paired 30-game/generator holdouts each. Earlier logs 36–44 contain the per-generator survival, p10, synthetic estimated score, combo rates, illegal/censored counts and failure diagnoses. Ledger `learning/tenx_used_epochs.txt` was audited after run44: 90 distinct numeric entries, no duplicates, and **every** known train epoch 1836–1845 plus holdout epoch 1955–1974 is present. Its historical coverage does not prove undocumented external/local runs are absent. The immutable baseline source remains `src/BLDualStrategy.c`; original research runner SHA `3441e2af31463655d380b3d27a90026d5a694a71`, checkpoint `16 120 120 30 60`, and saved candidate weights `120 120 30 60` remain unchanged. The strict script checks survival≥10, estimated score≥10, p10≥10, 0 illegal/censored and nonworsening combo breaks/deadline misses on both holdouts; no relaxations, no saved checkpoint promotion, no main/live v3.4 edits, no iPhone builds.
