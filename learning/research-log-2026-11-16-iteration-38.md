# Iteration 38 — combo-preserving future-pair Pareto routing (bounded research only)

Date: November 16, 2026 UTC. Strict research branch only; main, v3.4 iPhone/Automatic Play, production binaries, and saved checkpoint are immutable.

## Prior failure and targeted new hypothesis

Run 43803697188 (iteration 37) compiled and performed both paired 30-game/generator holdouts. Heavy and shifted-heavy gains remained approximately 1.05–1.29x, and shifted-heavy holdout 1957 combo-break rate *increased* to 2.894/100 moves from unchanged v1.7's 2.166/100. All second-stage weight variants were unsafe or regressed. So the bottom-three shape-coverage feature is retained but set OFF; earlier single-minimax hard shape reranker restored ON. No failed candidate promoted.

New feature in source commit `fb0be7172dfe1fa81842b72e7f57c53a28346199` changes the **decision rule**, not the four weights: with a witnessed active combo and one prior miss, a *proven exactly legal complete current three-piece sequence* that preserves combo is preferred beyond the old utility-gap threshold only if (a) it does not destroy the last 3x3/horizontal/vertical five-bar placement on a crowded board, AND (b) it retains at least as many of six pairwise challenging-future-shape coexistence checks as the alternative survival route. An urgent last-chance current-tray clear and other existing safety/legality guards remain. The hypothetical future pairs are geometric stress proxies, not actual hidden next pieces. New rule regression assert requires a proactive row-clear within the next two moves. Source/test follow-up commits `1b0c2e52d7d4a65b5af8bf955c3cc2d37d4c4645` and `5a2a090fadd0a9fe30fb5e0041d366bfa9f7f2d2`.

## Actual bounded CI experiment

[GitHub Actions run 43804331999](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43804331999) triggered by research workflow-only push commit `f7066aa59f3d2d77457c2db3b3da3215f22e567d`, after confirming the Actions listing had no concurrent jobs and the exact workflow path, runner and cross-role seed ledger. Disjoint provisional training epoch **1838** and holdouts **1959 and 1960**, all unused in accessible existing ledger/recent Actions. The full historical external/local seed set may be incomplete. Actions checkout and fail-fast seed preflight succeeded before training. All three epoch IDs were reserved in ledger only **after** checkout/preflight (commit `f93d7b935a4e8fb967dd9c1474d617eb22222dbf`), without altering the executing seed snapshot. Five paired training games/generator, up to four finalists with eight disjoint training games, two separate 30-game/generator paired holdouts. Timeout 12 minutes, failures artifacts uploaded. No source promotions.

## Unchanged strict 10x criteria

Independent unchanged v1.7 SMART SURVIVAL baseline; exact 8x8/three-piece moves; all three fresh normal, heavy, shifted-heavy generators; mean survival ≥10x and synthetic estimated score ≥10x for **each** generator and holdout, p10 survival ≥10x, illegal moves zero, combo breaks/move and avoidable combo deadlines no worse, zero censored runs under 4096, independent replicated holdout confirmation. Synthetic estimated score is **NOT** the actual Block Blast game score. Nothing may be deployed even if partially improving; record all failures and diagnose worst mode.

At log creation: experiment underway, no measured result claimed.
