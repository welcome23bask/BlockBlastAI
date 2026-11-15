# Iteration 25 — compilation blocker corrected; no training claimed

2026-11-15. Reviewed research workflow `.github/workflows/v22-tenx.yml`, checkpoint `16 120 120 30 60`, previous iteration 24 evidence, and Actions runs on research branch. All 10 listed research workflow runs are completed; no overlap was present in that list.

Diagnosed run 43760908521 (job 132955285726, artifact 14180770095): compilation failed before any training at `learning/BLTrainableStrategy.c:324` with `sameSafety undeclared`. Source lines 323 had literal backslash-n text inside a C `//` comment, accidentally commenting out the `tuned` and `sameSafety` declarations. Fixed the escaped-newline formatting on research branch, commit `164c8b0bca05ad0e51af2605a428c75a8354319b`. The research-only 5000/4700 safety thresholds are unchanged. This is a targeted infrastructure repair, not a gameplay improvement or validated compile.

No new training or simulation was run in this iteration. The full historical seed-use audit remains incomplete; epoch 1822 and holdouts 1915/1916 are provisional and cannot be called never-trained. Do not launch a potentially invalid holdout evaluation until a complete ledger is established. Next safe step: compile repaired source and establish audited unused seed epochs before one bounded experiment, then compare unchanged v1.7 on normal, heavy, shifted-heavy, with independent repeat for any apparent pass. Also investigate shifted-heavy whole-tray completion and combo-deadline failures.

No checkpoint promotion, main branch changes, deployment, or dylib build. No 10x verification. Simulator proxy scores are not actual Block Blast scores.
