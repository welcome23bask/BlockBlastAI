# Heavy-bar-access bounded experiment — 2026-11-22

Research branch only; no live v3.4, main, deployment, iPhone build, or validated checkpoint change.

Prior verified failed run 45085777116: heavy mean survival 1.2452x / 1.3259x and heavy p10 0.9583x on holdout 288; normal p10 0.9302x on holdout 289. Synthetic estimated score is not actual game score. All strict 10x gates failed. Evidence artifact 14778789272.

Targeted new unbenchmarked strategy from source commit 34414166, guarded by c0c175f: require exact legal horizontal or vertical five-cell bar landing before crowded-board irregular finalist admission bonus. Hypothesis: preserving hard-bar feasibility reduces heavy lower-tail deaths. This run is the FIRST bounded simulation attempt of that new geometry condition; unchanged v1.7 SMART SURVIVAL paired baseline.

At pretrigger audit, GitHub repository-wide Actions first page (100 runs) showed no active runs. 372 ledger epochs documented; raw seed formula 163000000 + 20000000*scenario + 1000000*epoch + 13717*game modulo 2^32. Selected hazard training 344, hazard validation 345, policy training 346, holdouts 348 and 349: 570 unique raw starts, no collisions against 372 documented epochs. Unrecorded external/local history cannot be excluded.

Workflow snapshot cb856dcfc0748326d9e1d783ee28ee463e285429 changed ONLY .github/workflows/v22-tenx.yml; GitHub Actions run 45094870568 observed in progress. Seed reservation in ledger commit 8a649e4b46b6c670b196e07d01b6770b75810da8 occurred AFTER workflow snapshot, leaving executing seed set intact.

Required next verification: inspect run/job logs and artifacts; ensure preflight/compilation passes, simulator-derived hazard train/validation and bounded policy training complete; two independent 30-game-per-generator paired normal/heavy/shifted-heavy holdouts report mean survival, p10, synthetic estimated score, illegal moves, combo breaks and avoidable deadlines. All strict >=10x per-generator gates, zero illegals/censor, nonworse combos/deadlines and independent replication required. Preserve any failed metrics; no promotion.

## Preflight failure and corrected retry
Run 45094870568 failed before training: workflow consistency and 570-start seed alias checks passed, 9/10 compilation guard tests passed, but `test_preprocessed_object_contains_active_bonus` expected literal `UINT64_C(...)` after `cc -E -P` expanded it to suffixed integer constants. C source syntax compilation with `-Werror` passed; this was a test expectation defect, not a measured gameplay regression. Preserve failed artifact. Corrected the preprocessor-aware test in research-only commit d86e75ee. Never replay failed epochs 344/345/346/348/349.

After verifying no active Actions runs, audited 377 reserved epochs and selected hazard train 375, hazard validation 376, policy train 377, holdouts 378/379 (570 distinct raw starts, no collisions with documented history). New immutable workflow snapshot 394ab3f1; reserved epochs in ledger AFTER snapshot via ad2807f2. Inspect new Actions run, preflight and paired holdout evidence before any improvement claim. No weights or live code promoted.
