# Bazz1TV research notes — high-score habits for BlockBlastAI
**Research date:** 2026-11-13. **Status:** source-based research plan; *not* a claim that all livestream frames or full spoken transcripts were inspected.

## Primary creator sources (Bazz1TV's own YouTube descriptions)

1. **How to Get High Scores in Block Blast - Combos** (2025-11-07):
   https://www.youtube.com/watch?v=pJdy4piU1eI
   - Explicit description: **get a clear every 3 moves or less** to keep the combo going.
   - Explicit description: favor **single clears early** to build the combo; later bigger clears profit from a high combo.
   - This is advice about sustained scoring, **not** a claim that each move must clear a line.
2. **Block Blast Pro Tip - Block Placement** (2025-12-02):
   https://www.youtube.com/watch?v=MMGfYRSxyRA
   - Creator identifies **placement and block forcing** as a focus. The accessible description does *not* explain the mechanism. Do not invent detailed block-forcing steps until the actual video/transcript can be analyzed.
3. **Block Blast World Record Part 1** (published 2026-03-20):
   https://www.youtube.com/watch?v=aMUDWKVlm3I
   - Describes a run documented over 30+ days and 600M+ points at the time, with archived footage starting at zero.
   - Explicitly describes his **slow and patient playstyle**. He says that impatience can end runs early.
4. **Block Blast World Record Part 29** (2026-03-16):
   https://www.youtube.com/watch?v=460xUVoGAg4
   - Notes 600M+ during record attempt and tips given on streams; this description alone does not prove that the 1B target was achieved.

A third-party report describes a Bazz1TV livestream run exceeding one billion:
https://views.terabox.com/hub/what-is-the-highest-block-blast-world-record-score-ever-recorded
No official Hungry Studio global-record leaderboard is publicly verified, and the creator descriptions inspected are insufficient on their own to certify an official **world record** or determine exact game modifiers.

## Separate supporting context — NOT automatically attributed to Bazz

- Independent community guide: https://www.reddit.com/r/blockblast/comments/1hl87zk/block_blast_ultimate_high_score_guide/
  - Plan all visible 3 pieces, clear a row/column within the combo window, protect space for 3x3 squares, large Ls, 1x5 straights, and avoid messy gaps.
  - Keep useful partial near-complete rows/columns as setup for the next tray, but sacrifice a streak if necessary to survive.
  - Suggests ~1/4 board occupation as a *heuristic to test*, not a hard coded optimal constant.
- https://mobi.gg/en/tips/block-blast-highest-score/
  - Similarly emphasizes combo planning and right-sized occupancy.
- These sources support potential experiments, but neither demonstrates a fixed 25% target is always optimal.

## Current AI design discrepancy

- `src/BlockBlastSolver.c` enumerates 3 visible pieces; `comboRun` and `comboBonus` start fresh **each call**. The solver's combo score only rewards consecutive clears *within the current call*, not the actual streak or time since the last line across trays.
- When a player has cleared a line recently, the current recommendation can waste the rest of the three-move combo window and break a very high combo.
- `src/BlockBlastOverlay.m` calls the solver based on the scanned **board and remaining pieces**. No persistent combo streak or last-clearing-move age is passed into `BLSolve`.
- The current baseline is primarily **survival moves**, not Block Blast's real point score. A 100M score requires authentic scoring rules and long streak validation, not just survival counts.

## Implementation experiments (future — not yet in the compiled build)

1. **Streak-aware state tracker:** derive verified `linesCleared` from confirmed board + tray transitions. Track count of nonclear placements since the previous clear and number of consecutive clears as a separate stat. Reset or pause on uncertain scans, restarts or manual input; *never invent combo state*.
2. **Combo window evaluator:** for each legal candidate sequence, simulate the clear/non-clear bit pattern in order, and penalize only windows that exhaust the actual three-move allowance. Use the creator's first-party description as a behavioral hypothesis to verify against the installed game version.
3. **Early single clears vs multi-clears:** test scoring profiles that prioritize sustaining the streak while board occupancy is safe. At dangerous occupancy, **survival outranks preserving combo**.
4. **Next-tray line readiness:** reward multiple distinct legal routes to finish a row/column next tray while reserving 3x3, 3x3-L and five-long capacity. This is a predictive heuristic, not insight into the game's hidden RNG.
5. **Video-based validation:** a sampled gameplay recording with visible board/tray/score/combos can be annotated move-by-move to compare decisions. YouTube video descriptions provide *strategy evidence* but **not a dataset of placements** without inspecting actual frames. Record version, modifiers and real scoring.

## Release gate
- Keep the user's known working **v1.1 BLINK** untouched.
- On fresh heldout simulated trays, report combo streak length, survival percentiles, score (under explicitly described scoring assumptions), and move legality for both candidate and v1.1.
- On the user's iPhone, verify actual move-to-move streak changes and clear counts without false positives.
- Do not label the new dylib **100M-ready** merely because a creator has documented a high run or because a synthetic benchmark improves.
