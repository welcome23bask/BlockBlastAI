# Iteration 45 — crowded-board orthogonal escape corridors

Research branch ONLY; no live v3.4, main, deployment or validated checkpoint edits.

## Hypothesis and change

Iteration 44 heavy-piece mean survival was just 1.0164x/1.0665x with no 10x pass; fragmentation was turned OFF afterward. Instead of repeating fragmentation or the failed hard-zero floor, source commit `9635b245c99513d8041397c1d3c1c2b3bfbaaf65` introduces `BL_RESEARCH_ORTHOGONAL_ESCAPE=1`: for crowded completed-tray boards with fewer than 40 empty cells, reward the *minimum* of the exact legal horizontal and vertical five-bar landing counts (capped at six, scaled 95), only when both are nonzero. This is a new board-geometric two-orientation escape hypothesis, **not** the learned death-hazard model from the separate protocol. Existing future-clear-access remains enabled; disconnected-region penalty, hard-zero floor and prior rejected strategies remain OFF. Exact current-tray legality and unchanged separately compiled v1.7 remain the comparison standard.

## Launch and audit

Workflow-only commit `f9ebc6f679a6e2d36b5efc31c4be6fcf3be6853f` updates `.github/workflows/v22-tenx.yml` to train epoch **1501** and independent holdouts **1552,1603**; prior to editing, accessible Actions showed no overlapping running job. Exact workflow, runner and ledger fetched from research branch. The three IDs are absent from accessible cross-role ledger. Independent preflight arithmetic checked 342 distinct new raw xorshift starts (30 normal/heavy/shifted-heavy games and eight stage-two training-offset games per epoch) against 7,828 documented historical starts, with no alias, including cross-generator 20/40-epoch offsets. This is a supplementary audit; the runner's own preflight must pass in Actions before training. Local/external unlogged historical seed use cannot be ruled out. The ledger was updated after the workflow-triggering commit in `e6dfe244fbc84df41b919f9191e3a9e232b017f9`, so the executing commit snapshot is unaffected. The 12-minute bounded run should compile, train 5 paired games/generator, vet up to 4 finalists on 8 disjoint training games/generator, then evaluate two 30-game/generator paired holdouts with strict 4096 zero-censor gate, upload evidence on failures and reject any sub-10x results. Synthetic estimated score is NOT actual game score.

## Outcome

Pending verification of Actions run/job/preflight and complete metrics. No 10x claim or checkpoint promotion. The heavy-piece survival/score and shifted-heavy lower tail are priority failure modes. A trained calibrated hazard model remains a future distinct hypothesis; this experiment does not claim to implement it.
