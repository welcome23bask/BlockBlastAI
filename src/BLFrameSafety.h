#ifndef BL_FRAME_SAFETY_H
#define BL_FRAME_SAFETY_H
#include "BlockBlastSolver.h"
#ifdef __cplusplus
extern "C" {
#endif
// Full rows and columns cannot persist in a settled Block Blast board;
// they are cleared as soon as a piece is placed.
int BLBoardHasUnclearedLine(uint64_t board);
// Validate ONLY the next move against the currently scanned live board.
// A later move is hypothetical and may only be shown after a new scan.
int BLNextGuideSafe(uint64_t board,const BLPiece tray[3],const BLSolution *answer);
#ifdef __cplusplus
}
#endif
#endif
