#ifndef BL_FUTURE_100_H
#define BL_FUTURE_100_H
#include "BLDualStrategy.h"
#ifdef __cplusplus
extern "C" {
#endif
// 100 distinct normalized, 8-connected future-piece probes. This is not
// a learned piece distribution or a claim of 100 independently proven gains.
typedef struct {
    int shapeAccess;       // number of shapes with >=1 legal landing
    int cappedMobility;    // sum of min(placements,4) over 100 probes
    int criticalAccess;    // 3x3, 5-wide, 5-tall, 3x2, 2x3
    int observedAccess;    // 35 scanner-observed tray shapes with any landing
    int observedMobility;  // sum of capped landings for the first 35 shapes
    int emptyRegions;      // 4-connected free-space islands
    int tinyEmptyCells;    // empty cells stranded in islands of size 1 or 2
    int largestEmptyRegion;// area of largest connected playable empty region
} BLFuture100Quality;
int BLFuture100ShapeCount(void);
int BLFuture100DescribeShape(int index,uint64_t *mask,int *width,
                              int *height,int *cells);
BLFuture100Quality BLFuture100Measure(uint64_t board);
// A candidate may replace Smart Survival ONLY when strictly safer under
// 100 shape probes, both routes are valid, combo timing isn't harmed,
// and critical landing zones don't disappear. No speculative future trays.
BLSolution BLFuture100GuardedChoice(uint64_t board,const BLPiece tray[3],
                                     BLComboContext combo,
                                     const BLSolution *baseline,
                                     const BLSolution *candidateA,
                                     const BLSolution *candidateB,
                                     int *upgraded);
#ifdef __cplusplus
}
#endif
#endif
