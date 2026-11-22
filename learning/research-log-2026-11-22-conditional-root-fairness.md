# Conditional crowded-board root fairness — 2026-11-22

Research branch only; no live v3.4, main, deployment, checkpoint or unchanged v1.7 changes.

## Failure diagnosis
Previous bounded CI 45054914237 completed hazard learning, training, and independent paired holdouts 224/228 but failed every strict 10x gate. Weakest was heavy holdout 228 at mean survival 1.0296x and synthetic estimated score 1.1536x; zero illegal/capped, no worse combo deadlines. Artifact 14767640877 preserves evidence. Global fair-root search was previously rejected on shifted-heavy 1943/1944; this is a different, occupancy-conditional ablation, not a repeat of those settings.

## New bounded hypothesis
Commit 21db004e enables the existing exact-legal root quota allocator only when the current observable board has fewer than 20 empty cells; for all other boards it retains depth-first traversal. The same global node budget is used, and neither next-tray RNG nor scenario label is inspected. Hypothesis: on critically crowded boards, early root monopolization can omit rare complete three-piece escape routes, while global fairness wastes budget on easier boards. Guard commit cbcec5d verifies conditional source activation and budget restoration. This is a candidate experiment, not a measured improvement.

## Execution and constraints
Fetched the exact .github/workflows/v22-tenx.yml, learning/run_tenx_reproducible.sh and learning/tenx_used_epochs.txt from feature/v2.2-10x-research-gate. Workflow currently still references already-reserved hazard 206/207, policy 209, holdouts 224/228 and MUST NOT be run unchanged. Known run 45054914237 has completed. Available connector exposes job listing for known run IDs, not a reliable repository-wide Actions run list, so absence of overlapping jobs could not be established. No workflow edit or experiment trigger was attempted, and no seed IDs were consumed. Source/test changes are uncompiled and unbenchmarked. External/local seed history remains incomplete.

Next safe bounded trial: verify repository-wide Actions non-overlap; audit five fresh cross-role epochs and all raw PRNG aliases against historical ledger and accessible Actions; update every workflow seed occurrence consistently; pass five-role preflight before training; compare unchanged v1.7 and conditional-fairness candidate on two independent fresh paired normal/heavy/shifted-heavy 8x8 three-piece holdouts. Preserve all failed metrics and artifacts; require per-generator >=10x mean survival, >=10x synthetic estimated score (NOT actual game score), >=10x p10, zero illegal/censored, nonworsening combo breaks and avoidable deadlines, and independent replication before any success claim or checkpoint promotion.
