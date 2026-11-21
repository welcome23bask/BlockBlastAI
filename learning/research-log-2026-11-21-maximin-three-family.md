# Research-only three-family maximin admission — 2026-11-21

Branch feature/v2.2-10x-research-gate only. Source e90bef02; regression guard 43e307ac. No checkpoint, main, release, iPhone or v3.4 changes.

## Hypothesis
The crowded-board finalist admission bonus previously required at least two legal placements in each of three irregular families but rewarded only min(irregularA,irregularB), ignoring the shifted-cross count after admission. The new bounded rule rewards 90 * min(irregularA,irregularB,shiftedCross) (counts capped at three). This specifically tests whether retaining shifted-cross flexibility rather than overscoring two-family flexibility improves heavy/shifted-heavy survival tails. Exact legal placement geometry only, no hidden next-tray RNG. The 20-finalist and search-node budgets and strict gates are unchanged.

## Evidence and limitations
Reviewed exact workflow .github/workflows/v22-tenx.yml, runner learning/run_tenx_reproducible.sh, cross-role ledger learning/tenx_used_epochs.txt, iteration-44 research log and provisional frontier. Last documented failed run 44837049501 has heavy and p10 bottlenecks. Current workflow still embeds previously reserved 794/795/796/797/798; must not be triggered unchanged. The connector does not offer a repository-wide Actions run listing, so concurrent run absence cannot be established. No new training, compilation, simulator evaluation, or seed reservation was performed. Accessible historical epochs do not establish complete external/local usage. The hypothesis is untested and is NOT a 10x improvement.

## Next bounded evaluation
Verify no overlapping Actions run through an authoritative repository-wide Actions check, audit five fresh hazard train/validation, policy train and two independent holdout epochs including legacy raw-start aliases, update workflow YAML snapshot, pass preflight before training, then run unchanged v1.7 paired normal/heavy/shifted-heavy 30-game holdouts twice. Preserve failed metrics and artifacts; no promotion unless all per-generator >=10x mean survival, estimated synthetic score and p10, zero illegals/censor, and nonworse combo breaks/deadlines independently replicate. Synthetic score is not game score.
