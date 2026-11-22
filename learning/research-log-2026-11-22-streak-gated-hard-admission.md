# 2026-11-22 — strict holdout failure and combo-streak admission hypothesis

Research branch only: `feature/v2.2-10x-research-gate`. No main, live v3.4, deployment, iPhone build, or validated checkpoint edits.

## Evidence
[Actions run 45001339434](https://github.com/welcome23bask/BlockBlastAI/actions/runs/45001339434) completed seed preflight, Python regressions, hazard-label simulation/model stage, C compilation, policy training epoch 1002, and both paired holdouts 1010/1011. The strict gate failed (expected nonzero workflow exit). [Artifact 14745479980](https://github.com/welcome23bask/BlockBlastAI/actions/runs/45001339434/artifacts/14745479980) retained evidence. All five epochs 1000/1001/1002/1010/1011 remain reserved in the cross-role ledger.

Paired candidate / unchanged v1.7 mean survival, synthetic estimated-score, p10 ratios:
- 1010 normal 2.2647 / 2.8006 / 1.6889; heavy 1.2428 / 1.6257 / 1.0000; shifted-heavy 1.2218 / 1.5326 / 1.6667.
- 1011 normal 2.6040 / 3.1472 / 1.3750; heavy 1.3616 / 1.6550 / 1.1765; shifted-heavy 1.1435 / 1.2465 / 1.3571.
- All six comparisons had zero illegal moves, zero capped games, no worse combo breaks per 100 moves or avoidable deadlines. Strict 10x mean, estimated-score, and p10 gates failed for every generator. Synthetic estimated score is NOT actual game score.
- Stage-two four candidates all failed regression checks; no weights were promoted. Weakest mean survival/score is shifted-heavy 1011, with heavy 1010 p10 unchanged at 1.0x.

## New bounded hypothesis (NOT yet tested)
Source commit `d5f5304f57403152524f52f4d226255fb6b8561a` changes ONLY research crowded-board hard-family finalist admission quality: its base +90*min(irregular landing families) is multiplied by `(path->streak>0)`. This prevents inactive-combo finalists from consuming scarce admission slots merely through geometrical hard-family access, while retaining their legal candidacy, exact legal search, hard-family predicates, and existing two-line-clear requirement. This targets combo-sensitive shifted-heavy tail and avoids hidden future RNG. The separate third-landing surplus already required a live streak. Regression assertion added in `2a9a7e556347e61a2fcb3e46ce11ee0db6fd89d4`.

This is a distinct uncompiled, unbenchmarked strategy hypothesis, not proof of improvement. No new workflow run was triggered: the workflow retains reserved 1000/1001/1002/1010/1011 and must first be updated with five newly audited cross-role IDs, after checking repository-wide Actions non-overlap and raw RNG aliases. External/local historical epoch completeness cannot be established. Preserve failed metrics and artifacts. Next run must pass preflight and compare unchanged v1.7 with two independent 30-game/generator holdouts, all strict 10x mean/score/p10, combo, legality and censor gates. No promotion.
