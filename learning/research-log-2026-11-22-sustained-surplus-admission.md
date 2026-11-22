# 2026-11-22 — sustained-combo surplus admission ablation (research only)

Branch: `feature/v2.2-10x-research-gate`. Main, live v3.4 iPhone/Automatic Play, deployments and validated checkpoints were not changed.

## Evidence and diagnosis
Fetched the exact workflow `.github/workflows/v22-tenx.yml`, runner `learning/run_tenx_reproducible.sh`, and cross-role ledger `learning/tenx_used_epochs.txt`, along with the prior two-streak research log, strategy source, regression guard, and raw-seed auditor. The latest documented completed paired benchmark, Actions 45001339434, failed every strict 10x gate; shifted-heavy holdout 1011 had mean survival 1.1435x and synthetic estimated score 1.2465x versus unchanged v1.7. These are synthetic estimates, not real Block Blast scores. The historical artifact remains at https://github.com/welcome23bask/BlockBlastAI/actions/runs/45001339434/artifacts/14745479980 .

## New bounded hypothesis
Research source commit `3ed079f7a57f1cf5ab439c45b0bf51a7ad3ed2e0` changes the crowded three-family **surplus** admission condition from `path->streak>0` to `path->streak>=2`. The previous base admission had already required two streak clears, but the surplus +75 could still admit one-streak routes. This correction aligns both bonus components with sustained combo survival and targets weak shifted-heavy lower-tail behavior. Regression guard updated in `8b162f39a1caf0d2430154f9e2e1f5d0ff925adf`. No hidden future RNG, baseline edits, legal-search relaxation or weight/checkpoint promotion.

## Experiment status and limitations
This is a new research-only hypothesis, **not compiled, trained or benchmarked** in this run. The workflow retains reserved five-role epochs 1000/1001/1002/1010/1011. The connector supports fetching specific Actions run jobs/logs but not repository-wide queued/running run listing, so non-overlap was not established. A workflow-triggering commit would risk concurrent experiments or a guaranteed seed-preflight failure and was deliberately withheld. No seed was consumed or reused. Historical external/local runs remain incompletely known.

Next safe experiment: verify no overlapping Actions, select five fresh unused cross-role epochs, audit all proposed raw PRNG starts, update every workflow reference/provenance, pass preflight before training, then compare this candidate against unchanged v1.7 on fresh paired normal/heavy/shifted-heavy 8x8 three-piece simulations, with two independent holdouts, strict per-generator >=10x mean survival, >=10x synthetic score and p10, zero illegals/censoring and nonworsening combo breaks/deadlines. Preserve failed artifacts and reserve epochs only after triggering an immutable workflow snapshot. No independently replicated strict 10x result exists.
