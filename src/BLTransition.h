#ifndef BL_TRANSITION_H
#define BL_TRANSITION_H
#include "BlockBlastSolver.h"
#ifdef __cplusplus
extern "C" {
#endif
// A single fresh frame may be trusted only when it EXACTLY matches the
// first recommended placement and the tray change expected from that move.
// Otherwise the caller must wait for multiple independent captured frames.
int BLIsExpectedTransition(uint64_t oldBoard, const BLPiece oldTray[3],
                           const BLSolution *solution, uint64_t observedBoard,
                           const BLPiece observedTray[3]);
// Infer a placement solely from two VERIFIED consecutive game states.
// Returns 1 only for one unambiguous legal placement, including the case
// where placing the last tray piece immediately refills all three slots.
int BLInferObservedPlacement(uint64_t oldBoard,const BLPiece oldTray[3],
                              uint64_t newBoard,const BLPiece newTray[3],
                              BLMove *observed);
// Maintain a separately verified streak anchor even if ReplayKit drops a
// screenshot briefly. Unknown or conflicting observations reset confidence.
typedef struct {
    uint64_t board;
    BLPiece tray[3];
    BLComboContext streak;
    double observedAt;
    int anchored;
} BLComboTracker;
void BLComboTrackerReset(BLComboTracker *tracker);
// Return 1 only when a uniquely identified legal placement was tracked.
// A no-change observation preserves trusted state, but does not increment it.
int BLComboTrackerObserve(BLComboTracker *tracker,uint64_t board,
                          const BLPiece tray[3],double timestamp);
#ifdef __cplusplus
}
#endif
#endif
