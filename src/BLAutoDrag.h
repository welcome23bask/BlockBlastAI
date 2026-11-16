#ifndef BL_AUTO_DRAG_H
#define BL_AUTO_DRAG_H
#include "BlockBlastSolver.h"
#include <stdint.h>
typedef struct {
    float fromX,fromY,toX,toY;
    int anchorX,anchorY,pieceIndex;
} BLDragPlan;
// Validate real board move independently and map one source tile to its
// target 8x8 cell. Returned coordinates use UIKit screen points.
int BLAutoBuildDrag(uint64_t board,const BLPiece tray[3],
                    const BLMove *move,float firstX,float firstY,
                    float lastX,float lastY,const float trayX[3],
                    const float trayY[3],const float trayPitch[3],
                    float screenWidth,float screenHeight,BLDragPlan *out);

// Every target cell of the selected piece must be CERTAINLY EMPTY in
// independent per-cell RGB evidence from the SAME confirmed frame.
int BLAutoTargetIsProvenEmpty(const BLPiece tray[3],const BLMove *move,
                             uint64_t certainlyEmptyMask);

// Eased motion to the exact same destination as the yellow-highlighted
// selected anchor square. At fraction >=1, coordinates are IDENTICAL to
// target (no extrapolation or momentum beyond the highlight).
void BLAutoDragPoint(const BLDragPlan *plan,float fraction,
                     float *x,float *y);
// Verifies the computed drag drop is the precise board-center position
// of the selected, occupied tray anchor tile shown in the yellow guide.
int BLAutoDragEndsOnHighlight(const BLDragPlan *plan,
                              const BLMove *move,
                              const BLPiece tray[3],
                              float firstX,float firstY,
                              float lastX,float lastY);
#endif
