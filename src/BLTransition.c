#include "BLTransition.h"
#include <string.h>
static int same_piece(const BLPiece *a,const BLPiece *b) {
    if(a->count!=b->count || a->count<0 || a->count>25)return 0;
    for(int i=0;i<a->count;++i)
        if(a->x[i]!=b->x[i] || a->y[i]!=b->y[i])return 0;
    return 1;
}
int BLIsExpectedTransition(uint64_t oldBoard, const BLPiece oldTray[3],
                           const BLSolution *solution, uint64_t observedBoard,
                           const BLPiece observedTray[3]) {
    if(!oldTray || !observedTray || !solution || solution->movesFound<1 ||
       solution->movesFound>3)return 0;
    const BLMove *move=&solution->moves[0];
    int used=move->pieceIndex;
    if(used<0 || used>2 || oldTray[used].count<=0 ||
       move->before!=oldBoard || move->after!=observedBoard ||
       observedTray[used].count!=0) return 0;
    for(int p=0;p<3;++p)
        if(p!=used && !same_piece(&oldTray[p],&observedTray[p]))return 0;
    return 1;
}

static uint64_t apply_legal(uint64_t board,const BLPiece *piece,int row,int col,
                            int *lines) {
    uint64_t bits=0;
    for(int k=0;k<piece->count;++k) {
        int x=col+piece->x[k],y=row+piece->y[k];
        if(x<0||x>=8||y<0||y>=8)return UINT64_MAX;
        uint64_t bit=UINT64_C(1)<<(y*8+x);
        if((board|bits)&bit)return UINT64_MAX;
        bits|=bit;
    }
    uint64_t placed=board|bits,erase=0;
    *lines=0;
    for(int r=0;r<8;++r) {
        uint64_t mask=UINT64_C(0xff)<<(r*8);
        if((placed&mask)==mask) {erase|=mask;++*lines;}
    }
    for(int c=0;c<8;++c) {
        uint64_t mask=UINT64_C(0x0101010101010101)<<c;
        if((placed&mask)==mask) {erase|=mask;++*lines;}
    }
    return placed&~erase;
}
int BLInferObservedPlacement(uint64_t oldBoard,const BLPiece oldTray[3],
                              uint64_t newBoard,const BLPiece newTray[3],
                              BLMove *observed) {
    if(!oldTray||!newTray||!observed)return 0;
    int oldCount=0,newCount=0;
    for(int p=0;p<3;++p) {
        if(oldTray[p].count<0||oldTray[p].count>25 ||
           newTray[p].count<0||newTray[p].count>25)return 0;
        oldCount+=oldTray[p].count>0;
        newCount+=newTray[p].count>0;
    }
    // When the last visible piece is placed, the game normally replaces
    // all three tray slots. The exact board transition is still verifiable.
    int replenished=(oldCount==1 && newCount==3);
    int solutions=0;
    BLMove candidate={0};
    for(int p=0;p<3;++p) {
        const BLPiece *piece=&oldTray[p];
        if(piece->count<=0)continue;
        int shapeOkay=replenished;
        if(!replenished && newTray[p].count==0 &&
           newCount==oldCount-1) {
            shapeOkay=1;
            for(int q=0;q<3;++q)
                if(q!=p&&!same_piece(&oldTray[q],&newTray[q]))
                    shapeOkay=0;
        }
        if(!shapeOkay)continue;
        int width=0,height=0;
        for(int k=0;k<piece->count;++k) {
            if(piece->x[k]<0||piece->y[k]<0||piece->x[k]>=8||
               piece->y[k]>=8)return 0;
            if(piece->x[k]+1>width)width=piece->x[k]+1;
            if(piece->y[k]+1>height)height=piece->y[k]+1;
        }
        for(int row=0;row<=8-height;++row)
          for(int col=0;col<=8-width;++col) {
            int lines=0;
            uint64_t result=apply_legal(oldBoard,piece,row,col,&lines);
            if(result!=newBoard || result==UINT64_MAX)continue;
            if(++solutions>1)return 0; // indistinguishable moves are unsafe
            candidate.pieceIndex=p;
            candidate.row=row;candidate.col=col;
            candidate.before=oldBoard;candidate.after=newBoard;
            candidate.linesCleared=lines;
          }
    }
    if(solutions!=1)return 0;
    *observed=candidate;
    return 1;
}

// Keep this recorder independent of overlay invalidations. A scan failure
// removes the displayed hint immediately, but does not retroactively erase
// an already observed clear. Reconciliation on the next *confirmed* frame
// either proves continuity or discards stale streak confidence.
void BLComboTrackerReset(BLComboTracker *t) {
    if(t)memset(t,0,sizeof(*t));
}
static int same_tray(const BLPiece a[3],const BLPiece b[3]) {
    for(int i=0;i<3;++i)if(!same_piece(&a[i],&b[i]))return 0;
    return 1;
}
int BLComboTrackerObserve(BLComboTracker *t,uint64_t board,
                          const BLPiece tray[3],double timestamp) {
    if(!t||!tray)return 0;
    int counted=0;
    if(t->anchored) {
        double gap=timestamp-t->observedAt;
        int identical=(board==t->board && same_tray(t->tray,tray));
        int beforePieces=0,afterPieces=0;
        for(int p=0;p<3;++p) {
            beforePieces+=t->tray[p].count>0;
            afterPieces+=tray[p].count>0;
        }
        // Some frames show an empty tray before the next three appear;
        // a refill on the SAME board is not a player action.
        int refillOnly=(board==t->board && beforePieces==0 && afterPieces==3);
        if(gap<0 || gap>4.0) {
            t->streak=(BLComboContext){0};
        } else if(identical || refillOnly) {
            // Preserve verified history; do not add a fictitious placement.
        } else {
            BLMove actual={0};
            if(gap<=1.75 &&
               BLInferObservedPlacement(t->board,t->tray,board,tray,&actual)) {
                t->streak=BLComboAdvance(t->streak,actual.linesCleared);
                counted=1;
            } else {
                // Multiple unseen moves, tray inconsistency, user restart, etc.
                // Do not infer a combo history from the displayed score.
                t->streak=(BLComboContext){0};
            }
        }
    }
    t->board=board;
    memcpy(t->tray,tray,3*sizeof(BLPiece));
    t->observedAt=timestamp;
    t->anchored=1;
    return counted;
}
