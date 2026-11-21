# Research-only correction — 2026-11-21 effective shifted-cross occupancy gate

Branch: feature/v2.2-10x-research-gate. Source commit 0a7a039d0493e35437bfa63c991362d50aecb0e4; regression commit 4f315ca94d8b4e72f91cc7473ce92760b0e2f3c4.

## Concrete defect and new bounded hypothesis

The prior +75 shifted-cross third-landing bonus was guarded by `64-bits(path->board)<32` **inside** an outer `64-bits(path->board)<24` condition. The purported occupancy-gating change was mathematically inert: all states reaching that expression already satisfied <32. This run corrects that ineffective change with a genuinely selective `<20` free-cell threshold. Hypothesis: only the most crowded complete-tray afterstates benefit from the extra shifted-cross landing preference; on 20–23-free-cell boards the bonus may displace combo-safe or normal-tail finalists. Three-family minimum access and exact disjoint-square predicate remain unchanged; 20-finalist cap, search-node budget and unchanged v1.7 baseline remain intact. No hidden future RNG.

## Evidence and limitations

Fetched the exact workflow, runner, ledger, provisional frontier, learned-hazard protocol, strategy and regression guard. The current workflow still contains reserved five-role epochs 794/795/796/797/798 and MUST NOT run unchanged. The available connector does not offer a repository-wide workflow run list to verify non-overlap; no workflow was triggered and no seeds were consumed. This source change is committed but not compiled, simulated, or independently validated; no gameplay gain or 10x result is claimed. The previous failed holdouts 797/798 remain the relevant lower-tail diagnosis. Accessible ledger history is not guaranteed complete.

## Next safe execution

First verify all Actions runs are non-overlapping and audit five fresh unused epoch IDs (hazard train/validation, policy train, independent holdouts) with the cross-role ledger and raw PRNG-start collision checker. Replace ALL workflow epoch occurrences consistently and pass preflight before training. Preserve failed artifacts. Run paired 30-game/generator normal, heavy and shifted-heavy comparisons against unchanged v1.7, measure mean/p10 survival, synthetic estimated score (not actual app score), illegals, combo breaks per move, avoidable deadlines and censoring. Reject unless every strict >=10x gate passes on both independent holdouts; never promote the checkpoint or touch main, v3.4 iPhone, deployments or validated weights.
