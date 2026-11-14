# 2026-11-14 second bounded research check (15:48 US/Central)

Research-only diagnostic. Reviewed branch checkpoint epoch 16 `120 120 30 60`, candidate weights, `learning/train_tenx.c`, and Actions history. Latest Actions on this branch: [successful 43639191903](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43639191903) and [failed 43639153599](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43639153599), both from 2026-11-14. No new Actions run started.

**Important reproducibility limitation:** The bounded iteration below used the existing locally compiled `/mnt/data/tenx_lab/train_tenx_full` research executable, whose local source differs from the current branch `learning/train_tenx.c`. This is therefore an exploratory result, **not a repository-source-reproduced verification**. Do not promote its checkpoint or declare 10x.

Starting with a copy of epoch-16 checkpoint, command `./solver train 1 1` produced local exploratory epoch-17 weights `120 90 30 60`. Holdout command `./solver test 5 tenx_trained_weights.txt 989 0` used a distinct seed epoch from training and the prior 307 diagnostic. The 8x8 three-piece simulator checks move legality, and each game is capped at 1400 moves.

| Generator | v1.7 avg moves | candidate avg moves | survival ratio | estimated score ratio | p10 ratio | v1.7/candidate combo breaks per 100 moves | missed safe combos baseline/candidate | invalid |
| --- | ---: | ---: | ---: | ---: | ---: | --- | --- | ---: |
| normal | 348.6 | 118.8 | 0.3408 | 0.2922 | 0.4545 | 1.779 / 2.862 | 0 / 1 | 0 |
| heavy | 35.8 | 38.8 | 1.0838 | 1.1924 | 1.0000 | 3.352 / 3.608 | 0 / 1 | 0 |
| shifted-heavy | 23.8 | 28.6 | 1.2017 | 1.5049 | 1.7857 | 0.840 / 1.399 | 0 / 0 | 0 |

**Verdict: FAIL.** All generators fail 10x average survival, score, and p10; combo-break rates worsen; avoidable combo deadlines increase. Sample size n=5/generator is only diagnostic, and no independent replication is warranted. Scores are simulator proxies, never real Block Blast scores.

**No changes to checkpoint, main, app, deployment, or iPhone dylib.** Next iteration should compile the exact research-branch sources and validate source/binary provenance before training, then use larger fresh holdouts if a promising candidate emerges.
