# BlockBlastAI v2.0 — Offline Learning Lab

**Status: WORKING TRAINING PIPELINE. NO IPHONE UPDATE AUTHORIZED.**

This branch is based on the user's v1.7 black-block vision build. It preserves all current phone source without changing the installed .dylib.

## What actually learns

- `learning/learn.c` implements a deterministic 8×8 turn-by-turn simulator with 28 shape templates, sequential three-piece trays, simultaneous row/column clears, and an **approximate** combo-and-score model.
- It runs an evolutionary self-play optimization: the learner changes **12 real numerical decision weights** in response to how the resulting games actually end.
- `learning/learn_combo_guarded.c` improves the policy by verifying a complete legal continuation with the remaining visible tray whenever the combo is due on the next move. **A single cleared line is sufficient.** It never selects an unsafe clear merely to protect a streak.
- `learning/models/candidate_combo_weights.json` is a candidate learned model, **not deployed**; training scripts export machine-readable files with `promoted_for_ios:false`.
- The method is **not PPO or a neural network**: it is evolutionary optimization of a compact board-value model. The existing SMART SURVIVAL planner remains the superior baseline until proved otherwise.
- No screenshots or live gameplay are transmitted, and nothing learns or mutates the game on the iPhone.

## Dataset / environment limitations

- Each game has an 8×8 grid; 3 visible fixed-orientation pieces; 28 simulated shapes; uniform and big-piece stress generators; clear timing modeled as 3-move window; approximate score proxy capped at 8x streak.
- **These assumptions are not verified as the official Block Blast game's actual generation or point rules.** Training against a wrong generator can make a model look good in the simulator but perform poorly in the app.
- A 200-move training cap is imposed to prevent unbounded tests. Use fresh **paired** seed streams for evaluation, withheld from all training adjustments.
- The real iPhone app still uses the v1.7 three-piece SMART SURVIVAL solver. YOLOv8n remains optional and does not have trained weights.

## Reproduce in the repository

On Linux/macOS with a C11 compiler:

```bash
cc -O3 -std=c11 -Wall -Wextra learning/test_learning.c -lm -o /tmp/learner_test && /tmp/learner_test
cc -O3 -std=c11 -Wall -Wextra learning/test_combo_guarded.c -lm -o /tmp/combo_test && /tmp/combo_test
cc -O3 -std=c11 learning/learn_combo_guarded.c -lm -o /tmp/blockblast_learn
mkdir -p /tmp/blockblast_v2_results && cd /tmp/blockblast_v2_results
/tmp/blockblast_learn 42 5 40 250
```

This performs **10,080 training episodes** (42 rounds × (1 incumbent + 5 candidate policies) × 40 seed pairs) and 500 held-out episodes (250 per policy). Outputs are JSON. Tested in GitHub Actions workflow `v20-combo-learning.yml`, successful run **43632573285**.

## Actual held-out results

### 1. Learning vs hand-weighted one-step policy (250 games per agent)

| Metric | Hand-weighted | Combo-protected learned |
|---|---:|---:|
| Average moves survived | 27.972 | 30.672 |
| Median moves | 23 | 25 |
| Bottom 10% threshold | 13 | 13 |
| Estimated score proxy | 553.132 | 644.188 |
| Combo breaks per game | 1.916 | 2.056 |
| Combo breaks per 100 moves | 6.85 | 6.70 |
| Avoidable skipped combo-deadline clears | 0 | 0 |
| Illegal moves | 0 | 0 |

The holdout gate passed on these conditions: >=3% improved average survival, no reduction in p10 survival, no >3% increase in *breaks per move*, <=1 missed safe deadline clear and >=98% of baseline proxy score.

**This is progress, but it is a comparison to a WEAKER one-step hand-weighted policy, not to the actual app solver.**

### 2. Exact current v1.7 SMART SURVIVAL vs one-step learner (24 games per distribution per agent)

| Scenario | v1.7 avg moves | Trained one-step avg moves | v1.7 score proxy | Trained score proxy |
|---|---:|---:|---:|---:|
| Mixed piece generator | **124.958** | 38.500 | **5,127.4** | 815.5 |
| Difficult big-piece generator | **32.875** | 16.833 | **1,181.3** | 258.0 |

**The learned one-step solver is much worse than the current three-piece solver.** This proves why replacing the phone engine would be a regression.

### 3. Hybrid prototype: learned evaluator re-ranks only legal v1.7 whole-tray plans (24 games per distribution per agent)

| Scenario | v1.7 avg moves | Hybrid avg moves | v1.7 score proxy | Hybrid proxy |
|---|---:|---:|---:|---:|
| Mixed piece generator | **124.958** | 119.667 | **5,127.4** | 4,928.5 |
| Difficult big-piece generator | 32.875 | 32.875 | **1,181.3** | 1,077.8 |

- Mixed-case lower-tail p10: v1.7 56 vs hybrid 58 (small sample; does not offset mean/proxy decline).
- Mixed-case combo breaks per 100 moves: v1.7 2.30 vs hybrid 2.19.
- No invalid moves or avoidable missed safe combo clears.
- Experimental hybrid changed 234 selections across 48 simulations.
- **Hybrid remains worse in mean survival and proxy score. Do not deploy it.**
- Comparative benchmark code in `learning/compare_v17.c` and `learning/compare_hybrid.c`; CI successful runs **43632792905** and **43633022467**.

## Release gate

No learning model will be compiled into or recommended for the iPhone until a candidate:
1. Improves **both average and 10th-percentile** game survival against the **actual v1.7 SMART SURVIVAL engine** across many fresh, paired, unseen piece sequences.
2. Maintains or improves verified combo-break rate per move, keeps all possible safe deadline clears and never chooses illegal moves.
3. Performs across substantially different simulated generators and on real logged board/tray transitions.
4. Keeps the black-block scanner, original blink-highlight preference, always-visible AI button and fallback engine stable.
5. Passes real-device testing in the same game version and skin. The simulator's score must never be presented as an official game score.

**Current verdict: DO NOT SHIP A NEW DYLIB.** Continue research with a *whole-tray learning objective* and better real-game piece-generation evidence before another release.
