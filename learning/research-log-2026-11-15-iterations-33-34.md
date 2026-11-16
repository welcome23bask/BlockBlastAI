# Research iterations 33–34: train/validation split, combo prevention, and uncensored benchmark

2026-11-15 America/Chicago. Research branch `feature/v2.2-10x-research-gate` ONLY, no main/production/iPhone dylib changes.

## Iteration 33 — bounded two-stage training selection

[GitHub Actions run 43793891213](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43793891213), source `b1d082912665a8538b871ed4d01643c367734aed`. It compiled research sources, passed the synthetic 8x8 / 3-piece legality tests, and completed one bounded epoch: first 21 candidate variations each tested on 5 paired games per generator under provisional training epoch 1833; then four finalist candidates tested on eight **different** training-game indices 100..107 per generator. The two holdout epochs 1949/1950 were never used during either train stage and used 30 paired games per generator each. This run used the old 1400-move cap because the later cap repair was not committed at its exact source revision.

Training: 15/21 candidates passed initial combo safety, but only 1/4 passed independent second-stage train-only checks for non-regressive mean survival, p10, estimated score and combo safety across all three generators. Its isolated selected weights were `120 120 90 0`. **No permanent checkpoint or candidate promotion**.

| Generator | Holdout 1949 survival / estimated-score / p10 ratio | Holdout 1950 survival / estimated-score / p10 ratio |
| --- | --- | --- |
| Normal | 1.5629 / 1.7913 / 1.4828 | 1.2721 / 1.4302 / 1.0000 |
| Heavy | 1.0520 / 1.0981 / 1.1765 | 1.0415 / 1.1845 / 1.2778 |
| Shifted-heavy | 1.0239 / 1.0226 / 1.1579 | 1.0126 / 1.0434 / 1.0000 |

All six comparisons reported zero candidate illegal placements, no more avoidable combo deadlines, and average survival / proxy score above the unchanged v1.7 baseline. P10 survival was at or above baseline in each. However **shifted-heavy holdout 1950 combo-break rate was 2.591 vs v1.7's 2.204 per 100 moves**, violating the no-worse combo-break gate. More fundamentally, no survival or proxy score ratio was near 10x; both exact gate verdicts were failure. Simulator score estimates are NOT real Block Blast scores. No claim of full improvement, promotion or independently verified 10x is justified.

## Benchmark measurement defect and repair after iteration 33

The previous simulator stopped every game at MAXMOVES=1400. A baseline mean of 162.133 moves on normal holdout 1947 leaves a theoretical upper bound of 1400/162.133 = 8.635x even for a perfect candidate. Therefore certain strict 10x average survival gates were mathematically unattainable under the old cap regardless of strategy. Research commit `e4c85de920f9d4a0c8f1b4d048130046591e92bd` raised the equal v1.7 and candidate cap to 4096 and added right-censoring counts. The strict runner now requires `capped_games=0` for both baseline and candidate across each of three generators in both holdouts (commit `b875588af416cd196243092911a442fc6a608c2a`). If any game reaches that cap without ending, proof fails closed until a larger cap is independently tested. This is a measurement correction, not evidence the AI suddenly improved. Exact post-change compile and holdout verification pending in iteration 34.

## Iteration 34 — proactive combo protection

Research-only commit `5300dc1c7086887bd1a698dd67b0ecee10b09731` gives a verified complete-tray combo-preserving route a moderately higher tolerance when a witnessed combo already has **one non-clearing move**. This tries to prevent future streak breaks *before* the last-chance rescue search, while retaining the dangerous-board hard-piece landing guard, exact legal moves and all strict gates. Extended unit tests (`9e64ff8218a395287f0dc9f84d1d70391740f5a6`).

[GitHub Actions run 43794363486](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43794363486), exact workflow source `c76981872661f8761bb70f1c34865c376c096655`, was **in progress** at initial log creation. It uses newly reserved provisional training epoch 1834 and separate holdout epochs 1951 and 1952. It includes the two-stage training selector and corrected 4096-move cap. The complete outcome and measurements must be inspected and appended after job completion; do not infer success.

## Ongoing constraints

The centralized research-only cross-role seed ledger `learning/tenx_used_epochs.txt` reserves every known used experiment epoch through training 1833 and holdouts 1950. Selection of 1834/1951/1952 was checked against all accessible research logs and recent Actions records, but outside history is not fully auditable. Never call global freshness independently certified. The exact 8x8 3-piece simulator uses approximated 28 shapes and simulated generators, not the proprietary Block Blast piece RNG or official score formula. No candidate was promoted. The saved checkpoint is still `16 120 120 30 60`, saved candidate `120 120 30 60`; main, deployment and iPhone runtime unchanged. The full 10x threshold has not been reached or independently replicated.

