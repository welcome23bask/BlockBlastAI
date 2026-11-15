#ifndef BL_VISION_CONTRAST_H
#define BL_VISION_CONTRAST_H
#include "BLVisionCore.h"
#ifdef __cplusplus
extern "C" {
#endif
// Second, palette-independent interpretation of an ALREADY VERIFIED 8x8
// geometry. Uses RGB distance between per-cell centers and actual board
// grid-intersection color, not names like "brick" or "green". Never
// touches tray labels. Returns 1 only for strongly separated image classes.
int BLVisionContrastBoard(const uint8_t *rgba,int width,int height,
                          const BLVisionResult *geometry,uint64_t *board,
                          int *margin);
#ifdef __cplusplus
}
#endif
#endif
