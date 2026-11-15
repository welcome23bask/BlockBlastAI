# BlockBlastAI 10x research — iteration 20 (2026-11-15)

Research branch only. No checkpoint promotion, production change, deployment, merge or dylib.

## Diagnosed blocker

Iteration 19 GitHub Actions [run 43743434708](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43743434708) failed **before training**, exit code 2: `Previously used/planned epoch: 1909`. The research runner's holdout exclusion list included 1909 and 1910 even though the workflow attempted to use those holdouts; it also excluded training epoch 1819. Thus iteration 19 did not produce a trained candidate or any performance metrics. The earlier iteration 18 result remains the most recent completed simulation benchmark. The persisted checkpoint is `16 120 120 30 60`, candidate file `120 120 30 60`.

## Bounded repair and next experiment

Updated research workflow at commit `6165d1820c4564bb9df9dcf1aca9b243c963eb82` to use provisional training seed epoch **1820** and distinct provisional holdouts **1911** and **1912**, avoiding the explicit exclusions. Retained the 21-trial, three-games-per-generator bounded search, two 30-game-per-generator holdouts, strict performance gates, evidence artifact, and 12-minute timeout. This triggers [run 43747877275](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43747877275), observed **in progress**. No result is yet verified.

**Limitations:** The historical seed ledger is incomplete, so provisional epochs are not certified never used; no independent verification or 10x claim is possible. Simulator scores are estimates, not actual Block Blast scores. Await actual run evidence before changing the research hypothesis or attempting another run.
