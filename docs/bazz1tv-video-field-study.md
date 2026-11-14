# Bazz1TV video field study — observed gameplay and AI replay audit

**Source:** User-provided 4x screen recording of Bazz1TV's *Block Blast Pro Gameplay From 0 to 500k in 27min*, public URL https://www.youtube.com/watch?v=esr8UN_skKw. This document includes analysis, not an upload of the recorded video or captured frames.

## Source, data quality and uncertainty

- Source: 451.3-second, 1108x512, 30fps H.264 screen recording (13,539 source video frames), around 4x playback of ~30 minutes original.
- 10fps board and tray samples and ~15fps board-only pass were classified in a fixed 8x8 grid.
- 361 **candidate** tray-piece removal events inferred; 128 **exact legal board-transition matches**, 47 near matches (up to two differing cells), 122 low-confidence and 64 unmatched.
- Exact transition means a legal placement plus simultaneous row/column clear matches the observed next board at sampled frames; already-full prestate rows/columns are excluded, to avoid mistaking ongoing clear animation for a new move.
- This is automated analysis, not 13,539 manually audited frames. The 4x speed, dragging previews, particles and score animations can obscure actions. Low-confidence data must never be used as ground truth.
- No full speech transcript was reconstructed.

## Visually read score results

| Uploaded video timestamp | Score |
|---|---:|
| 00:15 | 140 |
| 01:00 | 23,898 |
| 03:00 | 124,404 |
| 04:00 | 203,537 |
| 06:15 | 378,167 |
| 07:15 | 500,067 |
| ~07:29 | **503,429** |

- Gold crown/best visibly displays **1,012,123,834**. This is a shown personal best, **not proof of officially verified global record status**.
- The 06:15-to-07:15 interval gains 121,900 points vs 23,898 in the first minute (about 5.1x). Long combos plausibly matter, but the actual engine score multiplier cannot be inferred from this alone.

## Stronger empirical shape-access pattern

Among 128 exact matched before/after placements:
- Median **21/64** filled cells after placement (P10 14; P90 28).
- 121/128 (94.5%) still have at least one legal 3x3-square landing zone.
- 128/128 (100%) have at least one legal 1x5 horizontal placement, **and** a 5x1 vertical placement.
- This is a selected high-confidence subset; **not** evidence that exactly 21 occupied cells is an optimal target or that future pieces are known.

## Three particularly clear legal transition examples

1. **02:10.6**, middle tray 3x3 at row 3, col 2, clears **three columns**; 23 occupied -> 8. This creates -15 net occupied because 9 added squares complete 24 filled column cells. Demonstrates deliberate 3-column setup and preservation of 3x3 landing zone.
2. **02:41.6**, right tray 3x3 at row 1, col 1, clears **three columns**; 27 -> 12. Repeated prepared corridor strategy.
3. **05:07.3**, middle tray zigzag `.##/##.` at row 7, col 5, clears **two lines**; 30 -> 18. A small four-square placement can release two prepared lines.

Initially tempting high-clear examples at 03:08.9, 04:24.1, and 06:47.9 were excluded from the exact set after identifying already-complete lines in their **before** screenshot (clear animation in progress). Avoid attributing those scores to the next dragged piece.

## AI replay comparison

In 114 exact states with enough observed tray information:
- v1.1 BLINK first move = observed move: **20/114 (17.5%)**.
- Experimental v1.3 first move = observed move: **20/114 (17.5%)**.
- v1.1 BLINK first move = experimental v1.3 first move: **103/114 (90.4%)**.

This is **move disagreement, not an optimality verdict**: multiple moves can be legal and valuable, and Bazz's long-term rationale and the next unseen pieces cannot be established from one snapshot. The v1.3 compiled checkpoint in the offline analysis used `BL_FUTURE_CLEAR_REWARD=10` and is not necessarily the current branch head.

Example 00:31.0: verified player used 4-cell `#./#./##` at R4/C5, left 21 occupied; v1.1 and v1.3 suggested a vertical 3-block at R6/C6, immediately completing a line and leaving 12 occupied. It is not possible to prove which decision leads to better later score from this frame alone.

## Implementation priorities derived from actual video

1. Persist a **verified combo / last-clear move counter across 3-piece trays** instead of resetting `comboRun` on every `BLSolve`. Need confirmed actual game rules and board transition evidence; don't invent combo state when screen scans are uncertain.
2. Add **3-column 3x3 setup detection** and score planning, but only when it does not compromise survival. The video confirms two uses at 02:10.6 and 02:41.6.
3. Preserve at least one legal landing zone for 3x3 and both orientations of long bars as a **risk-sensitive** prior, not as an inviolable constraint.
4. Evaluate **score and combo survival**, plus P10/median game survival, not just mean simulated moves. Match real score/line clear rules before making claims.
5. Use exact video snapshot CSVs as **challenge cases** where the AI differs from Bazz; don't train blindly to imitate his placements or treat medium/low reconstruction as ground truth.
6. Keep the user's **v1.1 BLINK scanner** untouched. v1.3 remains experimental, not verified for 100M.

## Artifact availability and permissions

A 10-page PDF report, offline HTML move viewer, and confidence-labeled datasets were produced for the current user in their conversation's downloadable artifacts; this GitHub file records only findings. Do **not** copy the user-uploaded screen recording into this repo without permission.

**Limitation:** No proof of 100-million readiness, no verified complete transcript, and no claim of an official world record.
