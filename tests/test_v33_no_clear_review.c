#include "../src/BLNoClearReview.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    BLNoClearReview r={0};
    // Reject stale and duplicated camera frames; count exactly five NEW ones.
    BLNoClearReviewBegin(&r,0x1234,7,10.);
    assert(BLNoClearReviewObserve(&r,0x1234,7,10.2)==0 && r.checks==0);
    for(int i=1;i<=5;++i){
        assert(BLNoClearReviewObserve(&r,0x1234,7+i,10.+i*.26)==0);
        assert(r.checks==(unsigned)i);
        assert(BLNoClearReviewObserve(&r,0x1234,7+i,10.+i*.26)==0);
        assert(r.checks==(unsigned)i);
    }
    // Five counts alone do not bypass two-second thinking window.
    assert(BLNoClearReviewObserve(&r,0x1234,13,12.1)==1);
    assert(r.complete==1 && r.active==0);
    // Different board/tray aborts the original negative conclusion.
    BLNoClearReviewBegin(&r,0x999,30,20.);
    assert(BLNoClearReviewObserve(&r,0x1000,31,20.2)==-1);
    assert(!r.active && !r.complete);
    // Each start is a new review; no cross-state confirmations.
    BLNoClearReviewBegin(&r,0x999,40,21.);
    for(int i=1;i<5;++i)assert(BLNoClearReviewObserve(&r,0x999,40+i,21+i*.6)==0);
    assert(r.checks==4);
    assert(BLNoClearReviewObserve(&r,0x999,45,23.9)==1);
    puts("PASS five separate fresh scans + 2-sec delay + state-change cancellation");
    return 0;
}
