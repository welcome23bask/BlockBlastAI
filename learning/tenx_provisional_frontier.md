# Provisional BlockBlastAI research frontier — NOT validated/released

Updated 2026-11-16 UTC. Branch `feature/v2.2-10x-research-gate` only. Neither a release checkpoint nor a trained iPhone model. Do not merge/deploy/build iPhone, alter main or overwrite `learning/tenx_checkpoint.txt` / `learning/tenx_candidate_weights.txt`.

## Two noteworthy experiments (not fully 10×)

**Provisional non-regression leader, iteration 44:** [Actions 43807467128](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43807467128), source/workflow revision `fb9b648958cc3671b0a55fdd823af7f236c838cf`. Key idea: exact future single-piece clearing access, plus a bounded penalty when empty cells are split among multiple disconnected four-neighbor islands on crowded candidate afterstates. Both fresh holdouts 1973/1974 used 30 paired games per normal/heavy/shifted-heavy generator and separately compiled unmodified v1.7 baseline. Mean survival normal 3.2370x/2.0571x, heavy **1.0164x/1.0665x**, shifted-heavy 1.2894x/1.2402x. Synthetic estimated score normal 3.9292x/2.4231x, heavy **1.0239x/1.1735x**, shifted-heavy 1.6831x/1.6471x. All six p10 ratios ≥1; all six combo-break rates lower, zero additional deadline misses, illegal moves or censored games. **Still fails strict 10× gates in every generator.** The heavy 1.0164x margin is thin and should not be represented as generalization proof.

**High normal mean but tail failure, iteration 42:** [Actions 43806245696](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43806245696), source/workflow revision `7f10addc5b64eb3b58d62de57b90ec167301f472`. Future single-piece exact clearing access without fragmented-pocket penalty. Normal survival 2.0630x/2.2664x; heavy 1.3939x/1.2118x; shifted-heavy 1.2319x/1.2032x. But shifted-heavy second holdout p10 16 vs v1.7 17 (0.9412x), so reject under no-regression gate.

Older iteration 34 had non-worse p10 and improved combo breaks but far smaller mean survival; follow `learning/research-log-2026-11-15-iterations-33-34.md` and all later per-iteration logs for provenance. Every failed experiment remains documented and should not be repeated with identical settings/seeds.

## Constraints and next genuinely different approach

- The **cross-role ledger** `learning/tenx_used_epochs.txt` excludes all documented train/holdout epoch IDs through iteration 44. Accessible history may omit local/external runs. Never reuse a reserved epoch, even one whose build failed.
- Future tests must use exactly two independent fresh 30-game/generator paired holdouts after training-only data and stage-two model selection. Strict all-generator ≥10× mean survival, estimated score, p10 survival; zero illegal moves and capped games; no worse combo breaks/deadlines. Estimated score is NOT actual app points.
- The biggest bottleneck now is **heavy-piece mean survival and modeled score**, not normal mean survival. A new calibrated short-horizon death-hazard/value model with independent training-only seeds is specified in `learning/tenx-next-learned-hazard-protocol.md`. Do not endlessly retune four coefficients or reactivate independently falsified fixed-tray rollouts / baseline shields.
- Seed methodology caveat: `seed_for()` in `learning/train_tenx.c` has cross-scenario 20/40-epoch raw RNG aliases; see the hazard protocol. Future experiments should domain-separate fresh seed streams before independently validating improvements, without rewriting historical evidence.
- Do not notify user of 10× success until full strict replicated threshold passes; research logs alone record failed/partial results. Do not change live v3.4 or validated checkpoint.
