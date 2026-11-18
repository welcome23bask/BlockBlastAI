# BlockBlastAI research iteration 81 — 2026-11-17

Research branch only. Fetched exact workflow .github/workflows/v22-tenx.yml, runner learning/run_tenx_reproducible.sh, cross-role seed ledger learning/tenx_used_epochs.txt, and iteration 80 log before acting.

GitHub Actions run 44193857840 completed with failure of strict 10x gates, but preflight, compilation, rules self-test, training and both independent holdouts 782/786 ran. Artifact 14377578333 preserves failed evidence. Trained isolated weights 120 120 30 -60 were NOT promoted to validated checkpoint.

Holdout 782 normal/heavy/shifted-heavy survival 1.7650x / 1.1492x / 0.9925x; score proxy 2.0093x / 1.3289x / 1.0273x; p10 1.0789x / 1.4118x / 0.9474x. Holdout 786 survival 1.4969x / 1.3479x / 1.1424x; score proxy 1.7498x / 1.7554x / 1.3892x; p10 0.7778x / 1.0000x / 1.2667x. Both have zero illegal and capped games, zero candidate avoidable misses, improved combo breaks. Synthetic proxy is NOT official gameplay score. The worst failure is shifted-heavy 782 survival/p10 below baseline; normal 786 p10 also regressed.

New research-only hypothesis, commit d02261421595b7a4c501a5ea4936c2d16db3e50d: even when a legal hard-piece pair exists, it may be dependent on a single fragile shifted-shape landing family. Add deterministic exact-legal shifted-shape landing-type redundancy floor for crowded completed trays in BLResearchCrowdedPairFailure; keep unchanged v1.7 reference, no hidden future RNG. This is distinct from the previous two-piece family-feasibility test, but uncompiled and untested; may worsen normal p10 and must be rejected if so.

Next bounded experiment: verify repository-wide zero active/queued/waiting/requested Actions runs; audit three unused epoch IDs 17..2000 against full ledger and accessible historical runs plus raw-start aliases, update ONLY research workflow .github/workflows/v22-tenx.yml to trigger its push workflow; ensure preflight passes before training. Do not reuse 780/782/786. Preserve artifact even on failed gates and reserve epochs AFTER executing workflow snapshot. Require strict >=10x mean survival AND synthetic score per generator, >=10x p10, zero illegal/censored, nonworsening combo breaks/deadlines and independent replication twice before any promotion or claim. External/local history may be incomplete.

No new Actions workflow launched this iteration because repository-wide overlap absence was not independently verified; no epochs newly reserved. Main, live v3.4 iPhone Automatic Play, deployments, unchanged v1.7 baseline and validated checkpoints remain untouched.
