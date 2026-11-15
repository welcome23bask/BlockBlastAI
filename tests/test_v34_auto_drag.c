#include "../src/BLAutoDrag.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
int main(void) {
    BLPiece tray[3]={{0}};
    tray[1].count=4;
    for(int i=0;i<4;i++){tray[1].x[i]=i;tray[1].y[i]=0;}
    BLMove m={0};
    m.pieceIndex=1;m.row=2;m.col=1;m.before=0;
    for(int i=1;i<=4;i++)m.after|=UINT64_C(1)<<(16+i);
    float tx[3]={70,162,310},ty[3]={660,660,660};
    float tp[3]={17.6f,17.6f,17.6f};
    BLDragPlan plan={0};
    assert(BLAutoBuildDrag(0,tray,&m,43,220,323,500,tx,ty,tp,390,850,&plan));
    assert(plan.pieceIndex==1);
    assert(plan.toY==300);
    assert(plan.fromY>500);
    assert(plan.toX>=80 && plan.toX<=250);
    uint64_t allEmpty=UINT64_MAX;
    assert(BLAutoTargetIsProvenEmpty(tray,&m,allEmpty));
    // A scanner saying this move is legal is NOT enough. The second
    // per-pixel view must certify all FOUR destination cells empty.
    assert(!BLAutoTargetIsProvenEmpty(tray,&m,allEmpty &
           ~(UINT64_C(1)<<17)));
    assert(!BLAutoTargetIsProvenEmpty(tray,&m,0));
    assert(!BLAutoTargetIsProvenEmpty(tray,&m,
           (UINT64_C(1)<<17)|(UINT64_C(1)<<18)));
    // The proof cannot be recycled for a different destination.
    BLMove moved=m;moved.col=2;
    assert(!BLAutoTargetIsProvenEmpty(tray,&moved,
            (UINT64_C(1)<<17)|(UINT64_C(1)<<18)|
            (UINT64_C(1)<<19)|(UINT64_C(1)<<20)));

    m.after++;
    assert(!BLAutoBuildDrag(0,tray,&m,43,220,323,500,tx,ty,tp,390,850,&plan));
    m.after--;
    tp[1]=0;
    assert(!BLAutoBuildDrag(0,tray,&m,43,220,323,500,tx,ty,tp,390,850,&plan));
    tp[1]=17.6f;
    assert(!BLAutoBuildDrag(0,tray,&m,323,220,43,500,tx,ty,tp,390,850,&plan));
    assert(!BLAutoBuildDrag(UINT64_C(1),tray,&m,43,220,323,500,tx,ty,tp,390,850,&plan));
    puts("PASS verified source-tile to target-cell drag geometry, with failures closed");
    return 0;
}
