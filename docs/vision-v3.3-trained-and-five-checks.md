# BlockBlastAI v3.3 — Trained vision + no-clear verification

## Trained model (experimental and carefully gated)
- Data source: 196 local screenshot and JSON pairs supplied by user.
- IMPORTANT: all 196 JSON labels have `reviewed: false`, `trainable: false`,
  `source: nativeScanner_unverified`. They are model **teacher labels**, not ground truth.
- A 16x16-patch CNN was trained on 138 images, validated on 29, held out 29.
  It matched 1,856/1,856 held-out **teacher** labels; a 6.7-KB Torch
  checkpoint is available as an external research artifact, not bundled in dylib.
- To keep iPhone inference fast, a separate RGB logistic verifier was trained
  on the same 196 teacher-labelled screenshots with a disjoint 30-screen holdout.
  It matched 1,920/1,920 held-out *teacher* cell predictions.
- v3.3 embeds only the RGB verifier's learned coefficients, NOT the full CNN.
  It activates only on the known purple/navy empty-cell distribution.
  It never changes the native board or pieces. When the models confidently
  disagree, it refuses an unsafe recommendation and waits for a fresh scan.
  Other skins still use the original native/adaptive/optional YOLO vision.
- These numbers are **teacher agreement, not measured correctness**.
  There is no evidence the model improves new-skin accuracy, and the original
  labeler and candidate share the same unreviewed source.
- No gameplay photos, raw images, user private data, or secret model training
  dataset is uploaded into this GitHub repository.

## No-clear verification
1. Original Smart Survival, 3-placement deadline, combo tracking, move guides,
   and tray detections remain in place.
2. Smart mode searches up to 1,500,000 states with a bounded ~1.9-second
   monotonic deadline on the background solver queue.
3. When it does NOT find a clear, it shows legal survival advice as *tentative*
   and DOES NOT tell the user "no clear" yet.
4. Five **additional** live ReplayKit captures must yield the same complete
   board + tray signature, with at least 2 seconds total.
5. Any different board/tray, scan failure, capture restart, or strategy switch
   invalidates pending negative verification and starts fresh.
6. If exhaustive and five scans agree, displays "NO CLEAR FOUND". If the
   time/node bound interrupted the search, displays "No CONFIRMED clear" and
   does NOT claim impossibility.
7. Synthetic C regression tests cover fresh-frame counting, stale-frame
   exclusion, changed-board cancellation, learned-skin gating, and solver rules.

## Safety and limits
- Any screenshot collector remains opt-in and sandbox-local as in v3.2.
- Five scans cannot prove a visually misread state is correct if the same
  scanner is consistently wrong; that is why the known-skin learned second
  opinion and manual correction process remain important.
- The budget is a cap for the deeper 3-try planner, not a hard end-to-end
  latency guarantee for the whole strategy.
- Real iPhone tests are still required after CI ARM64 build.
