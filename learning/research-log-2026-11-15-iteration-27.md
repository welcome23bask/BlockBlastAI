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
