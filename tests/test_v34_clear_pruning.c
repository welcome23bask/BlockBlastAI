// Proof-bound regression for visible-piece first clears; protects combo deadlines.
#include "../src/BLThreeMoveClear.h"
#include "../src/BLGuideSafety.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
static BLPiece square(void){
    BLPiece p={0};p.count=4;
    p.x[0]=0;p.y[0]=0;p.x[1]=1;p.y[1]=0;
    p.x[2]=0;p.y[2]=1;p.x[3]=1;p.y[3]=1;
    return p;
}
int main(void) {
    BLPiece tray[3]={square(),square(),square()};
    BLThreeMovePlan empty=BLFindThreeMoveClearTimed(
        0,tray,(BLComboContext){0},1500000,1900);
    // Three 2-wide squares can contribute at most six cells to any row;
    // no three placements can fill a whole row or column.
    assert(!empty.found && empty.completeSearch && empty.upperBoundPruned);
    assert(empty.explored==0);
    // One possible clear in a single move: six occupied bottom-row cells,
    // and 2x2 square fills exactly the remaining two cells.
    uint64_t bottom6=UINT64_C(0x3f)<<56;
    BLComboContext lastTry={.known=1,.streak=8,.misses=2};
    BLThreeMovePlan rescue=BLFindThreeMoveClearTimed(
        bottom6,tray,lastTry,1500000,1900);
    assert(rescue.found && rescue.firstClear==1 && !rescue.upperBoundPruned);
    assert(BLGuideValidateSolution(bottom6,tray,&rescue.solution));
    // With four occupied bottom cells, clearing needs two squares, which
    // requires at least two combo tries left.
    uint64_t bottom4=UINT64_C(0x0f)<<56;
    BLComboContext twoTries={.known=1,.streak=8,.misses=1};
    BLThreeMovePlan two=BLFindThreeMoveClearTimed(
        bottom4,tray,twoTries,1500000,1900);
    assert(two.found && two.firstClear<=2 && !two.upperBoundPruned);
    assert(BLGuideValidateSolution(bottom4,tray,&two.solution));
    BLThreeMovePlan one=BLFindThreeMoveClearTimed(
        bottom4,tray,lastTry,1500000,1900);
    assert(!one.found && one.completeSearch && one.upperBoundPruned);
    // Existing diagonal-clear fixture must continue to pass unchanged.
    const uint64_t board=UINT64_C(0xdf3000df3e000003);
    BLPiece odd[3]={{0}};
    odd[1].count=2;odd[1].x[0]=1;odd[1].y[0]=0;
    odd[1].x[1]=0;odd[1].y[1]=1;
    odd[2].count=4;
    odd[2].x[0]=0;odd[2].y[0]=0;
    odd[2].x[1]=1;odd[2].y[1]=0;
    odd[2].x[2]=2;odd[2].y[2]=0;
    odd[2].x[3]=2;odd[2].y[3]=1;
    BLThreeMovePlan diagonal=BLFindThreeMoveClearTimed(
        board,odd,lastTry,1500000,1900);
    assert(diagonal.found && diagonal.firstClear==1);
    assert(BLGuideValidateSolution(board,odd,&diagonal.solution));
    puts("PASS upper-bound pruning no false negatives on 1/2-try rescues, diagonal piece, and 3-square impossibility");
    return 0;
}
