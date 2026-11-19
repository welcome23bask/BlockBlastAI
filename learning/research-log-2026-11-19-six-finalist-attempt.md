# Research attempt — 2026-11-19, six legal finalist afterstates

Branch: feature/v2.2-10x-research-gate ONLY. No main, live v3.4, iPhone build, deployment or validated checkpoint edits.

Prior completed run 44513261386 failed strict gates: heavy mean survival 1.1897x/1.3962x, shifted-heavy 1.1985x/1.1906x, and heavy p10 0.9412x on first holdout. Diagnostic hazard fitting remains disconnected from policy; this experiment tests a different **policy search** hypothesis, not hazard coefficient retuning.

Research source commit f23b76f81db42ff840b70b26ee4152c9f06741d0 increases exactly legal complete-tray finalist diversity from four to six in learning/BLTrainableStrategy.c; the future-geometry late reranker still uses only observable board state. Workflow commit cdea7c4606c5771d87d9b5530e74ae6f8f5e0a95 updates hazard train/validation epochs to 610/611, strategy train to 612, and paired holdouts to 613/614. All five are 17..2000, distinct, absent from the 222 accessible documented ledger epochs, and no 20/40-epoch alias relative to the ledger or each other. Unrecorded external/local seeds cannot be excluded. Actions listing showed zero queued and in-progress runs immediately before commit. The workflow push trigger remains configured, 12-minute limit and all strict gates unchanged.

At immediate post-commit check, no Actions execution for cdea7c46 was yet visible. **No preflight, training, simulation, or 10x outcome is verified yet**. Do not reserve 610..614 until execution snapshot is confirmed; do not rerun with the same epochs/settings. Follow up with Actions run/job/logs and preserve failed evidence. Synthetic estimated score is not real-game score. No checkpoint promotion.
