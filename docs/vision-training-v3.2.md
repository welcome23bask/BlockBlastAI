# BlockBlastAI v3.2 — Opt-in Vision Training

Vision Training is a **local screenshot collector**, not a trained model or a remote training service.

## Start collecting on your iPhone

1. Open Block Blast with the new v3.2 iPhone library installed.
2. Tap the floating **AI** bubble and scroll to **VISION TRAINING** at the bottom.
3. Tap **TRAINING: OFF** to explicitly switch it **ON**.
4. Make sure **AUTO SCAN** is ON, then close the panel and play normally.
5. The existing ReplayKit capture scans stable board/tray states. After at least 3 matching fresh frames over 0.65 seconds, the collector may save a new unique board/tray crop.
6. Open the panel to see **X/200 saved locally**. Tap **Export training ZIP** and choose *Save to Files* to copy the dataset out.
7. Only after exporting, optionally tap **Clear samples** to start a new batch.

## Design and privacy

- **OFF by default at every launch.** No recording until the user taps ON.
- Collection is based on existing ReplayKit game frames. Never captures continuously at 60 FPS for storage.
- Saves only the board and three-piece tray region, not the score, account details, notifications or unrelated screen contents. Incidental UI covering the board may still appear in a crop, so inspect before sharing.
- All images and JSON metadata stay under the host app's private Documents/BlockBlastAI/VisionTraining folder. No automatic cloud upload, model retraining, phone-photo permission, or GitHub upload.
- Up to 200 distinct states per batch; at least 1.5 seconds between recorded states; deduped using board/tray shape and approximate skin fingerprint.
- Automatic JSON labels have `reviewed: false` and `trainable: false` by design. Never treat predictions as ground truth before human review and correction.
- Images are JPEG board+tray crops; JSON gives the 8×8 occupancy array, three local tray masks, crop coordinates, timestamp and dedupe key.
- ZIP uses a minimal standard "store" writer with CRC32; exported through the iOS share sheet.
- The feature is **not** background recording: the game must be open and actively producing permitted ReplayKit frames.
- The solver, calibration and highlight strategy are unchanged.

## Next phase (not part of v3.2)

Build a mobile labeling UI to correct board and piece masks, split by skin into train/holdout groups, train a lightweight board-cell classifier and tray-mask network using those reviewed labels, measure accuracy, export Core ML, and integrate it as a second opinion. Never train directly on unreviewed scanner predictions.
