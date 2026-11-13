# BlockBlastAI v0.4 — Automatic gameplay move assistant (BETA)

**iPhone direct download:** https://github.com/welcome23bask/BlockBlastAI/releases/download/v0.4/BlockBlastAI.dylib  
**Current releases:** https://github.com/welcome23bask/BlockBlastAI/releases  
**Known working on the user's iPhone:** v0.1 was observed running. v0.4 has **only passed a GitHub macOS build and C solver tests**, not a device run.

## Automatic v0.4 features

- **Automatic board calibration:** finds the dark 8×8 square grid in a captured game frame.
- **Automatic board recognition:** estimates occupied versus empty grid cells.
- **Automatic 3-piece recognition:** finds the red/orange/green or similarly bright colored tiles in the three piece-tray thirds below the board.
- **Automatic solving:** uses the local beam-search C solver to plan placements without an internet AI account. Handles 1, 2 or 3 pieces still remaining in the tray.
- **Automatic on-board highlights:** gold outlines indicate the suggested squares. The floating bubble says \`P1 ↓\`, \`P2 ↓\`, etc. to identify which tray piece to use.
- **Automatic refresh:** while the panel is closed and the game is visible, ReplayKit attempts a screenshot every ~1.8 seconds. It waits for two matching observations before changing its suggestion.
- **Movable/resizable settings UI:** existing drag header and ↘ corner remain.

### Test on a non-critical game save

1. Back up your currently working game IPA. In KSign, inject **v0.4 only**, not v0.1/0.2/0.3 alongside it.
2. Sign and install the compatible IPA.
3. Open Block Blast. After a few seconds, iOS may prompt for screen capture permission.
4. **No calibration or manual piece entry should be required.** The bubble should eventually read \`P1 ↓\` / \`P2 ↓\` / \`P3 ↓\` and outlined squares should appear directly over the game grid.
5. If there are no highlights or the tiles are misplaced, tap the bubble. The status field will report if ReplayKit is unavailable or the game frame is unrecognized. If needed, tap **Auto scan** to retry; manual mode remains available as a fallback.
6. Send a screenshot with the actual game and highlights visible for debugging before trusting live recommendations.

**Important limitations:** This is an experimental color/geometry detector tuned using a single user-supplied full-screen screenshot, not a trained vision model and not guaranteed to work across Block Blast skins, animations, ad overlays, device orientations, or layouts. Capturing may be unavailable in some signed/injected builds. Auto is initially ON; tapping the AUTO SCAN control pauses/resumes it. On a scan error it may pause to avoid repeated permission prompts. It never presses, drags, or places a game piece itself. The solver is heuristic and may choose a non-optimal move. Nothing is sent to an external server.

---

# BlockBlastAI v0.3 — Scanner alignment fix and grid preview

**Download v0.3 beta:** https://github.com/welcome23bask/BlockBlastAI/releases/download/v0.3/BlockBlastAI.dylib  
**Previous v0.2:** https://github.com/welcome23bask/BlockBlastAI/releases/download/v0.2/BlockBlastAI.dylib  
**First tested v0.1 fallback:** https://github.com/welcome23bask/BlockBlastAI/releases/download/v0.1/BlockBlastAI.dylib

v0.3 has **passed CI compilation and C solver tests but has not been tested inside the user's iPhone app**.

- **Fix:** set a stricter scanner threshold to reduce mistaken "occupied" readings from the blue game background. This targets false-positive squares; it is not guaranteed to fix all themes or calibration errors.
- **Verify alignment:** after **📍 Calibrate**, a turquoise 8×8 outline appears over the game. Each outlined square must fit an actual board square. The two corner squares are marked 1 and 64. Tap the **🧩 AI** bubble to return to the panel. If the outline is offset, tap **📍 Calibrate** again.
- **Review on demand:** tap **🧭 Check board alignment (show grid)** to re-display the outline without changing calibration.
- **📷 Scan board** now avoids treating moderately bright sky/background as filled blocks in the blue/cyan theme. **Always compare the scanned board with the actual game and correct mistakes by tapping the cells before Solving.**
- **Piece shapes are still manual.** This version does not identify the tray pieces or automate play.

Keep the original compatible IPA and working v0.1 backup. When replacing the dylib in KSign, inject **only one** version. The screenshot scanner can fail depending on ReplayKit availability and game visual style.

---

# BlockBlastAI v0.2 — Movable overlay and beta screen scan

**New version:** [Download v0.2 iPhone ARM64 dylib](https://github.com/welcome23bask/BlockBlastAI/releases/download/v0.2/BlockBlastAI.dylib)  
**Working fallback:** [Download v0.1 dylib](https://github.com/welcome23bask/BlockBlastAI/releases/download/v0.1/BlockBlastAI.dylib)

v0.2 builds successfully on GitHub's macOS runner. It **has not yet been tested on an iPhone**; v0.1 was confirmed working inside Block Blast on an iPhone. Keep the old v0.1 IPA backed up.

## What's new in v0.2
- **Move the panel:** drag its top bar labelled “Drag panel.”
- **Resize the panel:** drag the **↘** handle in its bottom-right corner. The inside content scales to fit and scrolls vertically. Panel position and size are saved on the device.
- **Calibrate board coordinates:** tap **📍 Calibrate** and tap the centers of the top-left and bottom-right board squares on the exposed game. Recalibrate if the game's layout or screen orientation changes.
- **Beta automatic board recognition:** tap **📷 Scan board**. The panel hides temporarily, ReplayKit captures a video frame, and an on-device brightness classifier estimates which 8×8 squares are occupied. Screen capture may ask for permission and may fail inside some games. **Always verify and correct the detected grid before solving.**
- **Overlay the suggested move over the game:** once you've entered all three pieces and solved, tap **🎯 On game**. Gold outlined cells show the current suggested placement directly on the game grid without blocking gameplay. Tap the floating 🧩 AI bubble to reopen the panel; use Previous/Next to change which move is highlighted.
- The solver remains local and heuristic. It does not move blocks automatically or transmit screen images.

## Still manual / experimental
- Piece-shape recognition is **not implemented in v0.2**. Draw all three piece shapes in the existing editors.
- Brightness-based detection may misread animated themes, special tiles, board effects, or fully empty/filled boards.
- UI capture and visual alignment have not been verified on the user's specific Block Blast installation.
- A successful compilation does not guarantee that modified game clients will run.
- Neither v0.1 nor v0.2 is an official Block Blast feature.

---

# BlockBlastAI Overlay — iPhone-only development starter (v0.1)

This is **source code**, not a finished `.dylib` or IPA. The target is a floating AI move assistant **inside a compatible, signed copy of Block Blast** on an iPhone. It is experimental and has not been tested inside Block Blast.

## What it does

- Displays a floating **🧩 AI** button over the host app (not a systemwide overlay).
- Tap it to open an 8×8 board and three editable 5×5 piece templates.
- Tap `SOLVE` to get the suggested order and 1-based row/column positions.
- Preview each recommended move on the overlay's grid in gold.
- Uses a compiled C beam-search solver. It is *heuristic*, not a guaranteed best score or future-tray predictor.
- No screenshots, network calls, game memory edits, or autoplay in v0.1.

## Files

```
.github/workflows/build.yml    GitHub Actions cloud iPhone compilation
src/BlockBlastOverlay.m       iOS UIKit overlay
src/BlockBlastSolver.h       Solver interface
src/BlockBlastSolver.c       Solver algorithm
tests/test_solver.c          Basic C tests
```

## What you need

1. iPhone on iOS 26.6; Files app and Safari.
2. KSign and a working signing setup; user already has these.
3. GitHub account: https://github.com/ . Github Actions needs to be enabled, with available macOS minutes/billing eligibility.
4. A *compatible and legitimately obtained Block Blast `.ipa`* that KSign can sign and inject. The regular App Store installation cannot simply be modified in place.

No paid AI API is necessary: the solver runs locally in the injected dylib.

## Cloud-build steps from Safari on iPhone

1. Open github.com and create a new repository (e.g., `BlockBlastAI`).
2. In the iPhone Files app, uncompress `BlockBlastAI_Overlay_Starter.zip`.
3. Upload the contents into the repository preserving the directory paths `src/`, `tests/` and `.github/workflows/`. GitHub's web `Add file > Upload files` can upload source files; for nested workflow path use `Add file > Create new file` and name the path `.github/workflows/build.yml`, then paste the provided YAML if the UI won't upload folders.
4. Open the repository's **Actions** tab. Allow workflows if GitHub asks.
5. Choose **Build iPhone Block Blast AI Dylib** > **Run workflow**. It also starts on pushes to the tracked files.
6. When the run succeeds, open the run page > **Artifacts** > `BlockBlastAI-iOS-arm64`. Download; GitHub bundles artifacts as a ZIP. Uncompress it using Files and retrieve `BlockBlastAI.dylib`.
7. KSign: import your *compatible* Block Blast IPA, find the signing/tweak injection option, select `BlockBlastAI.dylib`, sign with your own signing setup, and install.
8. Back up your game progress first; test the modified app without extra tweaks. Look for a **🧩 AI** floating button.

KSign **does not compile source code**. The GitHub macOS runner does this. Apple requires proper app and dynamic-library signatures; KSign's on-device signing still needs to produce an acceptable package.

## First use

1. Open Block Blast, tap 🧩 AI.
2. Tap all filled cells in the 8×8 grid.
3. Tap cells in each of the three piece-editing templates (keep their orientation as shown in game).
4. Tap `SOLVE CURRENT THREE PIECES`.
5. Gold cells show where the selected piece goes. Use Previous / Next to preview each step. Row 1 = top, column 1 = left.
6. Close the overlay when placing pieces. Enter the next board and pieces as needed.

## Limitations and caution

- **This is a starter, not a tested, signed, or installable IPA.** A successful build does not guarantee the injected overlay will appear in the game. The host app's rendering and scene lifecycle may require additional compatibility work.
- Does not read Block Blast's screen automatically. Automatic recognition and drawing over the game board require further work.
- Uses a 100-state beam search with a heuristic score, **not** guaranteed mathematically optimal.
- If the game has 6+ cell pieces that don't fit the 5×5 editors, this UI needs expansion.
- Apple's required code signing and app protections apply, and game terms may restrict modified clients.
- Do not share private signing certificates, Apple passwords, or proprietary game IPA files publicly.
- If you see no 🧩 AI icon, confirm the unsigned dylib was injected and signed, check app launch behavior, and try a build with no other tweaks before debugging.

## Sources for the approach

- KSign releases: https://github.com/Nyasami/Ksign/releases
- Apple code signing: https://support.apple.com/guide/security-pdf/app-code-signing-process-sec7c917bf14/web
- Example external Block Blast board-solver reference (not used in this code): https://github.com/tiltti/blockblast-solver
