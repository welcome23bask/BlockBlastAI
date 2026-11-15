#ifndef BL_VISION_CORE_H
#define BL_VISION_CORE_H
#include <stdint.h>
#include "BlockBlastSolver.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t board;
    BLPiece pieces[3];
    float firstX, firstY, lastX, lastY;
    int flippedInput;
    int detectedPieces;
    // Source coordinates of local (0,0) tray tile for auto-drag.
    // Zero when this theme/provider could not confidently localize it.
    float trayFirstX[3],trayFirstY[3],trayCellPitch[3];
} BLVisionResult;

// rgba is 4 bytes per pixel in RGBA order, stride = width*4.
// Automatically corrects a vertically inverted CoreGraphics bitmap.
// Returns 0 rather than guessing if board/tray geometry is inconsistent.
int BLVisionAnalyzeRGBA(const uint8_t *rgba, int width, int height, BLVisionResult *result);

#ifdef __cplusplus
}
#endif
#endif
