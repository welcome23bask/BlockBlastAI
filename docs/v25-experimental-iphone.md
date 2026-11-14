# BlockBlastAI v2.5 EXPERIMENTAL iPhone build

This is a *test build*, not a proven upgrade. v2.5 improved aggregate survival on 210 synthetic paired test games but failed three of five strict heldout epochs. It has **not** achieved 10× or proven improvement on a real iPhone.

## What's in this build
- Exact v2.5 research choice: 4 core heuristic weights `120,120,0,-20`; 6 feature-group weights `0,0,0,1,0,0`. Only the FIVE Round 4 features are enabled: empty region count, largest free region, tiny pockets, horizontal 4-long open runs, vertical 4-long open runs.
- Two separate three-piece solvers in the same binary:
  - `v2.5 TEST AI`: experimental move evaluator using the selected new features.
  - `v1.7 ORIGINAL`: unchanged SMART SURVIVAL baseline for immediate comparison.
- Extra verified combo deadline protection: attempt to preserve a streak with even a single line if all remaining visible tray pieces can still be placed; never invent an illegal preview sequence.
- Existing black-block navy theme scanner, other theme fallbacks, ReplayKit capture, draggable/resizable UI, yellow blinking placement guides, always-visible floating bubble and prior three strategy tabs.
- Existing Core ML YOLO hook is unchanged and still needs separately trained weights.

## Using it on an iPhone
1. Back up the previous working `BlockBlastAI.dylib`.
2. Unzip this experimental ARM64 artifact and replace only the `BlockBlastAI.dylib` in the user's existing KSign injection/signing setup.
3. Launch Block Blast with the same black-square theme; wait for the floating `AI` bubble.
4. Tap the bubble; scroll to the **very bottom** of the AI panel.
5. Switch **v2.5 TEST AI: ON** (purple) / **v1.7 ORIGINAL: ON** (green).
6. Choose **SMART SURVIVAL** in the strategy tabs; v2.5 only replaces this strategy's evaluator. MAX LINES and KEEP COMBO stay original.
7. Close the panel to automatically rescan; every toggle clears previously shown yellow hints.
8. Compare multiple actual games using comparable piece conditions. Observe incorrect detection separately from strategic quality.

The experimental mode defaults **ON** in this special test build. The selection is saved in the app's user defaults. Change back with the bottom switch immediately if performance worsens.

**Limitations:** Successful CI compilation cannot prove runtime loading or screen scanning in the live Block Blast app. iOS signing/injection compatibility depends on the user's app and device. Static feature weights do not automatically learn from gameplay; training is offline. The library only provides placement recommendations; it does not autonomously drag or place game pieces.

## Testing
The dedicated CI build includes tests for exact 8×8 row/column clearing, legal placement sequences, black and previous color themes, the original v1.7 solver, and the v2.5 solver's combo deadline. It also inspects source for the user-facing toggle and chosen weights.

The working original `src/BLDualStrategy.c` is left untouched. The experimental evaluator is a separate `src/BLV25Strategy.c` module.
