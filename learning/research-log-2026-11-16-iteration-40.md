# Iteration 40 — conservative v1.7 reference-anchored risk shield (research only)

Date: November 16, 2026 UTC. ONLY branch `feature/v2.2-10x-research-gate`, no live iPhone/main/checkpoint edits.

## Diagnosis and hypothesis

Iteration 39 [run 43804845920](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43804845920) improved combo-break rates but failed: normal mean survival 0.9917x on holdout 1961, normal p10 0.8200x on 1962 and shifted-heavy survival only 1.0221x on 1962. All four stage-2 candidates were rejected for some regression. Preserving more risky afterstate finalists was not sufficient. Source flag `BL_RESEARCH_DUAL_POOL` therefore default OFF again; previous failed bottom-three, next-tray rollout and proactive override all remain OFF; single-minimax hard coverage remains as last relatively combo-safe research variant.

New **model-selection hypothesis** in `learning/BLTrainableStrategy.c` commit `c190508a0fd0c73ea96fc083d9e5bbce3aad723e`: on a crowded afterstate (>32 filled cells) or an urgent active streak with at least one miss, compare the research candidate's fully legal current-tray plan with an independently compiled **unchanged v1.7 SMART SURVIVAL** full-tray plan from the *same visible board and same three pieces*. A baseline plan is substituted only if it fits at least as many current pieces, does not worsen any known combo deadline, and either rescues full placement, better preserves combo, or exceeds the research plan by more than 1000 in the preexisting board-geometry + min-hard coverage + future-pair feasibility score. No access to future RNG or true game points; v1.7 source untouched and separately remains the control in paired holdouts. This is an ensemble fallback, not a claim about actual app scores or guaranteed performance. It does more computation but remains under the 12-minute bounded workflow limit, with compile/selftests before training. Research-only v1.7 library symbol is linked by the existing runner; no phone code changed.

## Bounded audit and run

[Actions run 43805340214](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43805340214) triggered by exact research workflow-path update commit `a8ad228c1390bb0872d566a7021f15454f2c9637`, after confirming no concurrent Actions jobs and reviewing exact workflow, runner and cross-role ledger. Provisional *training* epoch **1840**, two distinct independent holdouts **1963,1964**, unused by accessible ledger and recorded historical runs; unlogged external/local use cannot be disproved. GitHub checkout and fail-fast seed preflight both passed before training; ledger reservation committed afterwards as `e44deb7d32b3c32da9b947d441efa739fffdf22f`, leaving the running seed snapshot unchanged. Twenty-one candidates on five paired training games/generator, at most four stage-2 finalists on eight disjoint training games/generator, then two 30-game/generator paired holdouts. Twelve-minute cap; failed evidence artifacts retained. No promotion possible from this workflow.

## Gates exactly unchanged

Per generator and per independent holdout: ≥10x mean survival, ≥10x **synthetic estimated** score, ≥10x p10 survival; zero illegal moves, no worse combo breaks per move and avoidable combo deadlines, no capped games at 4096, independent confirmation before 10x claim. Baseline `src/BLDualStrategy.c` remains unchanged. Synthetic score estimates are **not actual Block Blast game points**. The phone v3.4 Automatic Play, main, deployment, saved checkpoint, and validated candidate remain untouched.

At log creation: CI running, no outcome claimed.
