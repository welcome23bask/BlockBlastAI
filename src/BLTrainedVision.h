#ifndef BL_TRAINED_VISION_H
#define BL_TRAINED_VISION_H
#include "BLVisionCore.h"
#ifdef __cplusplus
extern "C" {
#endif
// Experimental verifier trained on 196 UNREVIEWED one-skin screenshots.
// 0 not applicable, 1 agree, -1 confident disagreement. Never overwrites scan.
int BLTrainedVisionOpinionRGBA(const uint8_t *rgba,int width,int height,
                                const BLVisionResult *nativeResult);
#ifdef __cplusplus
}
#endif
#endif
