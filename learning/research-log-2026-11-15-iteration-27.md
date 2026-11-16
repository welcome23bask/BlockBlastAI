# Iteration 27 — resolve self-blocking seed configuration and test shifted-heavy shape coverage

Date: 2026-11-15 (America/Chicago). Branch: `feature/v2.2-10x-research-gate`; no production or phone build.

## Confirmed root causes

1. Latest configured workflow still used training epoch 1822 and holdouts 1915/1916 although the current reproducible runner explicitly rejects all three in its documented used/planned-epoch restrictions. A new job would fail before compiling or training. Earlier run 43760908521 failed compiling due to `sameSafety` being accidentally commented out; that source error was repaired in iteration 25 but had not been rebuilt.
2. Training over 21 four-weight variants has repeatedly produced zero combo-safe candidates. In the last completed genuine training run 43756868749, the incumbent candidate showed normal average-survival ratios 2.5786x and 1.7708x across two holdouts, but shifted-heavy only 1.0115x and 1.0734x. One shifted-heavy score ratio was below baseline; combo outcomes were worse in some scenarios. The 10x gate failed.
3. The synthetic hard generator frequently supplies irregular shapes 21–27; research-policy board scoring only directly represented square, bar, and rectangle footprints. It lacked coverage of those irregular landing zones, a plausible source of shifted-heavy fragility. This is a hypothesis, not a proven causal explanation.
4. Existing training logs reported only aggregate candidate-safety rejection counts, obscuring which generator's survival, combo breaks, or avoidable deadlines drove incumbent rejection.

## Targeted research-only changes

- `learning/BLTrainableStrategy.c`: add a bounded afterstate feature for the exact masks of seven irregular hard-generator shapes 21–27; reward shape-family coverage and capped legal placement counts, and penalize near-complete loss of coverage. The independent baseline stays `src/BLDualStrategy.c` and is unchanged. Source commit: `c059bd12553cdac2a4e93680b10c01f00d681a1a`.
- `learning/train_tenx.c`: print paired per-generator incumbent diagnostics before the usual strict combo and legality checks; does not modify gates. Commit `56acc9f90b602d0246eae6b652b1b159224e588b`.
- `learning/run_tenx_reproducible.sh`: optional `TENX_PREFLIGHT_ONLY=1` exits after seed-policy checks, before compiling. This checks known reservations but does not certify unrecorded historical runs. Commit `537c3e84def98e8e6ccb0e17f69b9a6eedb9af64`.
- `.github/workflows/v22-tenx.yml`: added no-compile preflight; changed provisional training epoch to 1827 and holdouts to 1937/1938; one bounded 21-candidate training epoch, three paired training games per generator, 30 games per generator in each of two holdouts, 12-minute CI timeout, no promotion. Workflow commit `a2a0df28744268a570ad488bedbec73c167b4b9b`.

## Seed and concurrency review

Scanned all research logs visible in the connected research-branch directory (iterations 2–15 and 17–26, the initial log, and provenance audit) for the exact proposed epoch IDs 1827, 1937, 1938; no matches. Reviewed the accessible branch-specific workflow run collection; its recent jobs were completed, including the last failed iteration 22 run. Since external historical executions or missing artifacts cannot be completely excluded, seed non-use is provisional, not independently certified. Three generator namespaces and different epochs were used; exact legality is still evaluated inside the synthetic simulator.

## Execution status and next diagnosis

Triggered GitHub Actions run [43787752985](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43787752985) on the research branch. The run was confirmed in progress while writing this log; outcome and compilation cannot yet be asserted. Examine preflight, compilation, `TRAIN_INCUMBENT` output, two strict holdout verdicts, and per-generator metrics. If any fail, preserve evidence and diagnose the precise cause before the next research-only change.

No improvement or 10x threshold is verified. Simulated proxy scores are not actual Block Blast scores. Saved checkpoint `16 120 120 30 60`, main, deployment, and iPhone dylib remain unchanged.

## Completed experiment results (updated after GitHub Actions finished)

Run [43787752985](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43787752985) completed with an expected failing performance verdict; source compilation, seed preflight, one training epoch, both independent provisional holdouts and evidence preservation all executed. Preflight printed `SEED_PREFLIGHT=OK training=1827 holdouts=1937,1938`. This means the documented-seed validation worked; it does not establish full external seed provenance.

With the irregular-shape feature, training found **7 combo-safe feasible candidates of 21** (14 rejected for combo, zero for illegal), compared with **0 of 21** in prior iteration 21. The provisional selected weights were `120 180 30 0`; no persistent weights were promoted.

- **Holdout 1937:** normal survival/estimated-score/p10 ratios `1.2273/1.2416/1.1429`, heavy `0.8472/0.6919/0.8696`, shifted-heavy `1.2737/1.5247/1.0000`. Candidate normal combo-break rate 2.570 vs baseline 2.140 per 100; avoidable deadlines 3 vs 0. Candidate heavy combo-break rate 3.878 vs 2.931.
- **Holdout 1938:** normal `1.3882/1.4612/1.0000`, heavy `0.9004/0.8402/1.0000`, shifted-heavy `0.9336/0.8576/1.0625`. Candidate normal combo-break rate 2.479 vs baseline 2.362 per 100. Candidate heavy avoidable deadlines 1 vs 0, shifted-heavy 2 vs 0.
- All six candidate/baseline scenario measurements reported zero illegal placements. Both holdouts explicitly reported `TEN_X_VERDICT=NOT REACHED - NO DEPLOY`.

**Newly confirmed training defect:** incumbent paired training metrics normal survival 3.3404x, proxy 3.9781x, p10 2.4483x but heavy survival 0.6987x, proxy 0.5933x, p10 0.7419x. `train_tenx.c` was summing scenario fitness, so a spectacular normal-generator gain could dominate substantial heavy losses. This was incompatible with the user's per-generator 10x gate and explains choosing unstable strategies.

**Next targeted fix (source-only; NOT yet tested):** change candidate selection from a sum to the **minimum per-generator fitness**, optimizing weakest-generator performance while retaining existing zero-illegal/strict combo safety checks. Commit `9d72435d9e80e84e39238f66c32a9cf6c457256b`. Reserve used epochs 1827, 1937 and 1938 in both train and holdout roles to prevent accidental reuse: commit `b7a3a4a5e3d3a267778adf69d601219e3ffe65eb`. This makes the currently configured workflow intentionally fail if re-run with already used epochs. The next bounded iteration MUST update the workflow to newly audited unused seed epochs, check no active run, and independently benchmark again. Do NOT launch duplicate experiments on the old IDs or falsely claim that worst-case fitness has been measured.

The 10x result has not been reached. Simulator-estimated scores are not actual game scores. v1.7 baseline, persistent checkpoint/candidate, main, iPhone dylib and deployment remain unchanged.
