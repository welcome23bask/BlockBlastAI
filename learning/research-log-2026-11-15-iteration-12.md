# Research iteration 12 — 2026-11-15 UTC (source/CI audit; no simulation)

Research branch: feature/v2.2-10x-research-gate. Source HEAD before this log: 9a2c2e13c16fb60a47334d29a425e9c2c9666726.

Verified via connected GitHub source reads:
- learning/tenx_checkpoint.txt = `16 120 120 30 60`; learning/tenx_candidate_weights.txt = `120 120 30 60`.
- learning/train_tenx.c implements 8x8 placement checks, three-piece trays, normal/heavy/shifted-heavy generators, 1400-move episode cap, unchanged v1.7 baseline comparison, survival/score-proxy/p10/combo/legality gates. Its `test` returns zero even when TEN_X_VERDICT is NOT REACHED; callers must check verdict.
- learning/run_tenx_reproducible.sh checks clean research checkout, trains one epoch on one seed/game per generator, runs 30 games per generator on each of two caller-selected holdout epochs, checks the verdict, and preserves evidence outside checkout. It excludes documented epochs 92, 93, 989, 1201, 1202 but cannot independently prove any other epoch has never been trained/tested.
- The checked-in workflow .github/workflows/v22-tenx.yml does NOT invoke that corrected runner: it runs holdout epoch 93, accepts the presence of a verdict even if failed, and then runs a separate smoke train. Its push path filter excludes changes to the reproducible runner and research logs.
- Latest GitHub Actions research run: 43639191903, success at 2026-11-14 20:46 UTC on old SHA 9c4d80bda51e6b86c4ff66d20d71e47bb97541d4; no current-source 10x proof.
- Recorded epoch-92 holdout: normal survival 1.2111x, proxy score 1.3214x, p10 0.7234x; heavy 1.2254x/1.2808x/p10 1.1111x; shifted-heavy 1.1672x/1.2440x/p10 1.1333x. Normal combo misses increased. These are simulator estimates, not actual Block Blast scores.

No exact-source compilation, new training, unused-seed simulation, or independent replication occurred in this audit. GitHub connector supports source reads and commits but no workflow dispatch. Local source checkout could not be established in this run. No workflow was rerun, no Actions minutes spent, and no checkpoint/weights were promoted. Main, deployment and iPhone dylib were not changed.

Next: obtain an executable exact-source checkout; audit all historical seed usage; run the corrected reproducible runner with two genuinely fresh holdout epochs; preserve evidence and independently replicate any passing candidate. Fail closed on all six generator/holdout gates including combo deadlines. Do not claim 10x without all gates.
