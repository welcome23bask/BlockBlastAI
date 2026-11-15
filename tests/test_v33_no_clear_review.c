#include "../src/BLNoClearReview.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    BLNoClearReview r={0};
    assert(BL_NO_CLEAR_RECHECKS==50);
    // A newly started review must NOT count the frame used by the solver.
    BLNoClearReviewBegin(&r,UINT64_C(0x1234),7,10.);
    assert(BLNoClearReviewObserve(&r,0x1234,7,10.2)==0 && r.checks==0);
    for(unsigned i=1;i<=49;++i){
        assert(BLNoClearReviewObserve(&r,0x1234,7+i,10.+i*.25)==0);
        assert(r.checks==i);
        // No second vote from a duplicated or backward-moving frame counter.
        assert(BLNoClearReviewObserve(&r,0x1234,7+i,10.+i*.25)==0);
        assert(BLNoClearReviewObserve(&r,0x1234,7+i-1,10.+i*.25)==0);
        assert(r.checks==i);
    }
    assert(BLNoClearReviewObserve(&r,0x1234,57,22.5)==1);
    assert(r.complete && !r.active && r.checks==50);
    // No further votes or repeated notification after completed.
    assert(BLNoClearReviewObserve(&r,0x1234,58,22.8)==-1);
    // Abort on a change in the board OR the tray signature.
    BLNoClearReviewBegin(&r,0x999,80,30.);
    assert(BLNoClearReviewObserve(&r,0x999,81,30.2)==0 && r.checks==1);
    assert(BLNoClearReviewObserve(&r,0x1000,82,30.4)==-1);
    assert(!r.active&&!r.complete);
    // Fifty confirmations are still insufficient until 2sec has elapsed.
    BLNoClearReviewBegin(&r,0x999,100,40.);
    for(unsigned i=1;i<=50;++i)
        assert(BLNoClearReviewObserve(&r,0x999,100+i,40.+i*.02)==0);
    assert(r.checks==50 && r.active && !r.complete);
    assert(BLNoClearReviewObserve(&r,0x999,151,42.0)==1);
    // Reset on failure, game change, strategy change or capture restart.
    BLNoClearReviewReset(&r);
    assert(!r.checks&&!r.active&&!r.complete);
    BLNoClearReviewBegin(&r,0x999,200,55.);
    assert(BLNoClearReviewObserve(&r,0x999,201,54.9)==-1);
    assert(!r.active);
    puts("PASS 50 distinct replay frames, >2s, no duplicates, signature cancellation and reset");
    return 0;
}
