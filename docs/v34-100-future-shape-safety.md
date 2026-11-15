# v3.4 — 100 future-shape probes, guarded Smart Survival

This release experiments with **100 distinct, normalized, eight-neighbor-connected
hypothetical future-piece shapes** and a stronger safety gate. It does **not**
claim 100 independently proven score improvements, 10× survival, or new vision
training. The continued 10× research still lives on a separate branch and
is not included until its improvement criteria pass.

## What it changes

- `BLFuture100.c` contains 100 unique, bounded (up to 5×5) future-piece
  footprint probes. The first 35 represent all distinct tray shapes proposed
  by the scanner in 196 user-provided screenshot examples (404 labeled tray
  observations); those scanner labels are **unreviewed**. The other 65 are
  diverse hypothetical legal-footprint probes, including unusual 8-neighbor
  connected shapes. This is NOT a claim these 100 shapes are all real pieces
  or that the observed frequencies reflect hidden game piece probabilities.
- For each verified solution, the lookahead calculates how many of these
  shapes retain >=1 legal landing and the capped number of placements.
- This is a **hypothetical mobility probe**, not an assertion that each
  shape appears in the actual Block Blast version or with equal probability.
- During Smart Survival **only**, the program compares the existing best
  safe plan against two existing solver alternatives. It may take a different
  plan **only** when:
    * the board and full tray sequence remain legal;
    * all currently visible pieces can still be placed;
    * no validated current-line clear count is lost;
    * a combo-preserving clear is not delayed or lost;
    * no existing 3×3, 5×1, 1×5, 2×3 or 3×2 landing type disappears;
    * future-shape coverage improves by >=3/100 shapes;
    * capped mobility improves by >=12 placements;
    * the improved plan still passes `BLGuideValidateSolution`.
- An actual clear plan found by the 3-try solver continues to override
  this heuristic, maintaining verified combo and deadline behavior.
- The original scanner, black-skin support, learned provisional color
  verifier, five-fresh-frame no-clear checking, training export, and
  on-game highlights are preserved.

## Tests, not guarantees

CI tests prove 100 distinct connected shapes and run 100 reproducible
synthetic board/tray scenarios. They check legality, combo deadlines,
cleared lines, and critical future-piece access. These are **not**
simulated full-game score benchmarks and do not prove higher real scores.
The next necessary step is independent held-out full-game benchmark
testing against unchanged v3.3 and real iPhone playback.

The 10× research automation remains separate; no unverified research weights
were silently promoted to the production iPhone library.

## Captured dataset relevance

- User ZIP: 196 screenshots, 404 provisional nonempty tray observations,
  35 distinct predicted shapes. No raw screenshots are committed to GitHub.
- All 35 observed patterns are protected by a dedicated regression in
  `tests/test_v34_future100.c`.
- The training label accuracy remains unverified; this is a heuristic future
  accessibility check, not trained strategy weights or true 10× research output.

## v3.4 enhanced update: additional safeguards, still named v3.4

The original v3.4 code, original v3.3 trained verifier, Smart Survival,
three-try clear solver, five-fresh-scan no-clear confirmation, vision dataset
capture/export and overlay UI are all preserved.

- For each candidate, future mobility now distinguishes the 35 **observed
  but unreviewed** shapes from the 65 speculative footprint probes. A route
  cannot reduce observed-shape coverage or capped mobility.
- A four-neighbor flood fill counts tiny stranded empty cells and empty
  regions; candidate plans cannot create more isolated 1–2-cell pockets.
- **No speculative line-tied switches**: A full-tray candidate must clear
  strictly more verified lines, increase future shape access, and preserve
  current combo deadlines and five critical landing types. Earlier
  experiments sometimes scored lower on shifted-heavy synthetic games, so
  the equal-clear speculative switch was removed.
- **Full-tray rescue**: if the Smart Survival route is partial or invalid,
  but an alternate solver objective has a complete, valid route for all
  currently visible pieces, it may be used if it does not sacrifice a
  verified current clear, combo deadline or previously found clear count.
  Unsafe or incomplete alternatives are discarded.
- **Mathematical no-clear precheck**: before exploring up to 1.5 million
  search states, the planner computes an optimistic contribution of each
  visible piece to every row/column, respecting how many combo tries remain.
  If even the optimistic bound cannot fill any line, there is provably no
  first clear with those visible pieces; expensive DFS is skipped.
  The board still undergoes 5 fresh image confirmations before a negative
  answer is displayed.
- The floating UI identifies itself as **BlockBlastAI v3.4**, rather than
  retaining outdated v2.9 branding in the title/status.
- A reproducible 30-game, 3-generator, paired full-game smoke benchmark
  compares *the same* generated future piece trays under v3.3's baseline
  and v3.4's guarded strategy, checking score proxy, moves survived,
  p10 survival, line clears, invalid placements, combo breaks, and
  avoidable combo deadlines. With conservative v3.4 gating, results
  **matched baseline** in all 30 games. This demonstrates no regression in
  this small synthetic test, **not a proven increase in real scores**.
- Four independent seed batches comprising 400 synthetic board/tray
  scenarios and targeted unit fixtures check legal moves, trapped pockets,
  extra lines, real piece access, 1/2-try combo rescues, and 100 distinct
  future piece shapes.

The separate 10× strategy learning research is not merged because its
independently replicated threshold has not been met. The current model
does not learn new strategy weights while the user plays.
