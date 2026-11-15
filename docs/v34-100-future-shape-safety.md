# v3.4 — 100 future-shape probes, guarded Smart Survival

This release experiments with **100 distinct, normalized, eight-neighbor-connected
hypothetical future-piece shapes** and a stronger safety gate. It does **not**
claim 100 independently proven score improvements, 10× survival, or new vision
training. The continued 10× research still lives on a separate branch and
is not included until its improvement criteria pass.

## What it changes

- `BLFuture100.c` contains 100 unique, bounded (up to 5×5) future-piece
  footprint probes, including small squares, 3×3 squares, horizontal and
  vertical bars, diagonal pieces, corners, zigzags, and assorted connected
  polyominoes.
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
