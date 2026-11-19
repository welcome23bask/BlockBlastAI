# Iteration 102 — 2026-11-18 (research branch only)

Fetched exact workflow, runner, cross-role ledger, exporter, and iteration 101 log. Workflow still uses reserved 200/201/202; no workflow replay or new seed reservation. Existing research gate remains unsatisfied; heavy generator survival remains the documented bottleneck.

Targeted data-integrity change in learning/export_hazard_afterstates.py: require the largest connected free region to equal 64 on an empty 8x8 board, be zero on a full board, and be nonzero on a partially empty board. This rejects impossible simulator-derived geometry before hazard training. Final research code commit: 277d362a0907e360a4c5f66399b01413f1a4bb97. Intermediate commits 4e3400c and 973232f introduced then removed an unjustified assumption that tray turns are limited to 0..2 and finalist IDs are contiguous; final source makes neither assumption. No local regression test or simulator run was executed. This is NOT demonstrated learning or gameplay improvement.

Next: add fixtures for empty/full/partial free-region boundary, execute regression tests, instrument actual simulator for legal continuation traces, and evaluate fresh audited holdouts against unchanged v1.7. Do not reuse reserved epochs, claim 10x, or touch live v3.4, main, deployments, or validated checkpoints.
