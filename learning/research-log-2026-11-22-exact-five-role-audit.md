# Research iteration — exact five-role raw-seed audit contract (2026-11-22 UTC)

Scope: ONLY `feature/v2.2-10x-research-gate`. No main, v3.4 iPhone, deployment, validated checkpoint or baseline edits.

## Evidence reviewed
Fetched exact `.github/workflows/v22-tenx.yml`, `learning/run_tenx_reproducible.sh`, `learning/tenx_used_epochs.txt`, iteration-44 report, frontier, and hazard protocol. Inspected Actions run 44837049501 job 136635994329 and its decoded log: preflight and hazard-learning stages succeeded; strict holdouts 797/798 failed. On 797, heavy p10 ratio 0.8500 despite mean survival 1.2320; on 798, normal p10 ratio 0.9737 despite mean 1.5871. Zero illegal/censored/deadline regressions in the logged comparisons; none approached 10x. Synthetic estimated scores are not real game scores. Existing evidence artifact 14679192465 remains linked to that run.

## New bounded methodology correction
The source-verified `learning/audit_raw_seed_aliases.py` previously accepted any >=3 epochs, including a three-role call that silently skipped hazard train/validation. Updated `check()` and CLI to require **exactly five** role epochs and added negative selftests for 3-role and 6-role calls. Research-only commit `adf8555cd7b1e5a1ef8ecbea2f9c9828f8d51bbf`. This strengthens the existing training/simulation collision gate, not the game strategy. It remains untested in CI. The workflow still uses reserved 794/795/796/797/798 and must not be launched with them. Full repository-wide Actions concurrency and historical external/local seed completeness cannot be verified through currently available connector operations; no workflow was triggered, no epochs consumed, and no performance claims made.

## Next bounded experiment
Verify no overlapping Actions jobs; audit five new cross-role epochs and legacy raw starts against the ledger and accessible runs; update **all** workflow epoch references in one snapshot; pass five-role preflight; run bounded hazard training and paired independent 30-game normal/heavy/shifted-heavy holdouts against unchanged v1.7. Focus next strategy on heavy lower-tail failure rather than more untested finalist-admission toggles. Preserve failed metrics/artifacts, reserve consumed epochs only after immutable run snapshot, and require independent replication of all strict >=10x gates before any promotion.
