# BlockBlastAI research iteration 87 — 2026-11-18 US/Central

Scope: research branch ONLY; no main, v3.4 iPhone, deployments, v1.7 baseline, or validated checkpoint edits.

Reviewed exact workflow .github/workflows/v22-tenx.yml, runner learning/run_tenx_reproducible.sh, cross-role ledger learning/tenx_used_epochs.txt, iteration 86 failure log, learned-hazard protocol, raw-seed audit implementation, and recent repository-wide Actions. Confirmed zero in_progress/queued/waiting/pending/requested runs before launch (GitHub REST Actions repository-wide filters all returned zero). Last completed iteration 85/86 analysis: shifted-heavy p10 regression 18 vs 19 on holdout 352; strict 10x gates failed.

NEW bounded test: source hypothesis commit f81a90a1, legal hypothetical small-piece line-clear rescue of shifted-piece access, first time under training/simulation. This is distinct from previous escape-family predicates; no hidden future RNG at inference. Unchanged v1.7 SMART SURVIVAL comparison with paired normal, heavy, shifted-heavy 8x8 three-piece games, two fresh 30-game/generator holdouts, one bounded training epoch.

Audited train epoch 100 and holdouts 101,102: distinct and absent from 171 documented ledger epochs; 342 proposed unique raw PRNG starts, no alias to any documented epoch under exact legacy formula across scenarios 0/1/2 and games 0..29,100..107. Accessible historical run metadata reviewed, but undocumented external/local history remains incomplete.

Workflow-only trigger snapshot 521e498f90c8cb2d4821414b9fe5ee3110588c51 launched Actions run 44299241503. GitHub job step "Fail-fast seed-policy check before compiling" completed SUCCESS, then compilation/training/simulation began. After preflight passed, ledger reserved 100/101/102 in separate commit 255e9e6f; this does NOT alter immutable executing snapshot. Strict gates unchanged: >=10x mean survival and synthetic estimated score per generator, >=10x p10, zero illegal/censored, no worse combo breaks/deadlines, independently replicated. Synthetic score is NOT actual game score.

At log creation outcome pending; preserve Actions artifact even on failed gates. Next run must inspect jobs, full holdout evidence, worst failure mode and implement one targeted new research-only hypothesis if gates fail. Do not promote checkpoints.
