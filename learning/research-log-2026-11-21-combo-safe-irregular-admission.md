# 2026-11-21 — combo-safe crowded finalist admission (research only)

Branch: `feature/v2.2-10x-research-gate`. Source commit `449bfea60335a21d4d2cb4452a35c538eee74b20`; guard commit `4ae56eae4563443b17e84a9ba8bd53b8e18fe3b7`.

## Evidence and failure hypothesis
Historical independently paired iteration-44 evidence (Actions 43807467128) failed all strict 10x survival/score gates, particularly heavy-piece survival (~1.02x and ~1.07x). Later run 44837049501 also failed strict holdout gates, including heavy p10 0.85x on 797 and normal p10 0.9737x on 798. Synthetic estimated score is not the actual app score. These reserved epochs cannot be rerun.

## New bounded structural hypothesis
The previous crowded-board finalist *admission* reward granted its base 90x weakest irregular-family landing count even to a path with `broken==1`; only the additional third-landing bonus required an unbroken combo. This could admit a combo-breaking finalist at the expense of a legal combo-preserving finalist, worsening deadline or combo-tail behavior. The new candidate requires `path->broken==0` for the entire crowded irregular admission reward, not just the third-landing increment. This does not change exact current-tray legality, v1.7 baseline, search budget, 20-finalist capacity, hidden RNG access, or checkpoint. Updated the preprocessor/compilation regression assertions to require the new condition.

## Execution and limitations
Fetched the exact `.github/workflows/v22-tenx.yml`, `learning/run_tenx_reproducible.sh`, and `learning/tenx_used_epochs.txt` on the research branch. The workflow still embeds reserved 794/795/796/797/798, which fail five-role preflight. The available connector exposes job details by known run ID but no complete repository-wide run listing; therefore overlapping Actions runs cannot be excluded. No workflow trigger, seed consumption, training, simulation, compiler execution, or independent replication occurred in this iteration. This is an untested research hypothesis, **not** a measured gameplay gain.

Next safe step: verify no concurrent Actions research, audit five fresh distinct 17..2000 role epochs against ledger and accessible history, check raw seed aliases, pass preflight, trigger bounded paired normal/heavy/shifted-heavy simulations against unchanged v1.7, retain all failed evidence, and reserve used epochs after immutable workflow snapshot. Never relax 10x per-generator mean/score/p10, legality, combo/deadline, censor or independent replication gates. No release or checkpoint promotion.
