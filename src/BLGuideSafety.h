#ifndef BL_GUIDE_SAFETY_H
#define BL_GUIDE_SAFETY_H
#include "BlockBlastSolver.h"
#ifdef __cplusplus
extern "C" {
#endif
// Verifies a whole solver sequence against the independently scanned board.
// Does not assume that solver output or piece recognition is always correct.
// A rejected move must never be painted onto the live game screen.
int BLGuideValidateSolution(uint64_t board,const BLPiece pieces[3],
                            const BLSolution *solution);
#ifdef __cplusplus
}
#endif
#endif
