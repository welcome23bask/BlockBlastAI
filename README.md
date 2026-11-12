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
