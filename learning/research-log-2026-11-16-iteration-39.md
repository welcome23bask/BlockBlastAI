# Iteration 39 — preserve search-wide hard-shape Pareto finalists (research only)

Date: 2026-11-16 UTC, independent research branch `feature/v2.2-10x-research-gate` ONLY.

## Evidence and hypothesis

Run 43804331999 (iteration 38) failed both 10x gates; no stage-two training finalist was verified safe and heavy holdout 1960 regressed: mean survival 0.9970x, synthetic estimated score 0.9380x and combo-break rate 3.100 versus v1.7's 2.692 per 100 moves. Normal p10 0.9615x and shifted-heavy p10 0.9412x on holdout 1959 also regressed. The proactive pair-Pareto override is now disabled by default, and the previously falsified bottom-three hard-shape scoring remains disabled; unchanged gate thresholds, baseline and checkpoint are maintained.

Current issue: old late future-pair reranking evaluates at most four finalists selected **only** by ordinary board utility, which may discard genuinely more feasible hard-piece landing boards. The new targeted research-only mechanism in `learning/BLTrainableStrategy.c`, commit `3581b325119380d3a63d8f7907572b46c7208261`, stores two *parallel* complete-tray pools: top 4 ordinary current-board-quality legal paths and top 4 hard-minimax-coverage legal paths, both separately for combo-preserving routes. The afterstate reranker compares all retained legal finalists using the SAME original min-hard-shape bonus and paired-future coexistence score. This does **not** use actual future pieces, relax legality, or change training weights. Exact upper-bound shortlist pruning (max hard bonus 450×6=2700) in commit `f05b6f62297c289d60a254096e214e5595635f13` avoids most redundant geometry calculations without excluding any candidate capable of entering a filled risk pool. Added pool stores only completed legal paths, and baseline `src/BLDualStrategy.c` untouched. No replay of rejected future trays or proactive override.

## Bounded experiment and seed audit

[GitHub Actions 43804845920](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43804845920) queued by workflow-path edit `e5a622c7887a0a985265c9dc1f565da6c5f088bb` after checking exact workflow `.github/workflows/v22-tenx.yml`, runner `learning/run_tenx_reproducible.sh`, and cross-role ledger `learning/tenx_used_epochs.txt`, and confirming no overlapping research Actions run. Fresh provisional train epoch **1839**, holdouts **1961,1962**, all distinct and absent from accessible ledger/history before launch. Documented historical coverage is not proof against unlogged external/local runs. GitHub checkout and fail-fast seed preflight succeeded before training; epochs reserved only afterwards, in cross-role ledger commit `632eabc9b6397c826985f75035aea1f6ee7f5a1a`, not affecting running checkout. Training 21 local candidates on five paired games/generator, max four finalists on eight independent training games/generator, two 30-game/generator holdouts; 12-minute timeout, failure evidence artifact always retained.

## Strict unchanged gate

Compare with untouched v1.7 SMART SURVIVAL on independent paired normal, heavy, shifted-heavy 8×8 three-piece synthetic games; require survival ≥10x AND synthetic estimated score ≥10x for every generator in each of two holdouts, p10 survival ≥10x, illegal placements zero, combo breaks per move no worse, avoidable combo deadlines no worse, capped games zero at 4096, independent replication before success. Synthetic estimated score is **NOT actual Block Blast score**. No saved checkpoint/production/phone/main changes or deployment regardless of partial gains.

At initial log save: active run, verdict unknown. Preserve all failed metrics and identify worst generator afterwards.
