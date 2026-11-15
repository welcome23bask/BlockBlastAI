# BlockBlastAI v3.4 — User ZIP visual corrections and occupied-cell automation guard

## Evidence inspected directly

User-uploaded `BlockBlastAI-Vision-B5C60128-4E08-4C21-9F42-F87968AE9DB2.zip` contains **41 cropped Block Blast screenshots and 41 provisional JSON records**. Every JSON record is marked `reviewed:false` and `trainable:false`. The screenshots remain in the private conversation; they were **not** uploaded to this GitHub repository.

Several captures reveal systematic undercounting of colored blocks. Visual checks against actual screenshots:
- Capture prefix `06FA6CF0` gray/peach/purple/green: **20 occupied cells visible** vs **12** in the unreviewed scanner JSON, with eight omitted blocks.
- Capture prefix `4828CBB8`, multicolor illuminated board: **41 occupied cells visible** vs **23** in the unreviewed scanner JSON, with eighteen omitted blocks.
- Capture prefix `887164EE`: **9 occupied cells visible** vs **1** in scanner JSON.
- Capture prefix `07D60147` green theme: **20 occupied** according to both the visible board and unreviewed JSON.
- Capture prefix `A12C1B95`: visibly all empty, 0 in the unreviewed JSON.

The earlier palette-independent **seam-RGB distance** classifier alone also missed dark-green tiles on the illuminated board. Green filled blocks and dark empty cells could have similar RGB *distance from the seam*, despite very different actual tile brightness. Therefore **identical readings over 50 fresh frames do not establish image correctness**.

## Changes shipped, still named v3.4

1. `BLVisionLumaFromCenters`: detect the first large brightness gap above a coherent **dark empty-cell population**. It can recognize pastel, green, gray, purple and pink occupied tiles in a single frame without assuming an individual named skin palette. Uncertain/black-inverted themes return no independent proof rather than guessing. Mixed-color fixes may only **add** occupied squares to the main scanner's result, not remove known occupied squares.
2. `BLVisionLumaEvidence` additionally supplies a conservative `definitelyEmpty` mask: destination cells must match the measured empty-cell RGB cluster closely. Brightness alone is not enough to certify empty.
3. `BLAutoDetectDetailed` now passes the independent mask and theme confidence alongside geometry. When the new evidence is strong, it can correct old **false-empty** board reads. A learned verifier trained on unreviewed old labels cannot override this strong pixel evidence.
4. `BLAutoTargetIsProvenEmpty` blocks ANY Automatic drag unless **every cell** of the proposed placement is in that independently verified empty mask. A normal legal solver move on a mistaken vision board is no longer sufficient.
5. `BLAutoBuildDrag` requires connected, unique, normalized tray-piece tiles, refusing fragmentation or duplicated coordinates caused by theme misreads.
6. Existing five-new-scan Automatic Play, one-piece-at-a-time touch safety, fresh post-drag transition proof, no-clear 50 scans, manual highlights, vision collector, 1000-streak counter and original Smart Survival remain.

## Independent testing

`tests/test_v34_reviewed_theme_samples.c` contains **only 64 coarse cell-center RGB triples and visual board masks** from user screenshots, not the screenshots, scoreboard, or unrelated image content. These board masks were inspected visually, independently of the old provisional JSON labels. The regression catches missed-color tile detection and rejects a uniform empty board as insufficient for certain-empty automation. `tests/test_v34_auto_drag.c` checks that any occupied/uncertain destination, disconnected shape, or invalid move prevents the automatic drag.

All original C regression tests and macOS ARM64 iPhone dylib build remain mandatory in GitHub Actions. A successful compile does not establish real-device accuracy on every skin or demonstrate actual iOS touch injection. Users must test on the iPhone; if a scan looks wrong, turn off Automatic and provide the full game screenshot before further play.

The separate experimental 10× strategy branch is unchanged and no unverified training labels or candidate strategy weights were promoted.
