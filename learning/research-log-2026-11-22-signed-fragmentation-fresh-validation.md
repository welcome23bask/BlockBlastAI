# Signed fragmentation: fresh bounded validation (2026-11-22)

Research branch only: feature/v2.2-10x-research-gate. No changes to main, live v3.4, deployments, or checkpoint.

## Evidence and failure mode
Previous bounded run 45029378139 failed strict gates; shifted-heavy holdout 362 mean survival 1.0633x and synthetic estimated score 1.1532x, with heavy p10 0.9412x. The fragmentation admission condition was previously a no-op because its helper returns -100 per stranded free cell. The corrected >= -400 condition now excludes 5+ stranded cells, but this correction had not been benchmarked.

## Targeted preflight fix and new experiment
The numeric regression for the signed fragmentation condition contained overescaped regular-expression tokens (literal backslash-s instead of whitespace matching), preventing the intended source assertion from functioning. Corrected in e09b93ad. This is a test-harness correction, not a demonstrated gameplay gain.

Verified the repository Actions list had no active runs immediately before workflow update. Audited 347 ledger-reserved epochs and selected fresh hazard training 146, hazard validation 147, policy training 148, and independent holdouts 149/183. Conservative legacy raw-start audit covered 3 generators x 38 game indices x 5 epochs = 570 starts, with no overlaps among the proposed starts or against the 347 documented historical epochs. This does not certify unrecorded local/external history. Workflow snapshot f0476e22 triggered Actions run 45046198256. All five epochs were reserved in ledger commit c3579f09 after the executing workflow snapshot, without altering its seeds.

The bounded experiment compares the corrected fragmentation admission candidate against unchanged v1.7 SMART SURVIVAL across normal, heavy, and shifted-heavy three-piece 8x8 generators, with independent paired holdouts and unchanged strict mean-survival, p10, synthetic estimated-score, legality, combo-break, avoidable-deadline and censor gates. Synthetic score is not the official game score. No checkpoint promotion is authorized. At the time of this log, the run had begun but had not reported preflight or benchmark results; inspect Actions jobs/logs and preserved artifact before any interpretation.
