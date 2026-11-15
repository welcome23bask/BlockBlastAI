#include "../src/BlockBlastSolver.h"
#include "../src/BLDualStrategy.h"
#include "../src/BLV25Strategy.h"
#include "../src/BLThreeMoveClear.h"
#include "../src/BLGuideSafety.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
int main(void) {
    BLComboContext c={0};
    for(int i=1;i<=1000;++i){
        c=BLComboAdvance(c,1);
        assert(c.known && c.misses==0 && c.streak==i);
    }
    for(int i=0;i<25;++i){
        c=BLComboAdvance(c,2);
        assert(c.streak==1000 && c.misses==0);
    }
    // Non-clearing moves still consume one of THREE move chances, never a
    // wall-clock timeout, even at the maximum tracked streak.
    c=BLComboAdvance(c,0);
    assert(c.streak==1000 && c.misses==1);
    c=BLComboAdvance(c,0);
    assert(c.streak==1000 && c.misses==2);
    c=BLComboAdvance(c,1);
    assert(c.streak==1000 && c.misses==0);
    c=BLComboAdvance(c,0);
    c=BLComboAdvance(c,0);
    c=BLComboAdvance(c,0);
    assert(c.streak==0 && c.misses==0);
    c=(BLComboContext){.known=1,.streak=999,.misses=2};
    c=BLComboAdvance(c,1);
    assert(c.streak==1000&&c.misses==0);
    // A fast legal combo save still exists with the 1000-streak context.
    assert(BL_COMBO_STREAK_MAX==1000);
    uint64_t board=UINT64_C(0x7f)<<56;
    BLPiece p[3]={{0}};
    p[0].count=1;p[0].x[0]=0;p[0].y[0]=0;
    c=(BLComboContext){.known=1,.streak=1000,.misses=2};
    BLThreeMovePlan clear=BLFindThreeMoveClear(board,p,c,600000);
    assert(clear.found && clear.firstClear==1 && clear.triesLeft==1);
    assert(BLGuideValidateSolution(board,p,&clear.solution));
    BLTwoStrategies original=BLFindTwoStrategies(board,p,c,20000);
    BLTwoStrategies smart=BLV25FindTwoStrategies(board,p,c,20000);
    if(original.smartSurvival.movesFound)
        assert(BLGuideValidateSolution(board,p,&original.smartSurvival));
    if(smart.smartSurvival.movesFound)
        assert(BLGuideValidateSolution(board,p,&smart.smartSurvival));
    puts("PASS combo tracking 1->1000, 1000 saturation, 3-miss reset, solver rescue at streak 1000");
    return 0;
}
