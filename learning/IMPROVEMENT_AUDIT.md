# Learning AI — Comprehensive improvement audit, November 14, 2026

The scope is the learning/decision-making system. Visual detection, iPhone overlays, proprietary game mechanics and score multipliers are separate systems. "All possible improvements" is unbounded; this lists the major practical weaknesses and makes their current status explicit.

| Priority | Improvement | Status in v2.1 | What blocks full completion |
|---|---|---|---|
| Critical | Replace one-step-only learner with three-piece end-state evaluation | **Implemented**: v1.7 generates complete legal 3-piece plans; 12-feature value model judges final board state | Need stronger counterfactual training to outperform the baseline |
| Critical | Keep even one-line combo streaks when safe | **Implemented in guarded arbitration**: exact combo transitions, immediate-clear deadline and no destructive alternative | Real game's combo timing is not verified |
| Critical | Never recommend illegal or impossible three-piece sequences | **Implemented and tested**: solver only supplies legal sequences; model only ranks complete sequences | Real screen-detection mistakes can still misrepresent the game state |
| Critical | Realistic scoring formula | **Not solved**: reports estimated point proxy only | Need observed real-game point deltas with multiplier off/on |
| Critical | Actual upcoming-piece generator and distribution | **Not solved**: 28 templates and three synthetic generators | Need anonymized actual tray sequences and their frequency, especially stressful/forced trays |
| Critical | Reliably beat current v1.7 SMART SURVIVAL before a phone update | **Not yet**: the trained ranker hurts some games; safe guard removes most deviations | Need better predictive features, more data and proper long-run holdout |
| High | Learn real long-horizon consequences, not immediate line reward | **Implemented**: trajectory remaining-move targets from completed trays | Labels have high variance; predictive R² 0.094 is weak |
| High | Prevent adjacent-frame leakage and benchmark overfitting | **Implemented**: grouped train/validation split by entire game; separate game seeds for benchmark | Add more time-separated sessions and independent test distributions |
| High | Include 3×3, long-bar, rectangle and shape diversity features | **Implemented** | Expand from 28 to observed shape catalog and handle rotations/versions |
| High | Protect severe lower-tail game-overs | **Implemented in evaluation**: p10 survival reported; model cannot discard big-shape availability | Need much larger holdouts and confidence intervals |
| High | Adversarial heavy-piece stress and generator shifts | **Implemented**: 3 generators, including unseen 75%-hard challenge | Still synthetic and may not reflect real Block Blast |
| High | Learn uncertainty rather than trust every value prediction | **Partly implemented**: conservative value-advantage threshold and safe fallback | Predictive confidence intervals / ensembles not yet trained |
| High | Sequence replay / experience buffer | **Partly implemented**: CSV of v1.7 tray-boundary trajectories with game identifiers | Need many more sessions and model-error-driven replay sampling |
| High | Train against real on-device trajectories | **Not yet** | Actual board/tray/score recordings, consent and exact parsing |
| Medium | Multi-head model for survival, combo-break risk, and future-tray feasibility | **Planned** | Accurate labels and broader data necessary |
| Medium | More capable nonlinear value function | **Benchmarked offline**: simple boosted/tree models did not outperform linear reliably on this small dataset | More examples and uncertainty/calibration testing |
| Medium | Multi-step Monte Carlo future piece rollouts | **Not yet** | Unverified piece generator; costs too high for unbounded phone solver |
| Medium | Train on full moves without excessive CPU | **Improved research architecture**: trajectory data collection once, offline predictor tuning, expensive head-to-head only after training | Final full-tray search remains slower than one-step training |
| Medium | Reproducible model, parameters and rollback | **Implemented**: 12 named normalized features, portable JSON, version field, promoted_for_ios=false | No phone deployment until gates pass |
| Medium | iPhone CPU/memory/latency validation | **Not applicable yet** | Requires proven model, ARM64 integration, actual iPhone timing tests |
| Medium | Improve YOLO visual recognition | **Separate experimental integration exists in older branch** | Needs labeled real screenshots and trained Core ML weights |
| Medium | Feedback loop using game-over screenshots | **Not yet** | Need trustworthy before/after state labels and reliable per-session history |
| Low | Display model confidence and alternative move reasoning | **Not deployed** | Depends on validated model and UI design |
| Low | Aim at 1M/100M+ scores | **Unverified aspirational goal** | Strong model, real game data, reproducible live outcomes, long-duration testing |

## Honest decision

Do **not** replace the active v1.7 phone .dylib. The model now learns a real whole-tray survival value, but no tested new policy consistently surpasses the old three-piece SMART SURVIVAL engine. More labels/data and model development are necessary. A successful GitHub compilation or synthetic training score is not enough.