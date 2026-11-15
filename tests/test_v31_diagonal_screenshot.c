// v3.1 device screenshot regression: true 23 occupied cells,
 // diagonal 2-square P2, orange 4-square L P3, left slot already used.
// This was incorrectly reported as "No legal placement found" in v3.0,
// even though P2 at R7/C6 immediately completes bottom row R8.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../src/BLThreeMoveClear.h"
#include "../src/BLFrameSafety.h"
#include "../src/BLGuideSafety.h"
#include "../src/BLDualStrategy.h"
#include "../src/BLV25Strategy.h"

static BLPiece diagonal(void) {
    BLPiece p={0};p.count=2;
    p.x[0]=1;p.y[0]=0;
    p.x[1]=0;p.y[1]=1;
    return p;
}
static BLPiece orangeL(void) {
    BLPiece p={0};p.count=4;
    p.x[0]=0;p.y[0]=0;
    p.x[1]=1;p.y[1]=0;
    p.x[2]=2;p.y[2]=0;
    p.x[3]=2;p.y[3]=1;
    return p;
}
int main(void) {
    const uint64_t board=UINT64_C(0xdf3000df3e000003);
    assert(__builtin_popcountll(board)==23);
    assert(!BLBoardHasUnclearedLine(board));
    BLPiece tray[3]={{0},diagonal(),orangeL()};
    // The requested move is upper-right of the diagonal at row 7 col 7,
    // lower-left at row 8 col 6, counting one-based board coordinates.
    BLSolution immediate={0};
    immediate.movesFound=1;immediate.totalLines=1;
    immediate.moves[0]=(BLMove){
        .pieceIndex=1,.row=6,.col=5,.before=board,
        .linesCleared=1
    };
    uint64_t withPiece=board|
        (UINT64_C(1)<<(6*8+6))|
        (UINT64_C(1)<<(7*8+5));
    uint64_t bottom=UINT64_C(255)<<(7*8);
    assert((withPiece&bottom)==bottom);
    immediate.moves[0].after=withPiece&~bottom;
    assert(BLGuideValidateSolution(board,tray,&immediate));
    assert(BLNextGuideSafe(board,tray,&immediate));

    // With one combo try remaining the only qualifying path must clear NOW.
    BLComboContext lastTry={.known=1,.streak=4,.misses=2};
    BLThreeMovePlan plan=BLFindThreeMoveClear(board,tray,lastTry,600000);
    assert(plan.found && plan.firstClear==1 && plan.triesLeft==1);
    assert(plan.solution.movesFound>0);
    assert(BLGuideValidateSolution(board,tray,&plan.solution));
    assert(BLNextGuideSafe(board,tray,&plan.solution));
    assert(plan.solution.moves[0].pieceIndex==1);
    assert(plan.solution.moves[0].linesCleared>=1);

    // On a fresh combo there is still a real clear with current pieces.
    BLComboContext fresh={0};
    BLThreeMovePlan freshPlan=BLFindThreeMoveClear(board,tray,fresh,600000);
    assert(freshPlan.found && freshPlan.firstClear>=1 && freshPlan.firstClear<=3);
    assert(BLGuideValidateSolution(board,tray,&freshPlan.solution));
    assert(BLNextGuideSafe(board,tray,&freshPlan.solution));

    BLTwoStrategies smart=BLV25FindTwoStrategies(board,tray,fresh,180000);
    BLTwoStrategies original=BLFindTwoStrategies(board,tray,fresh,180000);
    if(smart.smartSurvival.movesFound)
      assert(BLNextGuideSafe(board,tray,&smart.smartSurvival));
    if(original.smartSurvival.movesFound)
      assert(BLNextGuideSafe(board,tray,&original.smartSurvival));

    // An actual illegal overlap still must be rejected.
    BLSolution overlap=immediate;
    overlap.moves[0].row=6;overlap.moves[0].col=4;
    assert(!BLNextGuideSafe(board,tray,&overlap));

    // A separate disconnected two-tile pair, not even corner-touching,
    // should not be accepted as an accidentally assembled shape.
    BLPiece bad[3]={{0},diagonal(),orangeL()};
    bad[1].x[1]=4;bad[1].y[1]=4;
    assert(!BLGuideValidateSolution(board,bad,&immediate));

    puts("PASS screenshot 415: 23 board cells, diagonal P2, immediate bottom-row clear, 1-try rescue, 3-try rescue, safety checks");
    return 0;
}
