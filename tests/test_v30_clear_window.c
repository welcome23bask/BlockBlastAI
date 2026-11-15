// Three-try-clear planner regression. Only CURRENT visible pieces count.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../src/BLThreeMoveClear.h"
#include "../src/BLGuideSafety.h"
#include "../src/BLFrameSafety.h"

static BLPiece one(void) {
    BLPiece p={0};p.count=1;return p;
}
static BLPiece five(void){
    BLPiece p={0};p.count=5;
    for(int i=0;i<5;++i)p.x[i]=i;
    return p;
}
static void check(BLThreeMovePlan p,uint64_t board,const BLPiece t[3],
                  int firstClear,int tries){
    assert(p.found && p.firstClear==firstClear && p.triesLeft==tries);
    assert(p.solution.movesFound>=firstClear && p.solution.movesFound<=3);
    assert(BLGuideValidateSolution(board,t,&p.solution));
    int preClear=0;
    for(int j=0;j<p.solution.movesFound;++j)
        if(p.solution.moves[j].linesCleared>0){preClear=j+1;break;}
    assert(preClear==firstClear);
}
int main(void){
    BLComboContext fresh={0};
    BLComboContext active={.known=1,.streak=4,.misses=0};
    BLPiece t1[3]={one(),{0},{0}};
    BLPiece t2[3]={one(),one(),{0}};
    BLPiece t3[3]={one(),one(),one()};

    // One, two or three placements can complete a full eight-cell row.
    check(BLFindThreeMoveClear(UINT64_C(0x7f),t1,fresh,500000),
          UINT64_C(0x7f),t1,1,3);
    check(BLFindThreeMoveClear(UINT64_C(0x3f),t2,fresh,500000),
          UINT64_C(0x3f),t2,2,3);
    check(BLFindThreeMoveClear(UINT64_C(0x1f),t3,fresh,500000),
          UINT64_C(0x1f),t3,3,3);

    // After each proven clear, all three tries are available again.
    BLComboContext c={.known=1,.streak=8,.misses=2};
    c=BLComboAdvance(c,1);
    assert(c.misses==0 && c.streak==9);
    c=BLComboAdvance(c,0);
    assert(c.misses==1);
    check(BLFindThreeMoveClear(UINT64_C(0x3f),t2,c,500000),
          UINT64_C(0x3f),t2,2,2);
    c=BLComboAdvance(c,0);
    assert(c.misses==2);
    BLThreeMovePlan late=BLFindThreeMoveClear(UINT64_C(0x3f),t2,c,500000);
    assert(!late.found && late.triesLeft==1 && late.completeSearch);
    check(BLFindThreeMoveClear(UINT64_C(0x7f),t1,c,500000),
          UINT64_C(0x7f),t1,1,1);

    // Transcribed from the user's watermelon screenshot: 37 occupied cells,
    // an available Z4 and five-horizontal piece. The 3rd column can be
    // completed immediately with a LEGAL Z4, even if only one try remains.
    BLPiece z4={0};z4.count=4;
    z4.x[0]=0;z4.y[0]=0;
    z4.x[1]=1;z4.y[1]=0;
    z4.x[2]=1;z4.y[2]=1;
    z4.x[3]=2;z4.y[3]=1;
    BLPiece watermelon[3]={z4,five(),{0}};
    uint64_t melonBoard=UINT64_C(0x7e0c8781f797e7c7);
    BLThreeMovePlan rescue=BLFindThreeMoveClear(melonBoard,watermelon,c,600000);
    check(rescue,melonBoard,watermelon,1,1);

    // Never fabricate a line clear when the three available pieces are
    // insufficient. No next tray can be predicted in advance.
    BLThreeMovePlan none=BLFindThreeMoveClear(0,t3,fresh,500000);
    assert(!none.found && none.completeSearch);
    assert(none.visiblePieces==3);
    BLPiece onlyBar[3]={five(),{0},{0}};
    BLThreeMovePlan bar=BLFindThreeMoveClear(0,onlyBar,active,500000);
    assert(!bar.found && bar.completeSearch);

    // A false 64/64 board scan is invalid regardless of AI scoring.
    assert(BLBoardHasUnclearedLine(UINT64_MAX));
    puts("PASS 3-try planner: 1/2/3 move clears, combo deadline, reset on clear, no fake clears, verified legal moves");
    return 0;
}
