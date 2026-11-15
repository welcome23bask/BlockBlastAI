# Research check 4 — 2026-11-14 18:22 US/Central

**Gate status: NOT VERIFIED. No training or simulation measurement claimed.**

## Verified current repository state
- Research source: `learning/train_tenx.c`, `learning/BLTrainableStrategy.c`, `learning/run_tenx_reproducible.sh`.
- Saved checkpoint: `16 120 120 30 60`; saved candidate: `120 120 30 60`. No checkpoint promotion.
- GitHub Actions research workflow run [43639191903](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43639191903) completed successfully on 2026-11-14 20:46 UTC. This was an earlier workflow, not an independent held-out 10× verification.
- Exact test source has 8×8 board / three-piece transition validation, normal, heavy, shifted-heavy piece generators, a minimum 10-game limit, and the required 10× survival / estimated score / p10, legality, combo-break and avoidable-deadline gates.
- The simulator's score is a **proxy**, never an actual Block Blast score.

## Execution audit
- Attempted direct checkout via `git ls-remote https://github.com/welcome23bask/BlockBlastAI.git HEAD` in the execution container; failed with `Could not resolve host: github.com`.
- The connected GitHub API allows reading/writing repository source but does not execute C programs. Avoided launching another GitHub Actions workflow to conserve minutes.
- Consequently no exact-source training epoch, fresh never-trained seeds, or independent holdout were executed in this check. Earlier diagnostic results are not promoted or represented as independently verified.

## Next bounded action
In a network-enabled clean checkout, confirm holdout epochs are unused and run `bash learning/run_tenx_reproducible.sh <fresh_epoch_A> <fresh_epoch_B>`. The existing script trains one isolated epoch and runs 30 paired games per generator for each of two independent held-out epochs. Require both holdouts to pass every gate before considering checkpoint promotion or stopping the research automation. The existing runner explicitly avoids writing the tracked checkpoint.

**No changes to main, deployment, production source, or iPhone dylib.**
