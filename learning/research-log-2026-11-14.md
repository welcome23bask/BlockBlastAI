# 2026-11-14 bounded offline research iteration

Research only; unchanged v1.7 SMART SURVIVAL baseline. No deployment, main changes, or iPhone dylib.

Source: learning/learn_combo_guarded.c (8x8 shapes/three-piece legality) and learning/BLTrainableStrategy.c; training driver copied from prior local research package. Local command: `./train_tenx train 1 2` with checkpoint `16 120 120 30 60`, followed by `./train_tenx test 8 tenx_trained_weights.txt 307 0`. MAXMOVES=1400. The trainer selected unchanged weights `120 120 30 60`; resulting checkpoint epoch 17. Evaluation uses epoch 307, disjoint from training. Small n=8 per generator is only a diagnostic, not a statistically strong verification.

| generator | baseline avg moves | candidate avg moves | survival ratio | estimated-score ratio | p10 ratio | baseline/candidate combo breaks per 100 moves | baseline/candidate missed combo | invalid |
| --- | ---: | ---: | ---: | ---: | ---: | --- | --- | --- |
| normal | 162.875 | 109.000 | 0.6692 | 0.6629 | 0.7931 | 2.379 / 2.179 | 0 / 0 | 0 |
| heavy | 34.250 | 31.750 | 0.9270 | 0.9152 | 1.1176 | 5.109 / 3.150 | 1 / 1 | 0 |
| shifted-heavy | 28.125 | 28.500 | 1.0133 | 1.1671 | 0.8421 | 2.222 / 3.070 | 1 / 0 | 0 |

Verdict: **NOT REACHED; DO NOT DEPLOY**. The model regressed on normal and heavy survival, and on shifted-heavy lower-tail and combo breaks. No second independent verification warranted. Scores are simulator proxies, not actual Block Blast scores. No GitHub Actions minutes consumed for this iteration. Prior GitHub Actions run was not independently verified in this iteration.
