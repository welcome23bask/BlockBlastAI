#ifndef BL_STRUCTURE_VISION_H
#define BL_STRUCTURE_VISION_H
#include "BLVisionCore.h"
#ifdef __cplusplus
extern "C" {
#endif
// A skin-neutral scanner: uses board contour + grid seams, local empty-cell
// contrast, and connected components of tray silhouettes. No RGB color lists.
int BLStructureAnalyzeRGBA(const uint8_t *rgba,int width,int height,BLVisionResult *out);
// Call when the user recalibrates, switches skins, or restarts the game.
// The game overlay preserves reference background between ordinary frames.
void BLStructureReset(void);
#ifdef __cplusplus
}
#endif
#endif
