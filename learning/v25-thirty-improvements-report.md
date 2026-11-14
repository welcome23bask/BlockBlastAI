# BlockBlastAI v2.5: 30 coded features, six tested batches

Research only. The iPhone dylib and production v1.7 SMART SURVIVAL code are unchanged. Results use synthetic 8×8 three-piece games and estimated scores, not official Block Blast scoring.

## Six rounds of five improvements

| Round | Five coded candidate features | Selection |
|---|---|---|
| 1 | 4×4 empty area; 4×2 rectangle; 2×4 rectangle; six-long horizontal gap; six-long vertical gap | Rejected (weight 0) |
| 2 | T tetromino fit; S zigzag; mirrored Z zigzag; J corner; L corner | Rejected (weight 0) |
| 3 | Row one short; column one short; row two short; column two short; shared row/column single-cell finish | Rejected (weight 0) |
| 4 | Empty-region count; largest empty region; tiny empty pockets; horizontal four-cell free runs; vertical four-cell free runs | Accepted (weight +1) |
| 5 | Open border cells; open center cells; row occupancy variance; column occupancy variance; cells with 3+ open neighbors | Rejected (weight 0) |
| 6 | Future shape-family coverage; scarce 3×3 penalty; missing long-bar orientation; 8-cell ring fit; two independent 3×3 landings | Rejected (weight 0) |

All 30 are implemented in learning/v25_features.h and staged by learning/train_v25_six_rounds.c. Each group is checked on two paired training seed groups, then independently on a different diagnostic group. Unsafe or low-value proposals are set to zero before the next group is introduced. **Only five of the 30 candidates are active in the selected research model**; do not confuse "coded" with "proved to improve play."

## Final independent evaluation

CI https://github.com/welcome23bask/BlockBlastAI/actions/runs/43649071361 completed successfully. Unit tests include address and undefined-behavior sanitizers. Six training rounds and five fresh, previously unused held-out epochs were run: 14 games x 3 generators x 5 epochs = **210 paired games** or **420 individual simulated games**.

Selected 6 group weights: 0 0 0 1 0 0. Inherited previous 4 heuristic weights: 120 120 0 -20.

| Generator | Original v1.7 average moves | Research candidate average moves | Change |
|---|---:|---:|---:|
| Normal | 130.66 | 167.04 | +27.8% |
| Large and difficult | 31.40 | 37.11 | +18.2% |
| Shifted-heavy | 30.50 | 34.06 | +11.7% |

No illegal moves were observed in those 210 paired simulations. The model missed no avoidable last-chance safe combo clear in these samples, compared with five missed by the original. Combined combo-break rates per move declined slightly in all three generators.

**But 3 of 5 independent held-out epochs failed at least one strict safety criterion**, including lower-tail survival, combo break rate or estimated score regressions. This model did NOT demonstrate robust improvement in every test. It did NOT achieve 10×.

**Final verdict: DO NOT DEPLOY an iPhone dylib.** This is research progress, not a safe replacement.

## Reproduction

Run the GitHub Actions workflow v25-six-batches.yml on the branch feature/v2.5-six-round-thirty-improvements. It builds the unchanged v1.7 comparison solver and the research-only model, tests feature gate isolation, runs six training/test cycles, and publishes weights, CSV, logs and summary JSON.

Next research needs larger and more diverse holdouts, learning individual feature weights rather than groups, improved long-tail survival, and validation against actual in-game piece distributions and scores.
