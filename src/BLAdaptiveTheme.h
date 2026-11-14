#ifndef BL_ADAPTIVE_THEME_H
#define BL_ADAPTIVE_THEME_H
#include "BLVisionCore.h"
// Theme-independent fallback for brown/wood and other high-contrast boards.
// Never guesses if the four square boundaries or grid/tray are uncertain.
int BLAdaptiveAnalyzeRGBA(const uint8_t *rgba, int width, int height, BLVisionResult *out);
#endif
