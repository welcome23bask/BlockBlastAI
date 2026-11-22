# 2026-11-22 UTC — pre-trigger raw RNG independence gate

Research branch only. No iPhone/main/deployment/checkpoint modifications.

## Evidence
Fetched the exact workflow `.github/workflows/v22-tenx.yml`, runner `learning/run_tenx_reproducible.sh`, cross-role ledger `learning/tenx_used_epochs.txt`, iteration-44 log, frontier, hazard protocol, and seed-audit source. GitHub Actions run 44837049501 bounded-research job 136635994329 completed with successful preflight and hazard pipeline but failed strict paired simulation gates; failure artifacts were uploaded. No replicated 10x result.

The workflow still hardcodes reserved hazard/train/holdout epochs 794,795,796,797,798. The newly added ledger-aware consistency checker correctly rejects those, so this workflow is NOT dispatch-ready. External/local seed history remains incomplete. Current connector exposes per-run jobs/logs but not a complete repository-wide running/queued run listing; non-overlap cannot be certified. No workflow edit/dispatch or new epoch consumption.

## New targeted methodology hypothesis
A workflow-only edit can trigger Actions before the runner's raw-seed alias audit, wasting the bounded slot or risking poorly audited seed selection. Existing pre-trigger consistency checker checked exact epoch uniqueness and ledger exclusion but did not check legacy cross-scenario PRNG start collisions (20/40-epoch offsets). Research-only commit 6696fe32718019d2b2bba3bbddd242796482674d now imports and invokes the source-verified `audit_raw_seed_aliases.check(*ids)` in the pre-trigger consistency check, and rejects duplicated preflight markers. This is a concrete fail-closed preflight improvement; not a trained policy or gameplay improvement. It has not been executed in CI.

## Next bounded experiment
Verify no running/queued repository Actions jobs with an authoritative listing. Audit five genuinely fresh epoch IDs across historical runs/ledger and raw starts, update every workflow reference consistently, run preflight before training, then compare the latest research-only policy against separately compiled unchanged v1.7 on two independent paired 30-game normal/heavy/shifted-heavy holdouts. Preserve all failures and never promote without every strict 10x mean/estimated-score/p10/combo/legality/deadline/censor gate passing twice. Synthetic estimated score is not actual game score.
