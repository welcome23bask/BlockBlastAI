# Research-only hypothesis — all-family third-landing admission gate

Date: 2026-11-21. Branch: `feature/v2.2-10x-research-gate` only. This is a source hypothesis, NOT a completed simulation or a validated improvement.

## Evidence and failure mode

Reviewed the exact workflow `.github/workflows/v22-tenx.yml`, runner `learning/run_tenx_reproducible.sh`, cross-role ledger `learning/tenx_used_epochs.txt`, iteration-44 results and frontier, and learned-hazard protocol. The last inspected run 44837049501 failed strict gates (including heavy/normal lower-tail regressions). The current workflow still embeds reserved hazard/train/holdout epochs 794/795/796/797/798; preflight must reject them. The ledger includes these five epochs. Historical external/local seed usage may be incomplete. The available connector has per-run job/log fetch but not a repository-wide run listing; no repository-wide non-overlap assertion is possible from this session.

## Targeted distinct hypothesis

Prior crowded-board admission awarded an extra 75 points whenever shifted-cross had three exact landings and fewer than 20 cells remained free, even if the other two irregular families had only two landings. This could favor a one-family surplus at the expense of more balanced heavy-piece escape options. The new research-only admission condition requires all three families to retain at least three exact legal landing positions before awarding that extra 75. The existing 90-point maximin bonus, 20-finalist bound, search budget, current-tray legality, and unchanged v1.7 comparator remain intact. No hidden future RNG is read. Source commits `8a21d66b` and `75b4d3c4`; regression expectation updated in `f4cec309`. No main, live v3.4, deployment or validated checkpoint changes.

## Execution status and next safe step

No Actions experiment was triggered: repository-wide concurrency was not verifiable, and workflow epochs 794–798 are already reserved. No seed was consumed or ledger reservation added. No compile, test, training or fresh paired simulation has been verified for this hypothesis. Before execution: establish no overlapping Actions jobs, audit five fresh distinct IDs in 17..2000 against ledger and accessible history and raw PRNG starts, update the workflow to those IDs, require preflight PASS, then run bounded training and two independent paired 30-game/generator normal/heavy/shifted-heavy holdouts against unchanged v1.7. Preserve artifacts even on failure. Require >=10x mean survival and synthetic estimated score per generator, >=10x p10 survival, zero illegal and capped games, no worse combo breaks or avoidable deadlines, and independent replication. Synthetic estimated score is not actual Block Blast points. Never promote failed weights or claim success from this untested change.
