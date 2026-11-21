# Five-role preflight omission guard — 2026-11-21

Research branch only. Fetched exact `.github/workflows/v22-tenx.yml`, `learning/run_tenx_reproducible.sh`, and `learning/tenx_used_epochs.txt` before editing. Workflow currently embeds reserved 794/795/796/797/798 and MUST NOT be triggered unchanged. Accessible external/local historical runs may be incomplete.

## Defect and bounded fix

Runner previously permitted BOTH hazard seed environment variables to be omitted, falling back to a three-role audit. This allowed an official research invocation to bypass hazard-training/validation epoch exclusion, even though the workflow's reproducibility contract is five-role. Commit `d8adf4becc04a9f4ed725a8c5adb161baad9b190` now fails closed if either hazard role is missing. The existing five-role raw-seed alias, ledger, and uniqueness checks remain unchanged; no gates are relaxed. The current workflow supplies both roles, so this fix has no expected effect on its properly configured execution. No seed consumed.

## Status and next experiment

This is a research-safety correction, not a demonstrated training or gameplay improvement. Not compiled or tested in Actions. No new workflow trigger: available connector does not expose repository-wide run listing sufficient to prove no overlapping Actions jobs. Before any future run, audit five fresh unused epochs against accessible Actions/logs/ledger, verify concurrency, run five-role preflight and raw-start collision checks, then execute paired normal/heavy/shifted-heavy holdouts against unchanged v1.7 with strict per-generator mean/score/p10/combo/deadline/legality/censor gates and independent replication. Heavy lower-tail survival remains the priority; hazard-model calibration should be tested as a distinct bounded hypothesis. No checkpoint promotion, main changes, iPhone build, or live v3.4 edits.
