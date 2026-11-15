#include "../src/BLFuture100.h"
#include "../src/BLV25Strategy.h"
#include "../src/BLGuideSafety.h"
#include "../src/BLFrameSafety.h"
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#ifndef BL_SCENARIO_SEED
#define BL_SCENARIO_SEED 0x6c6c2a47U
#endif
static uint32_t rng=BL_SCENARIO_SEED;
static uint32_t next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static int first_clear(const BLSolution *a) {
    for(int k=0;k<a->movesFound;++k)if(a->moves[k].linesCleared)return k+1;
    return 0;
}
static int lines(const BLSolution *a){
    int l=0;for(int k=0;k<a->movesFound;++k)l+=a->moves[k].linesCleared;return l;
}
int main(void){
    int upgraded=0,valid=0,preempted=0,observations=0;
    for(int seed=0;seed<100;++seed){
        // Independent synthetic scenarios cover low and heavy occupancy.
        uint64_t board=(((uint64_t)next()<<32)|next());
        if(seed%3==0)board&=(((uint64_t)next()<<32)|next());
        if(seed%3==1)board|=(((uint64_t)next()<<32)|next());
        if(BLBoardHasUnclearedLine(board))board=0;
        BLPiece tray[3]={{0}};
        for(int p=0;p<3;++p){
            uint64_t mask=0;int w=0,h=0,cells=0;
            assert(BLFuture100DescribeShape((int)(next()%100),&mask,&w,&h,&cells));
            for(int y=0;y<h;++y)for(int x=0;x<w;++x)
                if(mask&(UINT64_C(1)<<(8*y+x))){
                    int k=tray[p].count++;
                    tray[p].x[k]=x;tray[p].y[k]=y;
                }
            assert(tray[p].count==cells);
        }
        BLComboContext combo={0};
        if(seed%4==0){combo.known=1;combo.streak=3;combo.misses=seed%3;}
        BLTwoStrategies b=BLV25FindTwoStrategies(board,tray,combo,3500);
        BLSolution base=b.smartSurvival;
        if(!BLGuideValidateSolution(board,tray,&base))continue;
        ++valid;
        int didUpgrade=0;
        BLSolution result=BLFuture100GuardedChoice(board,tray,combo,&base,
            &b.keepCombo,&b.maxLines,&didUpgrade);
        assert(BLGuideValidateSolution(board,tray,&result));
        assert(result.movesFound==base.movesFound);
        if(didUpgrade) {
            ++upgraded;
            assert(lines(&result)>=lines(&base));
            int latest=3-(combo.known&&combo.streak>0?combo.misses:0);
            int original=first_clear(&base),changed=first_clear(&result);
            if(original && original<=latest)
                assert(changed && changed<=original);
            BLFuture100Quality old=BLFuture100Measure(base.moves[base.movesFound-1].after);
            BLFuture100Quality new=BLFuture100Measure(result.moves[result.movesFound-1].after);
            assert(new.shapeAccess>=old.shapeAccess+3);
            assert(new.cappedMobility>=old.cappedMobility+12);
            assert(!(old.criticalAccess&~new.criticalAccess));
            assert(new.observedAccess>=old.observedAccess);
            assert(new.observedMobility>=old.observedMobility);
            assert(new.tinyEmptyCells<=old.tinyEmptyCells);
            if(new.emptyRegions>old.emptyRegions)
                assert(new.largestEmptyRegion>=old.largestEmptyRegion);
        }
        ++observations;
    }
    printf("100 seeded test boards (seed=%u): valid_baseline=%d, evaluated=%d, shape-upgrades=%d, preempted=%d\n",(unsigned)BL_SCENARIO_SEED,valid,observations,upgraded,preempted);
    puts("PASS: no illegal moves, combo-deadline regression, line-clear reduction, or lost critical landing types");
    return 0;
}
