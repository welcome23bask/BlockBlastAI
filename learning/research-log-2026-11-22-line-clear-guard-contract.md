# Research-only preflight regression correction — 2026-11-22 UTC

Scope: `feature/v2.2-10x-research-gate` only. No main, live v3.4, checkpoint, deployment or iPhone build changes.

Fetched exact `.github/workflows/v22-tenx.yml`, `learning/run_tenx_reproducible.sh`, `learning/tenx_used_epochs.txt`, historical iteration-44 evidence, hazard protocol, strategy source, and active compilation guard.

## New concrete failure diagnosis
The current research source's crowded irregular finalist admission condition requires `path->lines>0`, but three assertions in `learning/test_irregular_admission_compilation_guard.py` still expected `path->misses==0)` with no intervening real-line-clear requirement. This means the fail-closed preflight compilation guard can reject the current source even when the C code is correct. This is a test/source contract mismatch, not a demonstrated gameplay gain.

Research-only commit `bc0b5c4426d09b43922c8c1b68f12fd027446120` updates all three positive/negative guard regexes to require `path->lines>0`. No production strategy, weights, or baseline changed. This is a targeted unblock for future bounded training/simulation, not an executed simulation. CI execution remains unverified.

## Remaining gates
The exact workflow currently references reserved hazard/train/holdout epochs 794/795/796/797/798, which must not be reused. Repository-wide Actions non-overlap and completeness of external/local historical seed usage were not established with available Actions access. No workflow push trigger was issued, no new epochs consumed, and no strict 10x outcome claimed. Before next experiment: verify no overlapping runs, audit five new cross-role epochs against ledger and raw PRNG aliases, update all workflow seed references, pass five-role preflight and compile guard, then run paired fresh normal/heavy/shifted-heavy holdouts versus unchanged v1.7 with strict per-generator mean/p10/estimated-score/combo/legality/censor gates and independent replication. Synthetic score is not actual game score.
