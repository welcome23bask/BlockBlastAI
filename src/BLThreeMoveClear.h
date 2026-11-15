#ifndef BL_THREE_MOVE_CLEAR_H
#define BL_THREE_MOVE_CLEAR_H
#include "BlockBlastSolver.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
// Search all visible tray-piece orders and legal board placements for a
// clearing line within the current three-placement combo window.
// A clear resets the window; if no clear is provable, caller uses Smart Survival.
// The game must be rescanned after each move before highlighting the next.
typedef struct {
    BLSolution solution;        // successive physical placements up to the clear (and optionally remaining pieces)
    int found;                  // one or more actual line clears in known pieces
    int firstClear;             // 1,2,3 (placement index to first clear)
    int triesLeft;              // 1..3 before an active combo expires
    int visiblePieces;          // only shapes detected in current tray
    int completeSearch;         // 0 if bounded DFS was exhausted
    uint32_t explored;
    int upperBoundPruned;        // proved no row/column can clear using known pieces within remaining tries
} BLThreeMovePlan;

BLThreeMovePlan BLFindThreeMoveClear(uint64_t board,const BLPiece pieces[3],
                                     BLComboContext combo,uint32_t budget);
// Use a larger search (up to 1.5M nodes) but cap wall-clock work to
// hardBoardThinkingMs. Never claim "no clear" if the search was truncated.
BLThreeMovePlan BLFindThreeMoveClearTimed(uint64_t board,const BLPiece pieces[3],
                                          BLComboContext combo,uint32_t budget,
                                          uint32_t hardBoardThinkingMs);
#ifdef __cplusplus
}
#endif
#endif
