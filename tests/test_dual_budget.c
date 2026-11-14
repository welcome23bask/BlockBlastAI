// Prove that the exhaustive solver obeys its hard node budget in otherwise
// combinatorial empty-board cases; do not hang a live iPhone overlay.
#define _POSIX_C_SOURCE 200809L
#include "../src/BLDualStrategy.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
static BLPiece one(void){BLPiece p={0};p.count=1;return p;}
static double msec(struct timespec a,struct timespec b) {
    return 1000.0*(b.tv_sec-a.tv_sec)+(b.tv_nsec-a.tv_nsec)/1e6;
}
int main(void){
    BLPiece p[3]={one(),one(),one()};
    BLComboContext combo={.known=1,.streak=11,.misses=2};
    struct timespec a,b;
    clock_gettime(CLOCK_MONOTONIC,&a);
    BLTwoStrategies result=BLFindTwoStrategies(0,p,combo,180000);
    clock_gettime(CLOCK_MONOTONIC,&b);
    assert(result.exploredNodes<=180000);
    assert(!result.exhaustive);
    assert(result.maxLines.movesFound==3&&result.keepCombo.movesFound==3);
    printf("Stress: nodes=%u, milliseconds=%.2f, max-lines=%d, keep-combo=%d\n",
        result.exploredNodes,msec(a,b),result.maxLines.totalLines,result.keepCombo.totalLines);
    puts("Search node limit and all-legal-fallback path passed.");
    return 0;
}
