# Research iteration 10 — 2026-11-15 (US/Central)

**Verdict: NOT VERIFIED. No new training or simulation performed.**

## Exact-source and checkpoint audit
- Research branch: `feature/v2.2-10x-research-gate`.
- Saved checkpoint `learning/tenx_checkpoint.txt`: `16 120 120 30 60`.
- Saved candidate `learning/tenx_candidate_weights.txt`: `120 120 30 60`.
- Inspected `learning/run_tenx_reproducible.sh`, `learning/README.md`, `.github/workflows/v22-tenx.yml`, and the previous iteration-9 audit through the connected GitHub repository.
- Latest branch-specific GitHub Actions research run visible: [43639191903](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43639191903), success on 2026-11-14 at commit `9c4d80bda51e6b86c4ff66d20d71e47bb97541d4`. It is not evidence of the required independent two-holdout 10x gate.

## Execution attempt and limitation
- Reattempted `git ls-remote https://github.com/welcome23bask/BlockBlastAI.git HEAD` in the execution container; failed with `Could not resolve host: github.com`.
- GitHub connector can read/write repository text but does not provide execution of the C benchmark or dispatch of a new workflow in this session. No substitute simulator or unverified binary was used.
- Existing workflow checks epoch 93 and does not fail merely because its verdict is negative; therefore a successful workflow is not a verified performance pass. The standalone runner requires a clean research checkout and two unused epochs; its preserved evidence behavior has not yet been execution-tested.

## Next reproducible experiment
1. In a network-enabled clean research-branch checkout, audit the entire training/evaluation seed history to select two never-trained, disjoint holdout epochs (and additional disjoint epochs for independent replication). Do not assume the script's small exclusion list is exhaustive.
2. Execute exactly one bounded training epoch with the checked-in standalone runner. Preserve source commit, compiler version, baseline hash, candidate weights, per-game results, raw logs and exit codes outside the checkout.
3. Evaluate unchanged v1.7 against the candidate on paired seeds separately for normal, heavy, shifted-heavy, enforcing 8x8 and three-piece legality. Require each generator to meet >=10x mean survival and >=10x estimated score, >=10x p10 survival, zero invalid placements, no worse combo breaks per move and no more avoidable combo deadlines. Beware the 1400-move censoring cap.
4. Independently reproduce every gate on additional unseen seeds before claiming verified; never call simulator proxy scores actual Block Blast scores.

No checkpoint promotion, Actions dispatch/minutes, main changes, merge, deployment or iPhone dylib build. Research remains unverified.
