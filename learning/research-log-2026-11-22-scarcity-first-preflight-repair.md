# Scarcity-first bounded retry: preflight regression repair — 2026-11-22

Research branch only. No main, iPhone v3.4, deployment, or validated checkpoint edits.

## Evidence from failed initial run

GitHub Actions run 45081923294 (workflow snapshot 675aead7) completed with failure at the fail-fast preflight, before training. Workflow consistency and raw-start audits passed for hazard 253/254, policy 255, and holdouts 256/257. The C compilation guard ran 10 tests; nine passed and `test_crowded_root_scarcity_order_is_active` errored with `ValueError: substring not found`. Root cause: the guard first strips all C comments but then searches for `#endif // BL_RESEARCH_FAIR_ROOT`, whose comment suffix has been removed. This is a Python test harness bug, not evidence of gameplay failure. Artifact 14777412668 and job logs preserve the failed preflight. All five previous IDs remain reserved.

## Targeted correction and new bounded attempt

Fixed the guard to search for the next `#endif` after the `search_fair_first` function in commit d46abcf8. The research strategy remains the existing scarcity-first visible-piece order under crowded-root fairness, compared against unchanged v1.7 SMART SURVIVAL. The source still compiles in the previous nine checks; the repaired test must pass in CI before training.

Verified no active jobs among the latest 100 repository Actions runs before triggering. Audited 367 documented reserved epochs. Chose fresh hazard train 283, validation 284, policy train 285, independent holdouts 288/289; five distinct 17..2000 IDs, absent from ledger. The source formula produces 570 unique proposed raw PRNG starts, disjoint from documented reserved starts across normal, heavy and shifted-heavy scenarios and all 38 checked game indices. Undocumented external/local historical use cannot be ruled out.

Updated only `.github/workflows/v22-tenx.yml` in immutable source commit 601baec2 to use the new five epochs. The path-specific push produced Actions run [45085777116](https://github.com/welcome23bask/BlockBlastAI/actions/runs/45085777116), observed in progress. Reserved epochs in the cross-role ledger afterward in separate commit 51343122, without modifying the executing workflow snapshot.

## Interpretation and follow-up

The new run was still in progress at last inspection. Check job logs for seed preflight, corrected guard, training and both paired holdouts. Require independent normal/heavy/shifted-heavy per-generator >=10x mean survival, synthetic estimated score and p10 survival, zero illegal/censored games, and nonworse combo breaks/deadlines in both holdouts; do not promote a checkpoint on failure. Synthetic score is not actual game score. Preserve all failures and diagnose the weakest generator for a different bounded research-only hypothesis. No independently reproduced 10x result established.
