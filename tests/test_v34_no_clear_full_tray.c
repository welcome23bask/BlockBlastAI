// User-requested negative notification must mean no clear across ALL visible
// piece orders, not merely no combo-saving clear in the remaining deadline.
#include "../src/BLThreeMoveClear.h"
#include "../src/BLNoClearReview.h"
#include "../src/BLGuideSafety.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
static BLPiece square(void) {
    BLPiece p={0};p.count=4;
    p.x[0]=0;p.y[0]=0;p.x[1]=1;p.y[1]=0;
    p.x[2]=0;p.y[2]=1;p.x[3]=1;p.y[3]=1;
    return p;
}
static BLComboContext last_try(void) {
    return (BLComboContext){.known=1,.streak=100,.misses=2};
}
int main(void) {
    BLPiece tray[3]={square(),square(),square()};
    uint64_t bottom4=UINT64_C(15)<<56;
    BLThreeMovePlan deadline=BLFindThreeMoveClearTimed(
        bottom4,tray,last_try(),1500000,1900);
    assert(!deadline.found && deadline.completeSearch);
    // A real line clear exists on move 2, although 100x combo cannot be
    // maintained (only one combo try remains). Do not display no-clear!
    BLComboContext fullWindow={0};
    BLThreeMovePlan allTray=BLFindThreeMoveClearTimed(
        bottom4,tray,fullWindow,1500000,1900);
    assert(allTray.found && allTray.firstClear==2);
    assert(BLGuideValidateSolution(bottom4,tray,&allTray.solution));
    BLNoClearReview review={0};
    BLNoClearReviewBegin(&review,0x4444,10,1.);
    for(unsigned i=1;i<=50;++i)
        BLNoClearReviewObserve(&review,0x4444,10+i,1.+i*.25);
    assert(review.complete);
    assert(!BLNoClearProofIsReady(&allTray,&review));
    // Three squares on empty board cannot make an entire row or column:
    // certified safe mathematical negative plus 50 fresh captures.
    BLThreeMovePlan impossible=BLFindThreeMoveClearTimed(
        0,tray,fullWindow,1500000,1900);
    assert(!impossible.found && impossible.completeSearch);
    assert(impossible.upperBoundPruned && impossible.visiblePieces==3);
    assert(BLNoClearProofIsReady(&impossible,&review));
    // A truncated search does NOT justify "no clear" even after 50 captures.
    BLThreeMovePlan uncertain=BLFindThreeMoveClearTimed(
        bottom4,tray,fullWindow,1,1900);
    assert(!uncertain.found && !uncertain.completeSearch);
    assert(!BLNoClearProofIsReady(&uncertain,&review));
    // No tray, or no captures, cannot justify a notification.
    BLThreeMovePlan empty={0};empty.completeSearch=1;
    assert(!BLNoClearProofIsReady(&empty,&review));
    BLNoClearReviewReset(&review);
    assert(!BLNoClearProofIsReady(&impossible,&review));
    puts("PASS 100-combo late clear not mislabeled, 50 scans + exhaustive proof required, truncated search never alerts");
    return 0;
}
