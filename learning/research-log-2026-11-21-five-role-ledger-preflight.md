# Five-role seed ledger preflight correction — 2026-11-21

Research branch only. Reviewed exact `.github/workflows/v22-tenx.yml`, `learning/run_tenx_reproducible.sh`, `learning/tenx_used_epochs.txt`, iteration-44 log, and learned-hazard protocol before change.

## Defect and fix

The runner previously excluded documented used epochs for policy training and two holdouts, but hazard train and validation IDs were checked only for numeric syntax and raw PRNG-start aliasing. Consequently, a previously used hazard epoch could pass the cross-role reservation policy if its raw start did not collide with proposed other roles. Research-only runner commit `7f3b48bede8c71297b54af2f95c6400dbb8a6c3c` now rejects hazard epochs outside 17..2000, any exact used-ledger hazard epoch, and any duplication among the five roles before invoking the raw-start audit or simulator. This is a preflight correctness fix, not a demonstrated gameplay improvement.

## Execution status

Current workflow still specifies hazard 794/795, policy 796, holdouts 797/798; all five are reserved in the cross-role ledger. This change intentionally causes these settings to fail earlier. No fresh training or paired holdout simulation was run; no seeds consumed and no performance claim. Repository-wide Actions non-overlap was not verified using the available connector, which lacks a repository-wide run listing. Do not trigger a workflow-only push with reserved epochs. Before the next experiment, audit five new IDs against accessible Actions and ledger, verify no overlapping run, check cross-scenario raw seed collisions, update every workflow reference, run preflight, and preserve all artifacts and failures. Historical external/local use remains incomplete.

Unchanged v1.7 comparator, strict per-generator 10x gates, checkpoint, main, and live v3.4 iPhone code remain untouched.
