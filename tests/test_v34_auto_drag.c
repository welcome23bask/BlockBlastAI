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
    assert(BLAutoDragEndsOnHighlight(&plan,&m,tray,43,220,323,500));
    float sx=0,sy=0;
    BLAutoDragPoint(&plan,1.f,&sx,&sy);
    assert(sx==plan.toX && sy==plan.toY);
    // The iPhone failure was a continuous overshooting drag from
    // (90,702) toward (240,384) on a 430x932 display. Every generated
    // point MUST remain between those endpoints and finish EXACTLY at
    // (240,384) without extrapolation.
    BLDragPlan realFailure={.fromX=90,.fromY=702,.toX=240,.toY=384,
                             .pieceIndex=1,.anchorX=1,.anchorY=0};
    float lastX=90,lastY=702;
    for(int i=0;i<=18;i++){
        float x=0,y=0;
        BLAutoDragPoint(&realFailure,(float)i/18.f,&x,&y);
        assert(x>=90 && x<=240 && y>=384 && y<=702);
        assert(x>=lastX && y<=lastY);
        lastX=x;lastY=y;
    }
    assert(lastX==240 && lastY==384);
    BLAutoDragPoint(&realFailure,1.2f,&sx,&sy);
    assert(sx==240 && sy==384);
    BLDragPlan wrongTarget=plan;
    wrongTarget.toX+=40;
    assert(!BLAutoDragEndsOnHighlight(&wrongTarget,&m,tray,43,220,323,500));
    // Runaway-drag protections: do not initiate near the edge, below the
    // phone viewport, or toward another screen's coordinate origin.
    float normalTx=tx[1],normalTy=ty[1];
    tx[1]=5;
    assert(!BLAutoBuildDrag(0,tray,&m,43,220,323,500,
                            tx,ty,tp,390,850,&plan));
    tx[1]=normalTx;
    ty[1]=844;
    assert(!BLAutoBuildDrag(0,tray,&m,43,220,323,500,
                            tx,ty,tp,390,850,&plan));
    ty[1]=normalTy;
    // Board center within nominal 8x8 but target nearly touches screen
    // edge: all automated coordinates must respect the safe inset.
    assert(!BLAutoBuildDrag(0,tray,&m,0,220,280,500,
                            tx,ty,tp,390,850,&plan));
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
    // A fragment of false-colored tray "pieces" must never be dragged.
    tray[1].x[3]=4; // disconnect rightmost tile from the first three
    assert(!BLAutoBuildDrag(0,tray,&m,43,220,323,500,tx,ty,tp,390,850,&plan));
    tray[1].x[3]=3;
    puts("PASS verified source-tile to target-cell drag geometry, with failures closed");
    return 0;
}
