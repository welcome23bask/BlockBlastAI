# Research iteration 18 — 2026-11-15

## Iteration 17 outcome (no deployment)

GitHub Actions [43740059527](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43740059527)
completed one bounded training epoch and two provisional simulation holdouts, but
failed strict 10× and combo safety gates (the workflow failure is deliberate).
It chose 120,120,30,180 (provisional) with training fitness 342.28.

Holdout 1905 (normal / heavy / shifted-heavy):
- Average survival ratios: 1.1158× / 1.1075× / 1.2342×
- Estimated score ratios: 1.1054× / 1.1690× / 1.4803×
- p10 survival ratios: 0.8286× / 1.0400× / 1.2143×
- Extra avoidable combo misses in normal: candidate 8 vs baseline 1
- Illegal moves: 0.

Holdout 1906:
- Average survival ratios: 1.3884× / 1.2836× / 0.9958×
- Estimated score ratios: 1.4500× / 1.4635× / 0.9503×
- p10 survival ratios: 1.7241× / 1.1176× / 1.1176×
- Candidate normal avoidable combo misses 9 vs baseline 1;
  heavy 1 vs baseline 0.
- Illegal moves: 0.

No candidate was promoted; research-only checkpoint in Git remains
`16 120 120 30 60`. Simulated proxy scores are not real game scores.

## New hypothesis

Raw fitness is not a sufficient safety gate: iteration 17 selected weights
that improved some mean metrics but repeatedly sacrificed avoidable combo
deadlines. Keep the original baseline fixed, but **exclude training
candidates with increased avoidable combo misses, increased combo breaks per
move, or any illegal placement on paired training seeds**, regardless of
the overall fitness. Among passing candidates, compare fitness as before.
If none pass, retain previous unpromoted weights.

To avoid silently repeating training samples when the saved checkpoint
does not advance, require an explicit *new* training seed epoch. This
iteration uses provisional training epoch **1818** (vs the earlier 1716)
and separate provisional holdouts **1907** and **1908**.
The known previously used epochs 1716, 1901–1906 are disallowed by
the launch script. A full historical audit across all older unrecorded
experiments remains incomplete; **do not certify seed independence or
a passing 10× result until this is resolved**.

A single training epoch, seven candidate trials, three paired games per
generator, and 30 held-out games per generator are still bounded by
the existing 12-minute Actions timeout. Evidence and failure output are
uploaded even when the strict result gate fails.

## Safety boundary

Changes only on `feature/v2.2-10x-research-gate`.
No main branch, production v3.4 dylib, existing checkpoint, or deployed
model changes. No promise of hourly quantitative gains.
