// Repeatedly prove a full-tray rescue is legal and cannot delay an expiring clear.
#include "../src/BLFuture100.h"
#include "../src/BLGuideSafety.h"
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
static BLPiece one(void){
    BLPiece p={0};p.count=1;p.x[0]=0;p.y[0]=0;return p;
}
int main(void){
    BLPiece tray[3]={one(),one(),one()};
    BLSolution partial={0};partial.movesFound=1;
    partial.moves[0]=(BLMove){
        .pieceIndex=0,.row=0,.col=0,.before=0,.after=UINT64_C(1)
    };
    BLSolution complete={0};complete.movesFound=3;
    complete.moves[0]=partial.moves[0];
    complete.moves[1]=(BLMove){
        .pieceIndex=1,.row=0,.col=1,.before=1,.after=3
    };
    complete.moves[2]=(BLMove){
        .pieceIndex=2,.row=0,.col=2,.before=3,.after=7
    };
    assert(BLGuideValidateSolution(0,tray,&partial));
    assert(BLGuideValidateSolution(0,tray,&complete));
    int changed=0;
    BLSolution rescued=BLFuture100GuardedChoice(0,tray,(BLComboContext){0},
                                       &partial,&complete,0,&changed);
    assert(changed && rescued.movesFound==3);
    assert(BLGuideValidateSolution(0,tray,&rescued));
    assert(rescued.moves[2].after==7);
    // A completely failed baseline must not hide a second objective's
    // independently validated full-tray rescue.
    BLSolution none={0};
    changed=0;
    BLSolution zeroRescue=BLFuture100GuardedChoice(0,tray,(BLComboContext){0},
                                       &none,&complete,0,&changed);
    assert(changed && zeroRescue.movesFound==3);
    assert(BLGuideValidateSolution(0,tray,&zeroRescue));
    BLSolution broken=complete;broken.moves[2].before=4;
    changed=1;
    BLSolution ignored=BLFuture100GuardedChoice(0,tray,(BLComboContext){0},
                                       &partial,&broken,0,&changed);
    assert(!changed && ignored.movesFound==1);

    // Prioritized expiring combo: a legal 1-move clear MUST NOT be replaced
    // by a full 2-move route that delays the first clear beyond the deadline.
    uint64_t bottom7=UINT64_C(0x7f)<<56;
    BLPiece remain[3]={one(),one(),{0}};
    BLSolution clearNow={0};clearNow.movesFound=1;clearNow.totalLines=1;
    clearNow.moves[0]=(BLMove){
        .pieceIndex=0,.row=7,.col=7,.before=bottom7,
        .after=0,.linesCleared=1
    };
    assert(BLGuideValidateSolution(bottom7,remain,&clearNow));
    BLSolution delayed={0};delayed.movesFound=2;delayed.totalLines=1;
    uint64_t afterFirst=bottom7|(UINT64_C(1)<<48);
    delayed.moves[0]=(BLMove){
        .pieceIndex=1,.row=6,.col=0,.before=bottom7,
        .after=afterFirst,.linesCleared=0
    };
    delayed.moves[1]=(BLMove){
        .pieceIndex=0,.row=7,.col=7,.before=afterFirst,
        .after=(UINT64_C(1)<<48),.linesCleared=1
    };
    assert(BLGuideValidateSolution(bottom7,remain,&delayed));
    BLComboContext lastTry={.known=1,.streak=4,.misses=2};
    changed=1;
    BLSolution protected=BLFuture100GuardedChoice(bottom7,remain,lastTry,
                                      &clearNow,&delayed,0,&changed);
    assert(!changed && protected.movesFound==1 &&
           protected.moves[0].linesCleared==1);
    puts("PASS complete-tray rescue, invalid candidate rejection, and expiring combo priority");
    return 0;
}
