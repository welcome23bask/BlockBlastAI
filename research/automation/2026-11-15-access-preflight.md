# 2026-11-15 bounded research preflight

Status: BLOCKED (infrastructure), not a completed training epoch or benchmark.

Observed: research branch exists. GitHub connector can fetch README on branch and recent commits; attempted `research/README.md` and `.github/workflows/research.yml` returned 404. Container `git ls-remote` failed because github.com DNS resolution is unavailable. The connector's available workflow operations expose job/artifact inspection by known run ID but no general run listing or workflow dispatch. Consequently no safe check for overlapping Actions jobs or audited unused seeds could be completed. No training or score claims.

Preventive gate for next bounded iteration: discover actual research file/workflow paths and latest checkpoint, identify latest run IDs and active jobs, refuse dispatch when overlap status is unknown, audit train seeds against all prior training manifests, freeze v1.7 SMART SURVIVAL reference, and reserve independent holdouts across normal/heavy/shifted-heavy generators. Fail closed if any prerequisite is unknown.

Next gameplay hypothesis (untested): explicitly penalizing low-percentile trapped-board states and impending avoidable combo deadlines may improve shifted-heavy survival without sacrificing combo continuity. Change search/evaluation only after recovering simulator code; compare against unchanged v1.7 on fresh paired holdouts and replicate apparent passes independently. Require 10x average survival and estimated score per generator, 10x p10 survival, zero invalid placements, no worse combo breaks/move and no more avoidable combo deadlines. Simulator estimated score is not real-game score.

No main changes, promotion, merge, deploy, dylib build or Actions minutes consumed.
