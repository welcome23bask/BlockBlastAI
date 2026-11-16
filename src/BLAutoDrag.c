#include "BLAutoDrag.h"
#include "BLGuideSafety.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
// Reject disconnected or duplicate scanner "pieces". Block Blast tray
// shapes are contiguous; a fragmented mask is a color-detection failure.
static int BLConnectedPiece(const BLPiece *p){
    if(!p||p->count<1||p->count>25)return 0;
    uint32_t bits=0;
    for(int k=0;k<p->count;k++){
        int x=p->x[k],y=p->y[k];
        if(x<0||x>4||y<0||y>4)return 0;
        uint32_t b=UINT32_C(1)<<(5*y+x);
        if(bits&b)return 0;
        bits|=b;
    }
    uint32_t reachable=bits&(~bits+1U);
    for(;;){
        uint32_t expanded=reachable;
        for(int y=0;y<5;y++)for(int x=0;x<5;x++){
            uint32_t pos=UINT32_C(1)<<(5*y+x);
            if(!(reachable&pos))continue;
            if(x>0)expanded|=pos>>1;
            if(x<4)expanded|=pos<<1;
            if(y>0)expanded|=pos>>5;
            if(y<4)expanded|=pos<<5;
        }
        expanded &= bits;
        if(expanded==reachable)break;
        reachable=expanded;
    }
    return reachable==bits;
}
int BLAutoBuildDrag(uint64_t board,const BLPiece tray[3],
                    const BLMove *move,float fx,float fy,
                    float lx,float ly,const float tx[3],
                    const float ty[3],const float tp[3],
                    float w,float h,BLDragPlan *out) {
    if(!tray||!move||!tx||!ty||!tp||!out)return 0;
    if(move->pieceIndex<0||move->pieceIndex>2 ||
       move->before!=board)return 0;
    const int p=move->pieceIndex;
    if(!BLConnectedPiece(&tray[p]))return 0;
    float pitchX=(lx-fx)/7.f,pitchY=(ly-fy)/7.f;
    if(!isfinite(pitchX)||!isfinite(pitchY) ||
       pitchX<20||pitchY<20||
       fabsf(pitchX-pitchY)>fmaxf(pitchX,pitchY)*.10f ||
       w<240||h<350||!isfinite(w)||!isfinite(h))return 0;
    if(tp[p]<pitchX*.25f || tp[p]>pitchX*.80f ||
       !isfinite(tp[p])||!isfinite(tx[p])||!isfinite(ty[p]))
        return 0;
    BLSolution verify={0};
    verify.movesFound=1;
    verify.moves[0]=*move;
    if(!BLGuideValidateSolution(board,tray,&verify))return 0;
    // Select an occupied local tile close to the shape centroid, never
    // the empty center of an L or U piece.
    float sumX=0,sumY=0;
    for(int k=0;k<tray[p].count;k++){
        sumX+=tray[p].x[k];sumY+=tray[p].y[k];
    }
    float midX=sumX/tray[p].count,midY=sumY/tray[p].count;
    int best=-1;float dmin=1e20f;
    for(int k=0;k<tray[p].count;k++){
        float dx=tray[p].x[k]-midX,dy=tray[p].y[k]-midY;
        float d=dx*dx+dy*dy;
        if(d<dmin){dmin=d;best=k;}
    }
    if(best<0)return 0;
    int px=tray[p].x[best],py=tray[p].y[best];
    float x0=tx[p]+px*tp[p],y0=ty[p]+py*tp[p];
    float x1=fx+(move->col+px)*pitchX;
    float y1=fy+(move->row+py)*pitchY;
    // All touchscreen coordinates are UIKit points, not screenshot pixel
    // coordinates. Keep the whole drag away from system-gesture edges.
    // A previous iPhone run carried a tile offscreen; fail closed rather
    // than risk another uncalibrated border or orientation mismatch.
    const float edge=fmaxf(22.f,fminf(pitchX,pitchY)*.45f);
    if(x0<edge||x0>w-edge||y0<ly+pitchY*.8f||y0>h-edge ||
       x1<edge||x1>w-edge||y1<edge||y1>h-edge ||
       y0<y1+pitchY*1.5f)return 0;
    // The entire interpolation must remain in the safe visible viewport.
    for(int step=0;step<=20;step++){
        float fraction=step/20.f;
        float x=x0+(x1-x0)*fraction,y=y0+(y1-y0)*fraction;
        if(!isfinite(x)||!isfinite(y)||
           x<edge||x>w-edge||y<edge||y>h-edge)return 0;
    }
    // Screen geometry and tray slot must agree. Do not dispatch a touch
    // starting on a different slot or system control.
    float slotMiddle=w*((float)p+.5f)/3.f;
    if(fabsf(x0-slotMiddle)>w*.28f)return 0;
    *out=(BLDragPlan){x0,y0,x1,y1,px,py,p};
    return 1;
}

int BLAutoTargetIsProvenEmpty(const BLPiece tray[3],const BLMove *move,
                             uint64_t certainlyEmptyMask) {
    if(!tray||!move||move->pieceIndex<0||move->pieceIndex>=3)return 0;
    const BLPiece *piece=&tray[move->pieceIndex];
    if(piece->count<=0||piece->count>25)return 0;
    uint64_t targets=0;
    for(int k=0;k<piece->count;k++) {
        int col=move->col+piece->x[k];
        int row=move->row+piece->y[k];
        if(col<0||col>=8||row<0||row>=8)return 0;
        uint64_t cell=UINT64_C(1)<<(row*8+col);
        if((targets&cell)!=0)return 0;
        targets|=cell;
    }
    // The scanner's own board is not sufficient; a second pixel
    // classifier must independently certify every destination EMPTY.
    return targets!=0 && (targets&certainlyEmptyMask)==targets;
}
