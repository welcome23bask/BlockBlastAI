# BlockBlastAI Simulator & Debug Lab

**Open the playable simulator:** \`simulator/index.html\` is a single, self-contained HTML5 file. It requires no internet, account, JavaScript libraries, app installation, or jailbreak.

- On a computer, download \`index.html\` and open in a browser.
- On an iPhone, download the **BlockBlastAI_Simulator.html** asset from the project's latest GitHub Release, save it in Files, then open it in a browser-capable HTML viewer. Safari may not always allow local HTML JavaScript launched directly from Files; if so, use GitHub Pages or another static host.
- To make a web URL for iPhone Safari, enable **Settings → Pages → Deploy from branch → main → / (root)**. Then the simulator is reachable at \`https://welcome23bask.github.io/BlockBlastAI/simulator/\`. GitHub Pages must be enabled by the repository owner; a code upload alone does not activate hosting.

The simulator recreates the 8×8 puzzle, three-piece tray, row/column clearing, move solver, a gold-highlight placement guide, and a draggable/resizable AI status panel. Its scenario picker includes a reconstructed board from the user's game screenshots, without publishing private screenshots themselves.

**Fault injection:** tests for a stopped screenshot service, occluded screen, phantom board squares, invented tray pieces, and vertically flipped images. The scanner gate requires three identical observations, checks board geometry and tray changes, and stops highlighting when scans are inconsistent. A virtual 60-minute test exercises 3,600 frames with injected bad observations.

**Run regression tests:** \`node simulator/test.js\`. These tests use the same embedded engine code the browser executes.

**Scope:** The simulator is not the official Block Blast app. A simulator passing regression tests cannot prove iOS ReplayKit capture, injection or in-game highlights work. The iPhone overlay is a separate \`.dylib\` release and needs a real-device test after any update.
