# Research-only crowded irregular deadline-risk admission hypothesis — 2026-11-21

Branch: feature/v2.2-10x-research-gate. No changes to main, v3.4 iPhone/Automatic Play, checkpoints or deployment.

Historical verified bottleneck: iteration 44 Actions 43807467128 heavy mean survival 1.0164x/1.0665x and synthetic estimated score 1.0239x/1.1735x versus unchanged v1.7; strict 10x failed. More recent Actions 44837049501 bounded-research job 136635994329 completed failure; no success inferred.

**New targeted untested hypothesis:** when admitting a crowded finalist for balanced exact irregular-family access, exclude candidates already carrying one or two avoidable combo misses. The old base 90x maximin admission reward allowed path->misses>0 when path->broken==0, potentially crowding out deadline-safe alternatives. Require path->misses==0 in that base admission predicate. The existing third-landing bonus already required misses==0. This affects only research finalist admission, not move legality, hidden RNG, baseline v1.7, pool size or search budget.

Source commit 2d16d0d63c9835e0b2ccffe71890fab877c379ec; compilation regression guard commit d6705aa4f9ec6fec12b95083da423e2e917854a9. No compilation or paired simulations have yet been performed; no gain is claimed.

Fetched exact workflow .github/workflows/v22-tenx.yml, runner learning/run_tenx_reproducible.sh, ledger learning/tenx_used_epochs.txt, iteration-44 log, frontier and hazard protocol. Current workflow still specifies already-reserved hazard 794/795, train 796, holdouts 797/798. Those IDs are in ledger and cannot be reused. Current GitHub connector exposes run-specific jobs/logs but not a repository-wide Actions runs listing, so non-overlap could not be verified. Do not trigger workflow merely by changing YAML without that verification.

Next safe bounded run: audit five genuinely fresh cross-role IDs 17..2000 against ledger and accessible historical runs; check raw seed aliasing and preflight before training; run 30 paired games/generator for each of two independent holdouts versus unchanged v1.7. Record mean and p10 survival, synthetic estimated score (not actual game points), illegal moves, combo breaks per move, avoidable deadlines and censored games. Require >=10x mean survival, score and p10 for each normal/heavy/shifted-heavy generator on both holdouts, zero illegal/censored and no combo/deadline regression; no checkpoint promotion on failure. External/local historical seed use may be incomplete.
