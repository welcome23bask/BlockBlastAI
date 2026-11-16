# Iteration 35 — bounded whole-hypothetical-next-tray resilience

Date: November 15, 2026 Central / Nov 16 UTC. Scope: research branch `feature/v2.2-10x-research-gate` only.

## Why this experiment

Previous iteration 34 achieved higher synthetic mean survival, estimated score, p10 survival (non-inferior), lower combo breaks, zero illegal/deadline regressions and zero capped games in both normal/heavy/shifted-heavy holdouts, but only 1.08x–1.81x ratios, well below 10x. Fixed 4-weight adjustments and merely counting individual future shape landing zones are not sufficient for safe *multi-piece* future tray planning.

## Exact gameplay hypothesis

Research-only `learning/BLTrainableStrategy.c` commit `703e4e4ea931b8361fa1024437130062c8450015` augments diverse current-three-piece complete legal afterstate reranking with **three fixed hypothetical challenging next trays**: (1) 3x3 square, five-bar, 2x2 square; (2) two 3x3 squares and 3x2 rectangle; (3) three irregular hard 3x3-bounding shapes. Each probe recursively applies exact 8x8 legal placements and simultaneous row/column clears for all three hypothetical pieces. Each sample has a strict 192-node budget: a legally completed tray earns a modest bonus, an exhaustively impossible tray incurs a modest penalty, and a budget-exhausted/unknown result earns no bonus or penalty. Unlike cheating, no hidden future RNG state is passed to the policy. It remains a **synthetic robustness proxy**, not a guarantee about the real proprietary piece generator.

The code still retains the paired hard-shape space check, urgent last-chance combo rescue, proactive combo protection with dangerous-board veto and unchanged strict 10x benchmark gates. The independent baseline is compiled from unchanged `src/BLDualStrategy.c`.

Research-only unit test commit `47e6ab919f7474403164631dc899f5e31a3abdda` asserts the fixed sample trays can legally be completed from an empty board and cannot be placed on a full board, in addition to the earlier full 3-piece legality and combo rescue regression suite.

## Bounded experiment

[GitHub Actions run 43795171397](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43795171397), head revision `9121faef97860e71b1014861e626fea05c4b4050`, queued at initial log write. It should compile the exact source, perform fail-fast seed preflight, execute the extended legality selftests, train one isolated epoch with five initial games per generator plus up to four candidates on a separate eight-game-per-generator training split, then run two 30-game-per-generator holdouts. Epochs: training 1835, holdouts 1953/1954. All three were absent from the central seed ledger and from 30 accessible research logs; unrecorded external runs cannot be excluded, so strict global never-use certification remains incomplete. No overlapping Actions run was visible when this workflow was triggered. 12-minute job timeout and no candidate promotion.

## Pending verdict and hard constraints

Do NOT infer any new performance before run finishes. Require exact 8x8/three-piece legality, all per-generator >=10x mean survival, >=10x simulated estimated score, >=10x p10 survival, zero illegal placements, no worse combo breaks per move and no more avoidable deadlines, zero capped games and independent fresh-seed replication. Simulated estimates are NOT observed Block Blast points. Saved checkpoint remains `16 120 120 30 60`, saved candidate remains `120 120 30 60`; main, deployment, and v3.4 iPhone dylib unchanged.

## Completed result: rejected rollout hypothesis

[Run 43795171397](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43795171397) completed both real CI compilation/selftests and both 30-game-per-generator paired synthetic holdouts on source `9121faef97860e71b1014861e626fea05c4b4050`. Fail-fast preflight, 8x8/3-piece legality selftests and evidence upload succeeded. Initial training 13/21 candidates passed combo safety, but all four independently checked second-stage training finalists showed at least one survival/p10/proxy regression, so NO weights were selected or promoted; the isolated result retained `120 120 30 60`. The all-generator strict 10x verdict failed both holdouts. All reported placements were legal, with no additional avoidable combo deadlines and zero right-censored games at the 4096 move cap.

| Generator | Holdout 1953 survival / estimated-score / p10 | Holdout 1954 survival / estimated-score / p10 |
| --- | --- | --- |
| Normal | 1.2905x / 1.4258x / **0.8868x** | 1.7491x / 2.0283x / 1.0000x |
| Heavy | 1.0624x / 1.1996x / 1.4706x | **0.9554x** / 1.0354x / 1.0000x |
| Shifted-heavy | 1.1623x / 1.3335x / 1.1765x | **0.9492x** / **0.9591x** / **0.8500x** |

Combo-break rate decreased versus v1.7 in all six comparisons, but the inconsistent survival and lower-tail regressions demonstrate that the three fixed future-tray sample bonuses do NOT generalize reliably. Do not select or deploy this policy. The scripted verifier correctly rejected it. Simulation score estimates are NOT actual phone-game scores.

**Research disposition:** Commit `d390f88ac5cb3e7a93955067c6f3dca8001fbde0` sets `BL_RESEARCH_NEXT_TRAY_ROLLOUT=0` by default, retaining the code behind an explicit opt-in for future controlled A/B. Commit `ca30888039193a112d0966c1dd644de17b2871b5` separately fixed a bounded-search performance defect: recursion exhaustion must short-circuit siblings instead of continuing to enumerate zero-budget branches. Both commits occurred *after* the measured run's source revision, so an exact-source compile regression test on the revised research branch remains required; neither is claimed to improve measured gameplay yet.

The seed ledger now reserves train epoch 1835 and both holdouts 1953/1954 via commit `ec6ccd15896772ef880e4f62b7352dcb1a0324ae`. The saved checkpoint remains unchanged; no main/deploy/iPhone changes. The 10x goal is unverified. Avoid repeating these identical future-tray probes as if they were a new learning attempt.
