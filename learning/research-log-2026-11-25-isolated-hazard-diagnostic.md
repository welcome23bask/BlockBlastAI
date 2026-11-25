# Isolated learned-hazard diagnostic — 2026-11-25

Status: real bounded local simulation, NOT the repository v1.7 benchmark, NOT a 10x pass or checkpoint promotion. Exact repository sources could be read via connector, but local git clone failed DNS and connector only lists PR-triggered workflow runs, so overlapping push-triggered runs could not be excluded. No workflow edit/trigger, main, phone build or checkpoint change. The current workflow's 887/888/889/892/910 are already reserved.

## Methods and provenance

A separate C11 eight-wide beam-search diagnostic used the exact 28 shape patterns, 8x8 simultaneous row/column clears, three-piece draws and legacy RNG formula from the research simulator. Unlike the repository reference, it chooses a full tray at once, uses a simplified geometric utility and does not implement the full v1.7 urgent combo rescue or exhaustive search. It is not a substitute for the official v1.7 paired benchmark. Compare its identical heuristic with/without a 1800-point calibrated three-tray hazard penalty, using only observable board features. Train-only logistic fit with episode-balanced weighting, L2=0.005, fixed penalty and independently validated Brier against train-only constant per generator. Trace labels from paired continuation of the diagnostic heuristic. The selected epochs 642 (fit), 649 (validation), 673 (development diagnostic), 772/828 (two untouched diagnostic holdouts) have 570 distinct raw starts audited against 447 documented reserved epochs; external/local history incomplete. Fit/validation: 15 games per generator, 896/794 labeled afterstates; development 8 games per generator per policy; each holdout 30 games per generator per policy. 408 evaluated games total, plus trace generation. No parameter selection on holdouts. Cap approximately 4096 moves, no censoring observed. Score is synthetic proxy, not real game score. Deadline proxy counts immediate legal clear opportunities missed at two combo misses; not identical to v1.7 exact avoidable-deadline metric.

## Calibration
| generator | model Brier | train constant Brier | result |
|---|---:|---:|---|
| normal | .111233 | .126892 | pass |
| heavy | .161221 | .193355 | pass |
| shifted-heavy | .247386 | .266826 | pass |

## Paired local diagnostic metrics
Each entry is heuristic / learned; moves and score are averages, breaks per 100 moves, deadlines are total counts.
| epoch | generator | mean moves | p10 moves | synthetic score | breaks/100 | deadline proxy |
|---|---|---:|---:|---:|---:|---:|
|673|normal|38.125 / 41.125|17 / 16|975.125 / 1062.875|4.918 / 4.863|0 / 0|
|673|heavy|29.625 / 32.250|17 / 17|823.375 / 909.875|4.641 / 5.426|0 / 0|
|673|shifted-heavy|22 / 21|19 / 17|528.750 / 522.625|5.682 / 3.571|0 / 0|
|772|normal|66.600 / 74.200|25 / 26|1927.367 / 2302|5.305 / 4.987|1 / 2|
|772|heavy|32.200 / 33.067|11 / 12|929.400 / 943.567|4.658 / 4.435|0 / 1|
|772|shifted-heavy|24.200 / 22.600|9 / 9|664.267 / 598.667|4.683 / 5.457|1 / 1|
|828|normal|77.700 / 85.900|20 / 26|2301.433 / 2614.100|5.191 / 4.695|4 / 4|
|828|heavy|27.367 / 29.833|14 / 14|653.833 / 768.067|5.725 / 5.698|0 / 1|
|828|shifted-heavy|23.667 / 24.100|15 / 15|607.933 / 674.767|5.493 / 5.533|1 / 0|

Zero illegal or capped games for every row. Heavy mean ratios 1.0269 and 1.0901; shifted-heavy 0.9339 and 1.0183. The candidate fails strict gates, with shifted-heavy regressions and additional immediate deadline misses on both heavy holdouts. Even good Brier calibration does not guarantee that the learned reranker improves policy. Do not promote or repeat these settings/seeds. Next hypothesis: preserve urgent clear and existing combo behavior at the FINAL selection stage before hazard rerank, then introduce scenario-blind hard-family feasibility features and obtain candidate-distribution labels. Validate on new audited IDs only. Full repository bridge still needs compilation and exact v1.7 benchmark; no official improvement claim.
