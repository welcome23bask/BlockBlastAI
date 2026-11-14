# BlockBlastAI v2.3 — Paired training and combo deadline research

Research date: 2026-11-14. **This is an offline simulator experiment, not a phone .dylib release.** The official Block Blast generator and score formula have not been established.

## What was changed
- Replaced the unsafely small independent training-only objective with paired, seed-matched evaluations of the **frozen v1.7 SMART SURVIVAL** solver and the trainable whole-tray search.
- Added lower-tail loss penalties, combo-break-rate penalties, extra cost for safe deadline clears skipped, and separate normal, hard, shifted-heavy piece generators.
- Added deterministic small-line combo rescue (in `train_tenx.c`): when an established combo is on its third-chance move, search for an immediate line clear that can still place **all remaining visible pieces**. This avoids sacrificing the entire tray just to save the streak.
- Kept the 18k-node training search separate from the independent 180k-node validation search.
- Kept all research changes outside the existing v1.7 iPhone solver and black-block scanner.

## Training output
GitHub run [43644411694](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43644411694) trained for 3 rounds, 8 mutations per round, 4 paired seeds per generator per training batch. The logged `episodes=1296` counts **both baseline and candidate simulations** (648 matched pairs). Last trained search weights: `120 -10 30 160`.

Held-out paired validation: **24 independent seeds per generator**, and two separate held-out seed epochs (941 and 952), a total of 144 baseline and 144 candidate game episodes across the two epochs. Both policies were simulated with the same seed stream in each corresponding pair.

### Holdout 941

| Piece scenario | v1.7 average moves | v2.3 average moves | Change | Combo deadline misses v1.7 → v2.3 |
|---|---:|---:|---:|---:|
| Normal | 119.250 | 117.542 | **−1.4%** | 0 → 0 |
| Difficult | 40.208 | 35.542 | **−11.6%** | 1 → 0 |
| Shifted heavy | 33.417 | 37.542 | **+12.3%** | 1 → 0 |

### Holdout 952

| Piece scenario | v1.7 average moves | v2.3 average moves | Change | Combo deadline misses v1.7 → v2.3 |
|---|---:|---:|---:|---:|
| Normal | 150.750 | 205.292 | **+36.2%** | 0 → 0 |
| Difficult | 36.833 | 45.083 | **+22.4%** | 0 → 0 |
| Shifted heavy | 33.500 | 35.458 | **+5.8%** | 0 → 0 |

- No illegal moves were found in the two holdouts.
- The strict small-clear rescue passed a test that checks exact move legality, line clearing, and all remaining tray-piece placements.
- The candidate had zero avoidable missed combo-deadline clears across both held-out test sets.
- Some survival, proxy-score, combo-break-rate and lower-tail metrics still regress on certain seeds/distributions. This candidate is **NOT consistently better than v1.7** and does not satisfy the 10× requirement.
- The supervised/linear model does not represent a neural network; these four numbers are trained weights within a complete three-piece search evaluator. There is no proven access to future hidden game pieces.

## Conclusion
**Do not promote the trained checkpoint or change the iPhone dylib.** The new learning loop is more rigorous and the combo safeguard fixed an observable simulator failure. Further work needs more diverse and real-world piece distributions, greater paired holdout sample sizes, a better simulator of official scoring, and progress on worst-case and hard-piece decisions.
