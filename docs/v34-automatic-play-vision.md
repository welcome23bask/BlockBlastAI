# BlockBlastAI v3.4 — Automatic Play and contrast-verified vision

## User-provided evidence
- ZIP `BlockBlastAI-Vision-76693D41-CB90-47C6-A285-A05B20E95E15.zip`:
  3 photo/JSON pairs.
- ZIP `BASEBALL9.zip`: despite its name, **200 Block Blast photo/JSON pairs**.
  These are real Block Blast board/tray captures across several skins,
  including brick, dark blue, wood and textured tiles. They are **not
  Baseball 9 screenshots**.
- All 203 JSON labels are marked scanner-predicted / `reviewed:false` /
  `trainable:false` and MUST NOT be used as ground-truth training labels.

### Read-only scanner audit
Compared a conservative RGB background-relative center detector to the
*unreviewed* labels using the captured board geometry:
- newer ZIP: 3/3 candidate masks agreed with provisional labels.
- BASEBALL9 ZIP: 192/200 produced high-contrast classifications; 183/192
  matched provisional board masks exactly and 184/192 within two cells;
  8 frames did not meet the strict contrast threshold.
- These are **agreement diagnostics, NOT measured accuracy**, since the
  reference labels came from the previous scanner, not a human verifier.
  Screenshots were NOT uploaded to GitHub; only aggregate data and
  synthetic regression fixtures are committed.

## Scanner improvement
`BLVisionContrastBoard` uses the existing verified 8×8 board geometry
and compares each cell center to the board's 49 interior grid crossings.
It identifies the FIRST strong contrast gap above the background cluster,
rather than treating darkest-colored blocks as empty because white/blue
blocks are much brighter. It rejects uniform, ambiguous, and impossible
full-row/full-column boards. It only overrides native occupancy if
- the old board is impossible (uncleared full row/column) AND a strongly
  separated corrected board was found, or
- at most 3 occupied-cell classifications differ with an exceptional
  separation margin (>=130).

This is a conservative correction, not a trained neural network. Existing
YOLO, learned RGB consistency and other skin scanners remain active.
New scan results still require independent fresh-frame agreement.

## Automatic Play mode
New **🤖 AUTOMATIC PLAY** toggle, separate from existing **AUTO SCAN**:
- Disabled by default. Automatic scanning and all manual highlighting
  remain available separately.
- Requires in-process iOS touch injection support at runtime. The touch
  engine uses a capability-tested adaptation of the MIT-licensed
  `Aethereux/AutoTouch` API with dynamic IOKit symbol lookup, so an
  incompatible or sandboxed iOS version shows 'Automatic unavailable'
  rather than claiming to drag anything.
- Uses accurately scanned local tray cell centers and board coordinates
  to plan the actual on-screen source/target gesture. Unknown/invalid
  source geometry disables automation; no approximate blind tap.
- Requires at least **five** agreeing fresh scans before a move.
- Rechecks that every planned placement is legal, initiates one
  continuous down/move/up gesture, and then waits for the next frame.
- A second move is FORBIDDEN until the new real board/tray independently
  verifies the previous move through `BLIsExpectedTransition` or
  `BLInferObservedPlacement`. A failed/no-effect/mismatched move halts
  the mode. Opening the settings panel or disabling scanning stops it.
- A passing CI build does not prove touches reach the game on the user's
  actual iOS/iGameGod environment. Touch compatibility, drag animation
  offsets, iOS version and theme orientation require on-device testing.

## Preserved
Combo tracking 1000, 50 independent no-clear confirmation frames, Smart
Survival, 100 future-piece shape probes, 3-try combo search, learned RGB
second opinion, optional YOLO support, movable/resizable UI, highlighting,
screenshot export, provisional dataset isolation and the separate
10× research gate. No experimental 10× weights were merged.

## Testing
- Synthetic blue/colored and wood scanner fixtures
- Legal source-tile to board-cell drag mapping, reject invalid positions
- Existing black/red/wood/pink/green vision, diagonal, combo-1000,
  50-frame no-clear and survival tests in GitHub Actions
- Native ARM64 Mach-O build compilation
- Real iPhone interactive validation remains pending
