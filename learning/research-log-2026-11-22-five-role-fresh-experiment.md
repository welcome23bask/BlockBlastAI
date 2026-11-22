# Fresh bounded five-role research run — 2026-11-22 UTC

Research branch only. No changes to main, live v3.4 iPhone, deployments, or validated checkpoints.

Fetched exact workflow `.github/workflows/v22-tenx.yml`, runner `learning/run_tenx_reproducible.sh`, ledger `learning/tenx_used_epochs.txt`, and raw PRNG auditor. Audited accessible repository Actions runs (544 total reported, latest prior run 44837049501 failed), and separately queried queued/in_progress/waiting statuses: all zero immediately before triggering. Historical local/external executions cannot be fully certified.

Genuinely new bounded experiment: current research-only hazard-learning and finalist strategy changes are now submitted to real CI against unchanged v1.7, instead of repeatedly modifying untested hypotheses. Selected hazard train 1654, hazard validation 1655, policy train 1656, holdouts 1657 and 1658. All distinct, within 17..2000, absent from 332 documented ledger epochs. Reproduced the exact source auditor's 32-bit legacy raw seed formula for all 3 scenarios and games 0..29 plus 100..107: 570 proposed starts unique internally and disjoint from all documented historical starts. No claim about unrecorded runs.

Workflow snapshot `3a173f79f7ea9d8ac5adbc41af530ec9a94c6991` triggered Actions run [44989421787](https://github.com/welcome23bask/BlockBlastAI/actions/runs/44989421787), verified queued and then in progress. After verifying the immutable executing snapshot, reserved all five epochs in the cross-role ledger in separate commit `fbbe4df403e865043c574010a86ce21034fcb463`, so the executing snapshot remains unaffected. This run must execute seed and compilation preflight, simulator-derived hazard training, then paired normal/heavy/shifted-heavy strict holdouts. The gate remains >=10x mean survival and synthetic estimated score per generator, >=10x p10 survival, zero illegal moves, no worse combo breaks/deadlines, no censoring, and independent replication before any success claim.

At documentation time, run is **in progress**; no measured improvement, pass, or checkpoint promotion claimed. Follow the job logs and uploaded artifacts; if failed, diagnose the worst generator or preflight failure and preserve evidence. Synthetic estimated score is not actual game score.

## Follow-up: CI failure and targeted correction

Actions run 44989421787 finished **failure** at 2026-11-22 05:26 UTC. `WORKFLOW_EPOCH_CONSISTENCY=PASS` for 1654–1658, immediately followed by `ERROR: working tree must be clean for reproducibility` from the runner. The earlier Python consistency checker can create untracked `__pycache__` files before the runner's clean-tree check. No simulator training or holdout ran; uploaded artifact 14740734302 contains only preflight provenance, not benchmark metrics. All five IDs remain reserved; do not retry them.

Targeted research-only runner correction `22d8b78a` exports `PYTHONDONTWRITEBYTECODE=1` for subsequent Python invocations. **This alone does not fix the workflow's preceding Python checker**, which must also run with bytecode disabled at the job or command level before the next fresh-seed trigger. No new workflow was triggered: a new five-role epoch audit and repository-wide non-overlap verification are required first. This fix is not a gameplay improvement and has not passed CI. Preserve unchanged v1.7 and strict 10x gates.
