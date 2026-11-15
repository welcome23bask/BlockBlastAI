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
#endif
