#include "../src/BLTransition.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    BLPiece tray[3]={0},next[3]={0};
    for(int i=0;i<3;++i) {
        tray[i].count=1;
        tray[i].x[0]=0;
        tray[i].y[0]=0;
    }
    BLSolution plan=BLSolve(0,tray);
    assert(plan.movesFound>0);
    memcpy(next,tray,sizeof(tray));
    int used=plan.moves[0].pieceIndex;
    next[used].count=0;
    assert(BLIsExpectedTransition(0,tray,&plan,plan.moves[0].after,next));
    assert(!BLIsExpectedTransition(0,tray,&plan,0,next));
    next[used].count=1;
    assert(!BLIsExpectedTransition(0,tray,&plan,plan.moves[0].after,next));
    next[used].count=0;
    int other=(used+1)%3;
    next[other].x[0]=1;
    assert(!BLIsExpectedTransition(0,tray,&plan,plan.moves[0].after,next));
    next[other].x[0]=0;
    assert(!BLIsExpectedTransition(1,tray,&plan,plan.moves[0].after,next));
    BLSolution empty={0};
    assert(!BLIsExpectedTransition(0,tray,&empty,plan.moves[0].after,next));
    puts("Expected-next-move transition tests passed.");
    return 0;
}
