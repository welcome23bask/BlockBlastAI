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
// Luminance-class evidence for dark-background / brighter-block themes.
// Unlike seam-distance clustering, this detects dark-green, purple and
// peach blocks even when their seam RGB distance resembles empty cells.
// A failure is NOT evidence that all 64 cells are empty.
typedef struct {
    uint64_t filled;
    uint64_t definitelyEmpty; // cells close to visually measured empty RGB
    int separation;           // observed empty-to-filled luminance gap
    int emptySamples;
} BLVisionLumaEvidence;
int BLVisionLumaFromCenters(const uint8_t centers[64][3],
                            BLVisionLumaEvidence *out);
int BLVisionLumaBoard(const uint8_t *rgba,int width,int height,
                      const BLVisionResult *geometry,
                      BLVisionLumaEvidence *out);

// Auto-play's first move can start on a genuinely empty grid. A multi-color
// classifier has no filled class to split on such a frame. Certify ONLY when
// all 64 cell centers agree and an independent grid-seam pattern is present.
// This does not interpret ambiguous color patterns as empty.
int BLVisionUniformEmptyBoard(const uint8_t *rgba,int width,int height,
                              const BLVisionResult *geometry,
                              uint64_t *provenEmptyMask);
#ifdef __cplusplus
}
#endif
#endif
