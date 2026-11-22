# Research iteration: 2026-11-22, post-run 45046268459

Scope: `feature/v2.2-10x-research-gate` only. No checkpoint promotion, deployment, main update, or iPhone build.

## Evidence
Workflow snapshot `96eb7913`, run https://github.com/welcome23bask/BlockBlastAI/actions/runs/45046268459, artifact https://github.com/welcome23bask/BlockBlastAI/actions/runs/45046268459/artifacts/14763588062. Five-role preflight, C compilation, hazard-label pipeline and paired training/holdouts completed. The strict gate failed, as expected. The run's two holdouts were 198 and 199; these epochs and 195/196/197 are already reserved in the cross-role ledger. Do not replay them.

Holdout 198: normal mean survival 2.2036x, synthetic score 2.6739x, p10 1.0789x; heavy 1.2136x, 1.4008x, 1.1579x; shifted-heavy 1.4059x, 1.8491x, 1.0625x.
Holdout 199: normal 1.6389x, 1.8197x, 0.8929x; heavy 0.9744x, 1.0627x, 1.0000x; shifted-heavy 1.1754x, 1.4412x, 1.1765x.
All six paired comparisons: zero illegal moves, zero censored games; combo-break rates improved, no increase in avoidable deadlines. Normal p10 REGRESSED on holdout 199. Heavy mean survival REGRESSED on holdout 199. No 10x gate passed; estimated score is synthetic, not actual gameplay score.

## Diagnosis and next bounded, genuinely different hypothesis
The signed-fragmentation fix now compiles and passes its boundary regression; it did not solve heavy lower-tail fragility. Training also rejected all four stage-2 candidates as regressions, so coefficient retuning alone is unlikely to resolve the problem. The next controlled hypothesis should be **lower-tail-first admission**: rank the existing exact-legal three-piece finalists by a training-only calibrated death-hazard lower-tail value before applying the crowded-board irregular-piece bonus, instead of tightening another arbitrary geometric threshold. Preserve exact legal move search, deterministic inference, independent validation, unchanged v1.7 comparator, and all six strict gates. Implement and test this on research branch only; do not treat the learned hazard diagnostic as an integrated policy until code and regression tests prove it. A future run needs five newly audited IDs and no active Actions jobs. Accessible GitHub Actions history is not a proof about external/local runs.

This iteration completed evidence-based failure triage and specified the targeted next strategy experiment; no new training job or simulation was launched in this iteration, and no improvement is claimed.
