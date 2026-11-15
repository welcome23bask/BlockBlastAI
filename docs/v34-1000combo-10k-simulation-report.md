# v3.4 combo 1000 tracking + 10,000-game synthetic learning attempt

Research scope: 10,000 synthetic training games (a separate fast greedy
policy inspired by 35 **unreviewed** screenshot-labeled shapes), plus 1,200
independent validation games. Each training game proposes a weight change,
compares the same generated board and tray stream under old vs proposed
weights, and checks a positive proposal on another 24 paired training
games across 3 generators. Failed proposals are not promoted. This is
**not** the same beam planner running on the iPhone and the training
results must not be described as the v3.4 AI reaching 100 combos.

Reproducible source/report provided as downloadable conversation artifacts;
the iPhone build includes only the independently tested **streak tracking
capacity** change, not the unverified synthetic weights.

Exact C simulation results:

```
TRAIN games=10000 mean_moves=21.506 score_proxy=346.934 mean_lines=7.721 mean_max_combo=4.776 highest_streak=32 reached_100=0 reached_1000=0 invalid=0 combo_breaks=14756
LEARNING_ATTEMPTS=10000 ACCEPTED_IN_SAMPLE=129 REJECTED=9871 CANDIDATE_WEIGHTS=220,190,10,80
HOLDOUT_BASE games=1200 mean_moves=20.703 score_proxy=333.634 mean_lines=7.346 mean_max_combo=4.544 highest_streak=30 reached_100=0 reached_1000=0 invalid=0 combo_breaks=1695
HOLDOUT_TRAINED games=1200 mean_moves=20.777 score_proxy=334.026 mean_lines=7.376 mean_max_combo=4.537 highest_streak=28 reached_100=0 reached_1000=0 invalid=0 combo_breaks=1707
HOLDOUT_GATES_FAIL
```

Interpretation: 129 locally promising training mutations were accepted, but
independent holdout combo retention was slightly worse, and no generated
game reached streak 100. The candidate is **not promoted** to the iPhone
strategy. No claim of a real-game score improvement or a successful
10× outcome. The simulator's scoring is a proxy, not official game scoring.

Production change:

- `BL_COMBO_STREAK_MAX` raised to 1000.
- `BLState.comboRun` widened from unsigned char (0..255) to `uint16_t`
  so the original fallback beam solver never wraps at 255.
- All existing combo deadlines, vision, five/now fifty-scan clear proofs,
  Smart Survival, move highlights and original v3.4 strategy preserved.
- Existing regression `test_v34_combo100.c` now exercises 1000 consecutive
  verified clear events, saturation, 3-miss reset and correct solver handling
  of a live combo value of 1000.
- A counter that supports 1000 does **not** guarantee a policy capable of
  reaching 1000. A credible next step is held-out full-game simulated
  search with deeper multi-piece planning, after validating real piece odds.
