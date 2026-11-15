#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../src/BLTransition.h"
static BLPiece one(void){BLPiece p={0};p.count=1;return p;}
int main(void){
    BLPiece piece=one();
    BLPiece oneLeft[3]={piece,{0},{0}};
    BLPiece fullTray[3]={piece,piece,piece};
    BLPiece twoLeft[3]={{0},piece,piece};
    BLPiece lastLeft[3]={{0},{0},piece};
    BLComboTracker t={0};
    // Start a known opportunity: row 1 has 7/8 cells occupied.
    assert(!BLComboTrackerObserve(&t,UINT64_C(0x7f),oneLeft,100.0));
    // Clear using the final tray piece. Replenishment arrives immediately.
    assert(BLComboTrackerObserve(&t,0,fullTray,140.0));
    assert(t.streak.known && t.streak.streak==1 && t.streak.misses==0);
    // 260 seconds without placing a piece does NOT use any combo try.
    assert(BLComboTrackerObserve(&t,0,fullTray,400.0)==0);
    assert(t.streak.streak==1 && t.streak.misses==0);
    // Two verified non-clearing placements each use ONE try regardless of time.
    assert(BLComboTrackerObserve(&t,UINT64_C(1),twoLeft,600.0));
    assert(t.streak.streak==1 && t.streak.misses==1);
    assert(BLComboTrackerObserve(&t,UINT64_C(3),lastLeft,800.0));
    assert(t.streak.streak==1 && t.streak.misses==2);
    // Third non-clear consumes the final try, with last-piece tray refill.
    assert(BLComboTrackerObserve(&t,UINT64_C(7),fullTray,1000.0));
    assert(t.streak.streak==0 && t.streak.misses==0);
    puts("PASS combo: 3 placements after clear, timer ignored, inferred refill, reset after third miss");
    return 0;
}
