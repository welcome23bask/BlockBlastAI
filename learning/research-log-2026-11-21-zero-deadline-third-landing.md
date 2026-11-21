# 2026-11-21 — zero-missed-deadline third-landing admission hypothesis

Scope: research branch `feature/v2.2-10x-research-gate` only. No live v3.4, main, deployment, or validated checkpoint changes.

## Evidence
Fetched the exact workflow `.github/workflows/v22-tenx.yml`, runner `learning/run_tenx_reproducible.sh`, cross-role ledger `learning/tenx_used_epochs.txt`, iteration-44 log, frontier, learned-hazard protocol, current C source and regression guard. Rechecked Actions run [44837049501](https://github.com/welcome23bask/BlockBlastAI/actions/runs/44837049501): preflight and hazard stage completed, strict simulation gate failed, artifacts uploaded. Earlier inspected holdouts showed heavy p10 0.8500x (797) and normal p10 0.9737x (798). These are synthetic metrics, not real-game scores.

## New targeted hypothesis (unverified)
The optional +75 crowded-board third-landing finalist admission reward previously required all three irregular families to retain three exact landings, a live unbroken streak and fewer than 20 free cells. It could still reward paths with recorded missed combo deadlines. Add `path->misses==0` to this bonus only, so a deadline-missing path cannot displace a deadline-safe finalist solely because of irregular landing surplus. Existing legal move generation, baseline v1.7, search budgets, other bonuses, and strict 10x gates are unchanged. Source commit `bea4275283b0d32516a6eeb7c68258efb0be3149`; regression guard commit `b52274e11353803ba05f25ebf97594a28f7db3d9`.

## Execution status and limits
This is a **source-only hypothesis, not a completed training/simulation improvement**. It has not been compiled or tested in CI. Current workflow uses already-reserved hazard 794/795, policy 796, holdouts 797/798. The connector can fetch jobs/logs by known run ID but cannot enumerate repository-wide active Actions; therefore safe non-overlap was not established and no workflow trigger or seed reuse occurred. Accessible ledger history may omit external/local runs.

Next safe execution: verify no concurrent Actions run; audit five fresh distinct epochs against accessible history and raw seed aliases; update all workflow references consistently; pass preflight; run bounded fresh paired normal/heavy/shifted-heavy 8x8 three-piece comparisons to unchanged v1.7, preserving failure artifacts. Never claim or promote 10x without two independent full strict per-generator mean survival, synthetic estimated score, p10, combo/deadline, legality and censor gates. No official game score claim.
