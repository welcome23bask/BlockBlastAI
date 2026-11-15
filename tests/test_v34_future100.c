#include "../src/BLFuture100.h"
#include "../src/BLGuideSafety.h"
#include "../src/BLThreeMoveClear.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
static int connectivity(uint64_t shape,int w,int h,int count){
    uint64_t seen=shape&(~shape+1),old;
    do {old=seen;for(int y=0;y<h;++y)for(int x=0;x<w;++x)
        if((seen>>(8*y+x))&1){
            for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx){
                int xx=x+dx,yy=y+dy;
                if(xx>=0&&xx<w&&yy>=0&&yy<h &&
                   (shape & (UINT64_C(1)<<(8*yy+xx))))
                    seen|=UINT64_C(1)<<(8*yy+xx);
            }
        }
    }while(seen!=old);
    return __builtin_popcountll(seen)==count;
}
static uint32_t rnd=0x4123990;
static uint32_t next(void) {rnd^=rnd<<13;rnd^=rnd>>17;rnd^=rnd<<5;return rnd;}
int main(void){
    assert(BLFuture100ShapeCount()==100);
    // The source dataset contains 35 distinct scanner-predicted tray shapes.
    // Do not silently drop their footprint coverage when changing probes.
    static const uint64_t observed[35]={
        UINT64_C(0x101010101),UINT64_C(0x707),UINT64_C(0x1f),
        UINT64_C(0x1010101),UINT64_C(0x30303),UINT64_C(0x70707),
        UINT64_C(0x303),UINT64_C(0xf),UINT64_C(0x30202),
        UINT64_C(0x70101),UINT64_C(0x702),UINT64_C(0x306),
        UINT64_C(0x30101),UINT64_C(0x10103),UINT64_C(0x207),
        UINT64_C(0x10107),UINT64_C(0x40407),UINT64_C(0x20301),
        UINT64_C(0x70404),UINT64_C(0x407),UINT64_C(0x107),
        UINT64_C(0x20203),UINT64_C(0x704),UINT64_C(0x301),
        UINT64_C(0x20302),UINT64_C(0x103),UINT64_C(0x701),
        UINT64_C(0x203),UINT64_C(0x10302),UINT64_C(0x603),
        UINT64_C(0x7),UINT64_C(0x101),UINT64_C(0x10301),
        UINT64_C(0x3),UINT64_C(0x10101)
    };
    for(int j=0;j<35;++j) {
        int found=0;
        for(int k=0;k<100;++k){
            uint64_t candidate=0;
            BLFuture100DescribeShape(k,&candidate,0,0,0);
            if(candidate==observed[j]){found=1;break;}
        }
        assert(found);
    }
    uint64_t masks[100];
    int widths[100],heights[100],counts[100];
    for(int i=0;i<100;++i){
        assert(BLFuture100DescribeShape(i,&masks[i],&widths[i],&heights[i],&counts[i]));
        assert(widths[i]>0&&widths[i]<=5&&heights[i]>0&&heights[i]<=5);
        assert(counts[i]>=1&&counts[i]<=9);
        assert(__builtin_popcountll(masks[i])==counts[i]);
        assert(connectivity(masks[i],widths[i],heights[i],counts[i]));
        for(int j=0;j<i;++j)assert(masks[j]!=masks[i]);
    }
    assert(!BLFuture100DescribeShape(-1,0,0,0,0));
    assert(!BLFuture100DescribeShape(100,0,0,0,0));
    BLFuture100Quality empty=BLFuture100Measure(0);
    assert(empty.shapeAccess==100 && empty.cappedMobility==400 && empty.criticalAccess==31);
    assert(empty.observedAccess==35 && empty.observedMobility==140);
    assert(empty.emptyRegions==1 && empty.largestEmptyRegion==64);
    assert(empty.tinyEmptyCells==0);
    BLFuture100Quality full=BLFuture100Measure(UINT64_MAX);
    assert(full.shapeAccess==0 && full.cappedMobility==0 && full.criticalAccess==0);
    assert(full.observedAccess==0 && full.observedMobility==0);
    assert(full.emptyRegions==0 && full.largestEmptyRegion==0);
    // Exactly one empty cell is unusable for nearly all pieces.
    BLFuture100Quality trapped=BLFuture100Measure(UINT64_MAX & ~(UINT64_C(1)<<27));
    assert(trapped.emptyRegions==1 && trapped.tinyEmptyCells==1 &&
           trapped.largestEmptyRegion==1);
    // Two isolated corners form two unusable regions.
    BLFuture100Quality corners=BLFuture100Measure(
        UINT64_MAX & ~(UINT64_C(1)<<0) & ~(UINT64_C(1)<<63));
    assert(corners.emptyRegions==2 && corners.tinyEmptyCells==2 &&
           corners.largestEmptyRegion==1);
    // At least 100 independently reproducible random board regressions:
    // adding occupancy must NEVER increase any shape's placement access.
    for(int i=0;i<100;++i) {
        uint64_t b=((uint64_t)next()<<32)|next();
        int missing=__builtin_ctzll(~b ? ~b : UINT64_C(1));
        BLFuture100Quality before=BLFuture100Measure(b);
        BLFuture100Quality after=BLFuture100Measure(b|(UINT64_C(1)<<missing));
        assert(after.shapeAccess<=before.shapeAccess);
        assert(after.cappedMobility<=before.cappedMobility);
        assert(after.observedAccess<=before.observedAccess);
        assert(after.observedMobility<=before.observedMobility);
        assert((after.criticalAccess&~before.criticalAccess)==0);
    }
    // Regression for empty candidates and invalid overlap; never hallucinate.
    BLPiece pieces[3]={{0}};pieces[0].count=1;pieces[0].x[0]=0;pieces[0].y[0]=0;
    BLSolution base={0};base.movesFound=1;base.moves[0].row=0;
    base.moves[0].col=0;base.moves[0].pieceIndex=0;
    base.moves[0].before=0;base.moves[0].after=1;
    assert(BLGuideValidateSolution(0,pieces,&base));
    BLSolution bad=base;bad.moves[0].before=1;
    int upgraded=0;
    BLSolution result=BLFuture100GuardedChoice(0,pieces,(BLComboContext){0},
                                                &base,&bad,0,&upgraded);
    assert(!upgraded && result.moves[0].after==base.moves[0].after);
    // Keep verified line clears from diagonal-piece fixture.
    const uint64_t board=UINT64_C(0xdf3000df3e000003);
    BLPiece tray[3]={{0}};tray[1].count=2;tray[1].x[0]=1;tray[1].y[0]=0;
    tray[1].x[1]=0;tray[1].y[1]=1;
    tray[2].count=4;tray[2].x[0]=0;tray[2].y[0]=0;
    tray[2].x[1]=1;tray[2].y[1]=0;tray[2].x[2]=2;tray[2].y[2]=0;
    tray[2].x[3]=2;tray[2].y[3]=1;
    BLThreeMovePlan plan=BLFindThreeMoveClear(board,tray,(BLComboContext){0},600000);
    assert(plan.found && BLGuideValidateSolution(board,tray,&plan.solution));
    puts("PASS 100 unique valid future shapes, 100 monotonicity tests, safe choice and combo regression");
    return 0;
}
