# Research iteration 8 — 2026-11-14 (US/Central)

**10× challenge: NOT VERIFIED. No new training or simulation was performed.**

## Verified current research state
- Research source inspected: `learning/run_tenx_reproducible.sh`, `learning/Makefile`, previous iteration-7 audit, checkpoint and candidate files.
- Saved checkpoint: `16 120 120 30 60`; candidate weights: `120 120 30 60` (unchanged).
- Latest research Actions runs returned by GitHub: [successful 43639191903](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43639191903) on `9c4d80bda51e6b86c4ff66d20d71e47bb97541d4`, and [failed 43639153599](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43639153599). Neither verifies the latest two-holdout acceptance gate.
- Other recent Actions runs belong to a separate v3.1 branch and are irrelevant to this v2.2 performance challenge.

## Execution and reproducibility audit
- Attempted `git ls-remote` against the repository from the execution container. It failed with `Could not resolve host: github.com`; no executable exact-source checkout was available.
- The GitHub connector successfully read source files, but cannot itself compile or execute the C simulator. Do not substitute a different locally compiled executable.
- Runner deletes raw training/holdout evidence at exit via its temporary-directory cleanup. A future successful run must preserve the full terminal transcript and exact source SHA, or explicitly export evidence before cleanup.
- Runner excludes a few previously used epochs but does not comprehensively prove freshness across all prior logs. Audit seed/epoch history before selecting two independent holdouts.
- Runner validates summary verdict strings, but independently reproducible verification also requires raw per-generator metrics, actual legality and combo checks, unchanged v1.7 baseline, and a second independent fresh-seed replication.
- Synthetic score is **not** an official Block Blast score. A capped 1,400-move episode can also mask ratios.

## Outcome
No training, new holdout, candidate promotion, CI dispatch, or GitHub Actions minutes. No changes to main, deployment, iPhone dylib, or saved checkpoint. Next action when exact-source execution becomes available: audit unused seeds, run the clean-branch script with full evidence capture, independently reproduce any candidate meeting every gate, and only then consider stopping research.
