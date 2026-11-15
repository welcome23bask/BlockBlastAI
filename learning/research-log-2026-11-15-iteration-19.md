# BlockBlastAI 10x research — iteration 19 (2026-11-15)

Research-only branch: `feature/v2.2-10x-research-gate`. No main, deployment, checkpoint promotion, or dylib changes.

## Iteration 18 observed evidence

GitHub Actions [run 43742965054](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43742965054), source revision `83f5316e0946511f4710658830e05b4aa06009fa`, completed the bounded training and both holdouts. The workflow failed intentionally on the performance gate. Training epoch 1818 evaluated 7 candidates with 3 paired games per generator, found **0/7 combo-safe feasible candidates** (7 combo rejections, 0 illegal rejections), and retained weights `120 120 30 60` provisionally. Persisted checkpoint remains `16 120 120 30 60`.

Holdout 1907 (30 games/generator): normal survival 1.8525x, estimated score 2.0252x, p10 1.0312x; heavy survival 1.1873x, score 1.3338x, p10 1.0000x; shifted-heavy survival 1.1079x, score 1.2476x, p10 0.8750x. Normal/heavy candidate avoidable misses 3/2 versus baseline 1/0. Zero illegal placements in all scenarios.

Holdout 1908: normal survival 0.8616x, estimated score 0.8571x, p10 1.3750x; heavy survival 1.0948x, score 1.1656x, p10 1.1765x; shifted-heavy survival 1.0181x, score 1.0177x, p10 0.8235x. Normal/shifted candidate avoidable misses 2/1 versus baseline 0/0. Zero illegal placements in all scenarios. Combo breaks per 100 moves were lower for candidate across these particular holdouts, but avoidable deadlines failed. All numbers are simulator estimates, not real game scores.

## Iteration 19 hypothesis and bounded action

The feasibility-first search rejected every candidate in iteration 18. Expand the mutation neighborhood from 7 to 21 candidate trials in one epoch, preserving the exact strict combo safety and legality constraints, same 3 paired training games per generator. This changes search breadth without loosening safety. Provisional training seed epoch 1819; independent provisional holdouts 1909 and 1910, 30 games/generator each. Workflow still has a 12-minute timeout and uploads failure evidence. Changes: trainer commit `3a47c68f8549dfad7b21418a6f4d23d636006903`, seed exclusions commit `b7763df4237af565b85a62cd415cddb88c20c92a`, workflow-triggering commit `21cc2c5b55634e8357e1ae4e421bb79a194e63fc`.

**Limitations:** Historical seed ledger has not been fully audited, so 1819/1909/1910 are only provisionally unused; no 10x verification can be claimed. The runner does not promote candidates, and no independent fresh-seed repeat is justified until all gates pass. Check [workflow runs](https://github.com/welcome23bask/BlockBlastAI/actions/workflows/v22-tenx.yml) for the new run's final status and evidence.