## Completed iteration 34 measurements — research-only proof of bounded non-regression, NOT 10x

Run [43794363486](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43794363486) completed with the strict performance-gate failure verdict on November 16 UTC. Source SHA `c76981872661f8761bb70f1c34865c376c096655`; preflight verified documented fresh train epoch 1834 and holdouts 1951/1952; exact-source compilation and 8x8/three-piece legality selftest **PASS**. One initial five-game/generator 21-candidate training epoch ran, but 0/21 variants passed initial combo-safety screening. The isolated trainer retained `120 120 30 60` (no checkpoint/weights promoted) and second-stage selection had zero finalists. This is an intentionally conservative failure of candidate training, not a failed build.

Holdout 1951 (30 paired synthetic games per generator):

| Generator | Mean survival ratio | Estimated-score ratio | p10 survival ratio | Combo breaks/100 v1.7 → candidate | Avoidable deadlines v1.7 → candidate | Illegal/capped games |
|---|---:|---:|---:|---|---|---|
| normal | 1.4080x | 1.5818x | 1.4737x | 2.067 → 1.482 | 0 → 0 | 0 / 0 |
| heavy | 1.3505x | 1.7289x | 1.1875x | 3.916 → 2.475 | 1 → 0 | 0 / 0 |
| shifted-heavy | 1.3146x | 1.6287x | 1.2308x | 2.033 → 1.547 | 0 → 0 | 0 / 0 |

Holdout 1952 (30 paired synthetic games per generator):

| Generator | Mean survival ratio | Estimated-score ratio | p10 survival ratio | Combo breaks/100 v1.7 → candidate | Avoidable deadlines v1.7 → candidate | Illegal/capped games |
|---|---:|---:|---:|---|---|---|
| normal | 1.0814x | 1.2052x | 1.0000x | 2.148 → 1.724 | 0 → 0 | 0 / 0 |
| heavy | 1.2191x | 1.5105x | 1.2500x | 3.298 → 2.269 | 0 → 0 | 0 / 0 |
| shifted-heavy | 1.3671x | 1.8063x | 1.2143x | 2.719 → 1.447 | 0 → 0 | 0 / 0 |

In all six comparisons the candidate's survival average and simulator-estimated score were greater than 1x, p10 was >=1x, combo breaks per 100 moves lower, zero reported illegal placements, no more avoidable combo deadlines, and **zero games reached the new 4096-move censoring cap**. As intended, the corrected benchmark no longer makes 10x mathematically impossible for the observed normal baselines (169.300 and 141.233 average moves), but the candidate's gains are still much lower than 10x. Both strict holdouts explicitly returned `TEN_X_VERDICT=NOT REACHED - NO DEPLOY`. Simulated proxy points are NOT real Block Blast scores.

Interpretation: this is a useful *paired-seed simulated incremental non-regression signal* for the combined research-policy changes with unchanged saved weights; it does not independently isolate the effect of proactive combo protection or confirm real-phone gains, statistical confidence, full 10x, or a deployable policy. The hourly research should use this evidence when designing further **long-horizon** strategies, not keep launching the same four-weight neighborhood unchanged.

The previously listed used seed epochs 1834, 1951 and 1952 are now reserved cross-role in `learning/tenx_used_epochs.txt` (commit `d0cbaac1196507c1f345c3049ec05c8c292c5ec0`). No new run was launched after these measurements to avoid excessive Actions minutes.

### Remaining gameplay limitations and next research work

1. Research candidate's per-generator gains remain roughly 1.08x–1.41x in mean survival and 1.21x–1.81x in score proxy; far from 10x. No valid weights passed training during iteration 34.
2. Current six hard-pair co-fit scenarios are an afterstate proxy, **not** actual probabilistic multi-tray planning. A future bounded experiment should evaluate next-tray distributions or adversarially difficult legal three-piece combinations with an explicit compute budget, rather than only more weight shifts.
3. Current training split (five initial plus eight separate training games per generator) can still overfit. More fresh independent gameplay traces and realistic generators would be needed for robust conclusions, but must remain separate from acceptance holdouts.
4. This lab does not address on-device screenshot recognition, orientation, drag release, touch calibration, actual Block Blast scoring, or proprietary piece generation. Those require a separately authorized iPhone/vision test and cannot be certified by synthetic C simulations.
5. Preserve the original v1.7 baseline and the user's separately active v3.4 phone/automatic-play build. Do not deploy a model based solely on these incremental offline metrics. Strict full 10x, zero-invalid, combo non-regression and independent replication gates remain mandatory for accepting the challenge.
