# Research iteration 14 — 2026-11-15 (source and execution audit)

Research branch: `feature/v2.2-10x-research-gate`. Source HEAD before this note: `c70e130a1c4c948b6f17b1afa32c1108542f5858`.

Read current `learning/tenx_checkpoint.txt` = `16 120 120 30 60`, `learning/tenx_candidate_weights.txt` = `120 120 30 60`, current reproducibility runner, workflow and existing independent benchmark. The most recent branch Actions run remains [43639191903](https://github.com/welcome23bask/BlockBlastAI/actions/runs/43639191903), completed successfully on 2026-11-14 20:46 UTC at old SHA `9c4d80bda51e6b86c4ff66d20d71e47bb97541d4`. It checks the existence of a verdict but not its success and is therefore not evidence for the 10x target.

Additional concrete evidence: `learning/benchmarks/whole_tray_strict_heldout.txt` reports 40 paired games per generator, with identical baseline and learned survival moves in normal (126.050), stress/heavy (29.975), and shifted-heavy (25.000); the normal learned proxy score and combo-break rate are slightly worse. These historical measurements do not satisfy any 10x claim and are synthetic estimates, not actual Block Blast scores.

The corrected `learning/run_tenx_reproducible.sh` requires a clean research checkout, one bounded training epoch, two holdouts and successful verdicts, with evidence retained in a temporary directory. However, its epoch exclusion list is incomplete relative to all historical runs and it does not independently prove the legality and combo gates beyond the simulator's own verdict. It cannot be used to declare verification without external repetition and an audited seed ledger.

Execution availability: GitHub connector read access works, but no general Actions workflow dispatch action is exposed. The container has git and a C compiler but cannot resolve `github.com` (checked this run), and has no repository checkout. Thus exact-source compilation, fresh training, new holdouts and independent replication were not possible. No Actions minutes consumed.

No checkpoint, weights, main branch, deployment or iPhone dylib changed. Next: obtain exact-source checkout, audit used seed epochs, run a bounded training iteration with fresh paired seeds across normal/heavy/shifted-heavy and correct 8x8 three-piece legality, preserve raw evidence, and independently repeat the full survival, proxy score, p10, invalid-placement, combo-break and avoidable-deadline gates. **10x is not verified.**
