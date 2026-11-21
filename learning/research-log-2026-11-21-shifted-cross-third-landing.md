# Research-only shifted-cross third-landing admission trial — 2026-11-21

Branch: feature/v2.2-10x-research-gate. Source commit f108f4d1; regression commit 422e56be. No main, live v3.4, deployment, iPhone build, baseline or validated checkpoint changes.

## New bounded hypothesis
On crowded completed-tray boards that already meet exact two-landing minimum for each of three irregular families and have disjoint legal 3x3-square landings, add 75 admission-quality points if shiftedCross has a third exact legal landing. This distinguishes fragile two-landing shifted-cross afterstates from slightly more redundant three-landing states without increasing the existing 20-finalist pool, changing search budget, observing hidden next-tray RNG, or altering v1.7. This is an untested candidate, not an improvement claim.

## Audit and blocker
Fetched exact .github/workflows/v22-tenx.yml, learning/run_tenx_reproducible.sh, learning/tenx_used_epochs.txt, the prior maximin research log, current source, regression guard, and provisional frontier. The workflow still uses reserved five-role epochs 794/795/796/797/798. No run was triggered: repository-wide Actions concurrency cannot be authoritatively established through available connector actions. Accessible ledger/history may omit external/local runs. No new seed IDs were consumed, no CI compilation or simulator holdouts completed, and no strict 10x pass is established. Do not reuse this workflow snapshot.

## Required next experiment
Verify repository-wide Actions non-overlap, audit five genuinely fresh epochs across all roles and historical raw PRNG starts, update only the research workflow YAML to trigger, pass source/seed preflight, and run two independent fresh paired 30-game/generator normal/heavy/shifted-heavy holdouts against unchanged v1.7. Preserve every failed metric and artifact. Require >=10x mean survival, synthetic estimated score and p10 in each generator, zero illegal/censored, no worse combo breaks or avoidable deadlines, then independent replication. Synthetic estimated score is not actual game score.
