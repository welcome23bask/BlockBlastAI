# BlockBlastAI research — gameplay iterations 31 and 32

Date: November 15, 2026 Central (Actions timestamps on November 16 UTC).
Scope: `feature/v2.2-10x-research-gate` ONLY. Production/main, iPhone v3.4 automatic play, and saved v1.7/reference checkpoint remain untouched.

## Gameplay diagnosis

Current research solver enumerates three visible pieces legally on an 8×8 board, but (1) finite search can overlook an urgent first-move clearing path; (2) rewarding individual future-shape placements does not guarantee space for two simultaneously; (3) 21 four-weight variants tuned against only five paired games are susceptible to strong holdout variance, especially p10 and shifted-heavy.

## Iteration 31: exact-legal last-chance combo rescue

Research-only C source commit `d2fa4d817125b397d604475e90338a53abf7140b` adds a separate bounded rescue to the trained solver only when a *witnessed* combo has exactly two misses and the main solver returns no immediately clearing move. The rescue enumerates actual legal first clears and requires that all remaining currently visible pieces can be placed; searches up to 30,000 nodes, sampling at most 12 complete routes per clearing first action. It makes no claims about future unknown trays. Tests in `learning/tenx_rules_selftest.c` include extreme main-search budget, urgent clear, non-clearable state, mixed irregular shapes, row/column clears and move legality (`b35b6af...`).

[GitHub Actions run 43792626108](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43792626108) at exact workflow SHA `3b278adb5bb74a78de7d35e91879d772ed98d7b6` passed documented seed preflight (train epoch 1831, holdouts 1945 and 1946), compiled the exact source, passed 8x8/three-piece selftests, completed one 5-game-per-generator epoch and both 30-game-per-generator synthetic holdouts. Only 1 of 21 training variants passed the initial combo safety gate. The two holdouts reported **zero illegal placements** and no candidate avoidable combo misses. Candidate average survival was above baseline in all six generator comparisons, but normal p10 survival was only 0.7727x of baseline on holdout 1946, and certain combo-break rates were higher. All 10x gates failed. Approximate mean survival by generator: normal 1.4467x/1.2408x, heavy 1.1875x/1.1068x, shifted-heavy 1.1168x/1.1383x. Simulation score estimates are not real Block Blast scores. Raw artifact 14193410281.

## Iteration 32: robust future-piece pair capacity

Research-only source commit `c576b17e74215e4bfd0901e8845d91047c86f8d9` maintains up to four diverse complete legal current-tray afterstates for each objective and scores the finalists for **six known difficult future-shape pairs** that can fit *without overlapping* on the resulting board. This is a conservative geometric reserve-space proxy, not any knowledge of the game's hidden next tray. It is capped to eight finalist boards instead of evaluating every leaf of the 180k-node search. Independent tests verify the empty board passes all six pairs, while a full board or a single isolated 3x3 landing zone passes none (commit `0d6ad5c...`).

[GitHub Actions run 43793347021](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43793347021), source `84181e6a0ceee092026a40202d0aee6fa6e56a25`, passed preflight (train 1832, holdouts 1947 and 1948), compilation, legality selftests, one 5-game-per-generator training epoch, and two 30-game-per-generator synthetic holdouts. 2/21 training candidates were combo-safe; selected isolated candidate `120 180 30 0`. All six candidate holdout means (survival and proxy score) were **greater than 1x** the baseline, and all six combo-break rates were lower, with zero illegal placements and no additional avoidable combo deadlines.

| Generator | Holdout 1947 survival / proxy / p10 ratios | Holdout 1948 survival / proxy / p10 ratios |
|---|---|---|
| Normal | 1.0849x / 1.1727x / 1.2051x | 1.2788x / 1.4247x / **0.8600x** |
| Heavy | 1.0525x / 1.1161x / 1.0588x | 1.3115x / 1.4720x / 1.0500x |
| Shifted-heavy | 1.0957x / 1.2487x / 1.0000x | 1.1126x / 1.2164x / **0.9412x** |

This is **promising mean/combo behavior but not strict non-regression or 10x**: normal and shifted-heavy p10 regressed on holdout 1948, and the 10x threshold is far away. The Actions workflow failed intentionally on its strict verdict. Raw artifact 14193167810. Any attributed benefit is not independently isolated from earlier changes; it is an observed candidate-v1.7 comparison, not proof that pair reranking alone caused the gains.

## Next bounded experiment

To address small-sample overfitting, research-only commit `51eded41972e3196623a7a631afe50e15ba2c158` adds a **second train-only validation split**: shortlist up to four of 21 initially combo-safe weight variants, re-evaluate on eight distinct training-game indices 100..107 in each generator, and only accept finalists with no mean survival, score-proxy, p10 or combo regressions on that separate training split. This was committed *after* iteration 32's workflow head SHA and has **not yet been compiled or evaluated**. It must be measured with independent fresh holdout epochs. No holdout data can be used for training; pass remains subject to full 10x gates on independent repetitions. The goal is to reduce acceptance of noisy four-weight variants, not to claim learning guarantees.

## Research constraints

- The central `learning/tenx_used_epochs.txt` ledger now excludes all completed iteration-31/32 training and holdout epochs (1831/1945/1946; 1832/1947/1948). Historical non-use was checked across 28 accessible research logs for each selected epoch, but external or unrecorded runs remain unverifiable: **freshness is provisional, not globally certified**.
- Source, tests and simulator model remain approximations; real Block Blast future-piece distribution and scoring are proprietary and unavailable in this offline lab. Estimates must not be misrepresented as observed phone scores.
- The saved research checkpoint remains `16 120 120 30 60`, candidate weights remain `120 120 30 60`. Neither experiment promoted weights, modified main, deployed, or built an iPhone dylib.
- The full strict 10x mean survival, estimated-score, p10, zero-invalid, combo non-regression and independent replication requirements are **NOT VERIFIED**.
