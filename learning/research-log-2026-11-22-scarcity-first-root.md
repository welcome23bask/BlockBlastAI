# Scarcity-first crowded-root search — 2026-11-22

Research-only change on feature/v2.2-10x-research-gate; no live v3.4, main, checkpoint, merge or deployment modifications.

## Evidence and hypothesis
Prior bounded Actions 45054914237 completed with failed strict gates; heavy holdout 228 had mean survival 1.0296x and synthetic estimated score 1.1536x. Known job 137307889194 is completed (conclusion failure). Conditional crowded-board fair-root allocation was introduced in 21db004e but has not yet been benchmarked. New hypothesis bc7d462e: within that occupancy-conditional allocation, enumerate scarce legal current-tray piece types first, sorting three visible pieces by number of exact legal placements (positive counts ascending, zero last). This may reduce heavy-generator search starvation; it does not inspect hidden future RNG, change legality, change v1.7 or enlarge the node budget. Source guard 89c2087e checks ordering and budget restoration.

## Execution boundary
Fetched exact .github/workflows/v22-tenx.yml, learning/run_tenx_reproducible.sh, learning/tenx_used_epochs.txt, research source and prior research log. Current workflow still references reserved 206/207/209/224/228 and MUST NOT be triggered unchanged. GitHub connector provides jobs for known runs but no reliable repository-wide Actions run listing, so repository-wide non-overlap could not be certified. No new workflow run was launched, no seed epochs consumed, and no compile/benchmark result is claimed. This is a new strategy implementation and regression guard, not yet a completed training-and-simulation comparison.

## Next bounded trial
After independently verifying repository-wide Actions non-overlap, audit five fresh epochs in 17..2000 against cross-role ledger, accessible Actions and raw RNG alias derivation; document external/local-history uncertainty. Update every workflow seed occurrence together and trigger the workflow via its YAML push trigger. Verify preflight before training. Run fresh paired 8x8 three-piece normal/heavy/shifted-heavy benchmarks against unchanged v1.7 with two independent holdouts, preserving artifacts. All strict >=10x mean survival, synthetic estimated score and p10 gates per generator, zero illegal/censored and nonworsening combo breaks/deadlines must pass twice before any success or promotion. Synthetic score is not actual app score.
