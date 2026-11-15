#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include "BLThreeMoveClear.h"
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <time.h>

typedef struct {uint64_t mask; unsigned char row,col;} BLPlacement;
typedef struct {
    BLPlacement positions[3][64];
    int count[3],visible,tries;
    uint32_t budget,visited;
    uint64_t deadlineMs;
    int truncated,haveBest,bestRank,bestFirst;
    BLSolution best;
} BLPlanner;

static inline int popcount(uint64_t n){return __builtin_popcountll(n);}
static uint64_t monotonicMs(void) {
    struct timespec ts={0};
    if(clock_gettime(CLOCK_MONOTONIC,&ts)!=0)return 0;
    return (uint64_t)ts.tv_sec*UINT64_C(1000)+(uint64_t)ts.tv_nsec/1000000;
}
static uint64_t clear_lines(uint64_t board,int *cleared){
    uint64_t gone=0;*cleared=0;
    for(int r=0;r<8;++r){
        uint64_t mask=UINT64_C(255)<<(r*8);
        if((board&mask)==mask){gone|=mask;++*cleared;}
    }
    for(int c=0;c<8;++c){
        uint64_t mask=UINT64_C(0x0101010101010101)<<c;
        if((board&mask)==mask){gone|=mask;++*cleared;}
    }
    return board &~gone;
}
static int fits(uint64_t board,int w,int h) {
    int options=0;
    for(int r=0;r<=8-h;++r)for(int c=0;c<=8-w;++c) {
        int clear=1;
        for(int y=0;y<h&&clear;++y)for(int x=0;x<w;++x)
            if(board&(UINT64_C(1)<<((r+y)*8+c+x))){clear=0;break;}
        options+=clear;
    }
    return options;
}
static int future_space(uint64_t board) {
    int quality=(64-popcount(board))*20;
    quality+=fits(board,3,3)*30;
    quality+=fits(board,2,2)*12;
    quality+=fits(board,5,1)*8;
    quality+=fits(board,1,5)*8;
    return quality;
}
// Before a FIRST clear, no lines have been removed. Thus every piece in
// the clearing sequence occupies cells that were free on the initial board.
// Compute the maximum possible contribution to each line by every visible
// piece, then sum only the best remaining-try slots. If every row/column
// still falls short of 8, a first clear is mathematically impossible.
// This is a NECESSARY upper bound, not an optimistic move suggestion.
static int BLCouldClearAnyLine(uint64_t board,const BLPlanner *ctx) {
    const uint64_t column=UINT64_C(0x0101010101010101);
    for(int line=0;line<16;++line) {
        uint64_t mask=line<8 ? (UINT64_C(255)<<(line*8))
                             : (column<<(line-8));
        int filled=popcount(board&mask);
        int perPiece[3]={0,0,0};
        if(filled==8)return 1; // invalid full board: don't hide anomaly
        for(int p=0;p<3;++p) {
            for(int k=0;k<ctx->count[p];++k) {
                uint64_t place=ctx->positions[p][k].mask;
                if(place&board)continue;
                int hits=popcount(place&mask);
                if(hits>perPiece[p])perPiece[p]=hits;
            }
        }
        // Sort three contributions descending. Every first clearing sequence
        // uses at most tries visible pieces (no unknown future tray pieces).
        for(int i=0;i<3;++i)for(int j=i+1;j<3;++j) {
            if(perPiece[j]>perPiece[i]) {
                int tmp=perPiece[i];perPiece[i]=perPiece[j];perPiece[j]=tmp;
            }
        }
        int upper=filled;
        for(int k=0;k<ctx->tries&&k<3;++k)upper+=perPiece[k];
        if(upper>=8)return 1;
    }
    return 0;
}
static void record(BLPlanner *ctx,const BLMove moves[3],int depth,
                   int firstClear,int totalLines,uint64_t after) {
    if(!firstClear||firstClear>ctx->tries)return;
    // Always prefer an actual clear to a speculative combo. Within clearing
    // plans, prefer room for every remaining visible tray piece and an early
    // clear before choosing the safest/flexiblest end board.
    int rank=(depth==ctx->visible?10000000:0)
             +depth*1000000-firstClear*100000
             +future_space(after)*6+totalLines*1000;
    if(ctx->haveBest&&rank<=ctx->bestRank)return;
    ctx->bestRank=rank;ctx->haveBest=1;ctx->bestFirst=firstClear;
    memset(&ctx->best,0,sizeof(ctx->best));
    ctx->best.movesFound=depth;ctx->best.totalLines=totalLines;
    for(int i=0;i<depth;++i)ctx->best.moves[i]=moves[i];
}
static void search(BLPlanner *ctx,uint64_t board,const BLMove moves[3],
                   int depth,unsigned used,int firstClear,int totalLines) {
    if(ctx->visited>=ctx->budget){ctx->truncated=1;return;}
    // Check periodically to keep large clear searches under the user-visible
    // ~2 sec thinking budget without paying a clock syscall per leaf.
    if(ctx->deadlineMs && (ctx->visited & 1023U)==0 &&
       monotonicMs()>=ctx->deadlineMs){ctx->truncated=1;return;}
    ++ctx->visited;
    if(firstClear)record(ctx,moves,depth,firstClear,totalLines,board);
    if(depth>=ctx->visible)return;
    // We know only the tray pieces currently visible; if the last chance
    // would be consumed without a clear, that path cannot preserve combo.
    if(!firstClear&&depth>=ctx->tries)return;
    for(int p=0;p<3;++p){
        if(!ctx->count[p]||(used&(1u<<p)))continue;
        for(int i=0;i<ctx->count[p];++i){
            const BLPlacement *place=&ctx->positions[p][i];
            if(board&place->mask)continue;
            int lines=0;
            uint64_t nextBoard=clear_lines(board|place->mask,&lines);
            BLMove newMoves[3];
            memcpy(newMoves,moves,sizeof(newMoves));
            newMoves[depth]=(BLMove){
                .pieceIndex=p,.row=place->row,.col=place->col,
                .before=board,.after=nextBoard,.linesCleared=lines
            };
            search(ctx,nextBoard,newMoves,depth+1,used|(1u<<p),
                   firstClear?firstClear:(lines?depth+1:0),
                   totalLines+lines);
            if(ctx->truncated)return;
        }
    }
}
static BLThreeMovePlan BLFindThreeMoveClearImpl(uint64_t board,
                                         const BLPiece pieces[3],
                                         BLComboContext combo,uint32_t budget,
                                         uint32_t thinkingMs) {
    BLThreeMovePlan out={0};
    if(!pieces)return out;
    int misses=(combo.known&&combo.streak>0)?combo.misses:0;
    if(misses<0||misses>2)misses=0;
    out.triesLeft=3-misses;
    BLPlanner ctx={0};ctx.tries=out.triesLeft;
    ctx.budget=budget?budget:500000;
    if(ctx.budget>1500000)ctx.budget=1500000;
    // No deadline for compatibility calls. Timed mode gets at most 2 sec;
    // zero ms means no deadline, never an unlimited node search.
    if(thinkingMs) {
        if(thinkingMs>2000)thinkingMs=2000;
        uint64_t now=monotonicMs();
        if(now)ctx.deadlineMs=now+thinkingMs;
    }
    for(int p=0;p<3;++p) {
        const BLPiece *piece=&pieces[p];
        if(piece->count<0||piece->count>25)return out;
        if(!piece->count)continue;
        int width=0,height=0;
        uint64_t unique=0;
        for(int k=0;k<piece->count;++k) {
            int x=piece->x[k],y=piece->y[k];
            if(x<0||y<0||x>=5||y>=5)return out;
            uint64_t cell=UINT64_C(1)<<(y*8+x);
            if(unique&cell)return out;
            unique|=cell;
            if(width<x+1)width=x+1;
            if(height<y+1)height=y+1;
        }
        ++ctx.visible;
        for(int r=0;r<=8-height;++r)for(int c=0;c<=8-width;++c) {
            uint64_t mask=0;
            for(int k=0;k<piece->count;++k) {
                mask|=UINT64_C(1)<<((r+piece->y[k])*8+c+piece->x[k]);
            }
            BLPlacement *slot=&ctx.positions[p][ctx.count[p]++];
            slot->mask=mask;slot->row=(unsigned char)r;slot->col=(unsigned char)c;
        }
    }
    out.visiblePieces=ctx.visible;
    if(!ctx.visible)return out;
    // A single clear requires at least eight squares in a row/column;
    // if even ALL remaining pieces cannot reach eight occupied in total,
    // no line can be cleared this tray, so skip expensive DFS completely.
    int available=popcount(board);
    for(int p=0;p<3;++p)available+=pieces[p].count;
    if(available<8){out.completeSearch=1;return out;}
    if(!BLCouldClearAnyLine(board,&ctx)) {
        out.completeSearch=1;
        out.upperBoundPruned=1;
        // This is a proven negative, not time-budget search truncation.
        // Five independently captured frames still have to verify the image.
        return out;
    }
    BLMove moves[3]={{0}};
    search(&ctx,board,moves,0,0,0,0);
    out.explored=ctx.visited;
    out.completeSearch=!ctx.truncated;
    if(ctx.haveBest){
        out.found=1;out.firstClear=ctx.bestFirst;
        out.solution=ctx.best;
        out.solution.approximate=ctx.truncated;
    }
    return out;
}

BLThreeMovePlan BLFindThreeMoveClear(uint64_t board,const BLPiece pieces[3],
                                     BLComboContext combo,uint32_t budget) {
    return BLFindThreeMoveClearImpl(board,pieces,combo,budget,0);
}
BLThreeMovePlan BLFindThreeMoveClearTimed(uint64_t board,const BLPiece pieces[3],
                                          BLComboContext combo,uint32_t budget,
                                          uint32_t hardBoardThinkingMs) {
    return BLFindThreeMoveClearImpl(board,pieces,combo,budget,hardBoardThinkingMs);
}
