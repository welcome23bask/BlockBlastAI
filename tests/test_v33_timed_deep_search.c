#include "../src/BLThreeMoveClear.h"
#include "../src/BLGuideSafety.h"
#include "../src/BLFrameSafety.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
int main(void) {
    const uint64_t board=UINT64_C(0xdf3000df3e000003);
    BLPiece p[3]={{0}};
    p[1].count=2;p[1].x[0]=1;p[1].y[0]=0;p[1].x[1]=0;p[1].y[1]=1;
    p[2].count=4;p[2].x[0]=0;p[2].y[0]=0;
    p[2].x[1]=1;p[2].y[1]=0;
    p[2].x[2]=2;p[2].y[2]=0;
    p[2].x[3]=2;p[2].y[3]=1;
    BLComboContext combo={0};
    BLThreeMovePlan clear=BLFindThreeMoveClearTimed(board,p,combo,1500000,1900);
    assert(clear.found);
    assert(clear.firstClear>=1&&clear.firstClear<=3);
    assert(BLGuideValidateSolution(board,p,&clear.solution));
    BLThreeMovePlan bounded=BLFindThreeMoveClearTimed(board,p,combo,1,1900);
    assert(!bounded.found && !bounded.completeSearch && bounded.explored<=1);
    BLPiece one[3]={{0}};one[0].count=1;one[0].x[0]=0;one[0].y[0]=0;
    // With only one known block the near-empty board cannot clear a line;
    // there is a mathematically proved impossibility, not a bounded guess.
    BLThreeMovePlan impossible=BLFindThreeMoveClearTimed(0,one,combo,1500000,1900);
    assert(impossible.completeSearch && !impossible.found);
    puts("PASS timed search finds real diagonal clear, reports node limit honestly, and proves trivial no-clear");
    return 0;
}
