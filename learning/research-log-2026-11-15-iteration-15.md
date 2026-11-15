# Research iteration 15 — 2026-11-15 (bounded exact-source execution)

Research branch: `feature/v2.2-10x-research-gate`. GitHub Actions run [43714256919](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43714256919), triggered 2026-11-15 12:31 UTC at source `38d2eccc5b7a59e371c27a61e12f2acf5a98f8f6`, completed with failure as intended because performance gates failed. Exact-source C compilation succeeded, one bounded training epoch completed, two provisional holdouts completed, and failure evidence was uploaded in [artifact 14162100318](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43714256919/artifacts/14162100318). Workflow used approximately one minute of wall time.

Saved branch checkpoint remains `16 120 120 30 60`; saved candidate remains `120 120 30 60`. Isolated epoch 17 produced provisional weights `120 90 30 60` with training fitness 154.85. No checkpoint promotion.

Each holdout uses 30 paired synthetic games per scenario (normal, heavy/stress, shifted-heavy). Ratios below are candidate/baseline (v1.7), in order: average survival, estimated score, p10 survival:

| Holdout epoch | Generator | Survival | Proxy score | p10 | Candidate illegal | Candidate combo misses |
|---|---|---:|---:|---:|---:|---:|
| 1901 | normal | 0.9087 | 0.9189 | 1.5161 | 0 | 8 vs baseline 4 |
| 1901 | heavy | 1.0203 | 1.0378 | 0.9444 | 0 | 0 vs baseline 0 |
| 1901 | shifted-heavy | 1.0783 | 1.2086 | 1.0000 | 0 | 1 vs baseline 0 |
| 1902 | normal | 1.1295 | 1.1613 | 0.7805 | 0 | 1 vs baseline 1 |
| 1902 | heavy | 1.1620 | 1.2955 | 1.1176 | 0 | 1 vs baseline 1 |
| 1902 | shifted-heavy | 1.0620 | 1.1564 | 1.2857 | 0 | 0 vs baseline 1 |

The runner printed `TEN_X_VERDICT=NOT REACHED - NO DEPLOY` for both holdouts. No result is an actual Block Blast score. Holdout epochs 1901/1902 were provisional; a complete historical seed audit remains outstanding, and this is not independent verification of the simulator legality/combo logic. Do not reuse these epochs as fresh holdouts.

Next: audit historical seed use, independently check 8x8 three-piece legality and combo-deadline definitions, then pursue research-only changes that materially improve survival rather than repeatedly testing the same candidate. Do not automatically rerun this workflow without a new candidate. Main, production build, deployment, iPhone dylib, saved checkpoint and weights unchanged.
