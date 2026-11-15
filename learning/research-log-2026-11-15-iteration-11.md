# Research iteration 11 — 2026-11-15 (US/Central)

**Verdict: NOT VERIFIED. No training or fresh-seed simulation executed.**

## Live source and Actions audit
- Research branch HEAD: `19b2af9417a756bc36894d0087de8e0279e03d51`.
- Saved checkpoint: `16 120 120 30 60`; candidate weights remain `120 120 30 60`.
- Read the exact branch's `learning/train_tenx.c`, `learning/run_tenx_reproducible.sh`, `.github/workflows/v22-tenx.yml`, checkpoint, and iteration-10 audit via the connected GitHub API.
- Latest research Actions run: [43639191903](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43639191903), success 2026-11-14 20:46 UTC, on old SHA `9c4d80bda51e6b86c4ff66d20d71e47bb97541d4`. Its success is **not** a 10x acceptance.
- Current workflow tests the old candidate on epoch 93 **before** the smoke-training step, greps only for existence of a verdict, and uploads an artifact named "Verified" even if the verdict is negative. Its test epoch is already used, and the smoke-test checkpoint is an artifact preview, not a promoted checkpoint.
- The standalone runner requires two caller-verified unused epochs, executes one training epoch with one seed per scenario and 30 test games per scenario/holdout, and fails closed on negative verdicts. Its known-used-epoch exclusion list is explicitly non-exhaustive. No independent replication runner has been executed.
- The simulator has a 1,400-move ceiling, so censored survival comparisons cannot establish unbounded 10x gains. Its score is a proxy, not an official Block Blast score.

## Execution access
Attempted `git ls-remote https://github.com/welcome23bask/BlockBlastAI.git HEAD` from the execution container; DNS failed: `Could not resolve host: github.com`. The GitHub connector supports repository text reads/writes but does not expose a fresh workflow-dispatch operation or execute C source. No substitute simulation or invented measurements used.

## Next step
In an executable exact-source checkout, audit all previously used training and evaluation seed epochs; select two disjoint unused holdout epochs plus separate independent replication epochs; run one bounded training epoch and paired 8x8/three-piece evaluations across normal, heavy and shifted-heavy generators. Preserve raw per-game evidence, compiler/source hashes, training checkpoint preview and exit codes. Require all specified mean-survival, proxy-score, p10, zero-illegal, combo-break-rate, and avoidable-deadline gates on **each** generator and again on independent seeds before any verification claim.

No Actions minutes spent, no main changes, merge, deployment, checkpoint promotion, or iPhone dylib build.
