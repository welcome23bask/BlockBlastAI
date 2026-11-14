// Independent local two-objective solver. The reference website's proprietary
// engine is not included or called: this enumerates legal three-piece routes
// under the existing 8x8 row/column clear model.
#include "BLDualStrategy.h"
#include <string.h>
#include <limits.h>

typedef struct { uint64_t mask; unsigned char row,col; } BLPlace;
typedef struct {
    uint64_t board;
    int depth,used,lines,firstClear,misses,streak,broken,seenClear;
    BLMove moves[3];
} BLPath;
typedef struct {
    BLPlace placements[3][64];
    int counts[3],total;
    const BLPiece *pieces;
    BLComboContext initial;
    BLPath maxPath,comboPath;
    int maxQuality,comboQuality,haveMax,haveCombo;
    uint32_t budget,visited;
    int truncated;
} BLSearch;
static int bits(uint64_t v) {return __builtin_popcountll(v);}
static uint64_t clear_full(uint64_t after,int *lines) {
    uint64_t erase=0;
    *lines=0;
    for(int r=0;r<8;++r) {
        uint64_t m=UINT64_C(255)<<(r*8);
        if((after&m)==m) {erase|=m;++*lines;}
    }
    for(int c=0;c<8;++c) {
        uint64_t m=UINT64_C(0x0101010101010101)<<c;
        if((after&m)==m) {erase|=m;++*lines;}
    }
    return after&~erase;
}
static int fit(uint64_t board,uint64_t mask,int w,int h) {
    int n=0;
    for(int r=0;r<=8-h;++r)
        for(int c=0;c<=8-w;++c)
            n+=!(board&(mask<<(r*8+c)));
    return n;
}
// Runs only when a candidate ties or improves a primary objective. Favors
// useful empty landing areas instead of merely unfilled cell count.
static int future_quality(uint64_t board) {
    int free=64-bits(board);
    int square=fit(board,UINT64_C(0x70707),3,3);
    int hbar=fit(board,UINT64_C(0x1f),5,1);
    int vbar=fit(board,UINT64_C(0x0101010101),1,5);
    int rect1=fit(board,UINT64_C(0x707),3,2);
    int rect2=fit(board,UINT64_C(0x30303),2,3);
    int rough=0;
    for(int r=0;r<8;++r) {
        uint64_t row=(board>>(r*8))&255;
        for(int c=1;c<8;++c)
            rough+=((row>>(c-1))&1)!=((row>>c)&1);
    }
    // Avoid rewarding a large number of possible 1x5 locations at the cost of
    // entirely losing all 3x3 and 2x3 positions when the board gets cramped.
    int missing=(square==0)*2200+(hbar==0)*600+(vbar==0)*600;
    return free*85+square*72+hbar*15+vbar*15+
           (rect1+rect2)*28-rough*12-missing;
}
static int preserved(const BLSearch *ctx,const BLPath *path) {
    if(ctx->initial.known && ctx->initial.streak>0)
        return !path->broken && path->streak>0;
    return path->seenClear;
}
static int earlier(int n) {return n? n:4;}
static void save_if_best(BLSearch *ctx,const BLPath *path) {
    int quality;
    int betterMax=!ctx->haveMax ||
        path->depth>ctx->maxPath.depth ||
        (path->depth==ctx->maxPath.depth && path->lines>ctx->maxPath.lines);
    int maybeMax=ctx->haveMax && path->depth==ctx->maxPath.depth &&
                 path->lines==ctx->maxPath.lines;
    int comboOK=preserved(ctx,path);
    int oldComboOK=ctx->haveCombo && preserved(ctx,&ctx->comboPath);
    int betterCombo=!ctx->haveCombo ||
        path->depth>ctx->comboPath.depth ||
        (path->depth==ctx->comboPath.depth && comboOK>oldComboOK) ||
        (path->depth==ctx->comboPath.depth && comboOK==oldComboOK &&
         earlier(path->firstClear)<earlier(ctx->comboPath.firstClear));
    int maybeCombo=ctx->haveCombo &&
         path->depth==ctx->comboPath.depth && comboOK==oldComboOK &&
         earlier(path->firstClear)==earlier(ctx->comboPath.firstClear);
    if(maybeCombo && path->lines>ctx->comboPath.lines)betterCombo=1;
    if(!betterMax&&!maybeMax&&!betterCombo&&!maybeCombo)return;
    quality=future_quality(path->board);
    if(maybeMax && (quality>ctx->maxQuality ||
                    (quality==ctx->maxQuality &&
                     earlier(path->firstClear)<earlier(ctx->maxPath.firstClear))))
        betterMax=1;
    if(maybeCombo && path->lines==ctx->comboPath.lines &&
        quality>ctx->comboQuality)
        betterCombo=1;
    if(betterMax) {
        ctx->maxPath=*path;ctx->maxQuality=quality;ctx->haveMax=1;
    }
    if(betterCombo) {
        ctx->comboPath=*path;ctx->comboQuality=quality;ctx->haveCombo=1;
    }
}
static void search(BLSearch *ctx,BLPath *path) {
    if(ctx->visited>=ctx->budget) {ctx->truncated=1;return;}
    ++ctx->visited;
    if(path->depth==ctx->total) {save_if_best(ctx,path);return;}
    int any=0;
    for(int p=0;p<3;++p) {
        if(ctx->pieces[p].count<=0 || (path->used&(1<<p)))continue;
        for(int j=0;j<ctx->counts[p];++j) {
            const BLPlace *pos=&ctx->placements[p][j];
            if(path->board&pos->mask)continue;
            any=1;
            int lines=0;
            BLPath next=*path;
            next.board=clear_full(path->board|pos->mask,&lines);
            next.used|=1<<p;
            next.moves[path->depth]=(BLMove) {
                .pieceIndex=p,.row=pos->row,.col=pos->col,
                .linesCleared=lines,.before=path->board,.after=next.board
            };
            ++next.depth;
            next.lines+=lines;
            if(lines>0) {
                if(!next.firstClear)next.firstClear=next.depth;
                next.seenClear=1;
                next.streak=next.streak<63?next.streak+1:63;
                next.misses=0;
            } else if(next.streak>0) {
                ++next.misses;
                if(next.misses>=3) {
                    if(ctx->initial.known && ctx->initial.streak>0)next.broken=1;
                    next.streak=0;next.misses=0;
                }
            }
            search(ctx,&next);
            if(ctx->truncated)return;
        }
    }
    if(!any)save_if_best(ctx,path);
}
static void pack(BLSolution *to,const BLPath *from,int approximate) {
    memset(to,0,sizeof(*to));
    to->movesFound=from->depth;
    to->totalLines=from->lines;
    to->approximate=approximate;
    for(int i=0;i<from->depth;++i)to->moves[i]=from->moves[i];
}
BLTwoStrategies BLFindTwoStrategies(uint64_t board,const BLPiece pieces[3],
                                    BLComboContext combo,uint32_t nodeBudget) {
    BLTwoStrategies out={0};
    if(!pieces)return out;
    BLSearch ctx={0};
    ctx.pieces=pieces;
    ctx.initial=combo;
    ctx.budget=nodeBudget?nodeBudget:180000;
    if(ctx.budget>500000)ctx.budget=500000;
    for(int p=0;p<3;++p) {
        const BLPiece *shape=&pieces[p];
        if(shape->count<0||shape->count>25)return out;
        if(!shape->count)continue;
        int width=0,height=0;
        uint64_t local=0;
        for(int i=0;i<shape->count;++i) {
            int x=shape->x[i],y=shape->y[i];
            if(x<0||x>=8||y<0||y>=8)return out;
            uint64_t bit=UINT64_C(1)<<(y*8+x);
            if(local&bit)return out;
            local|=bit;
            if(x+1>width)width=x+1;
            if(y+1>height)height=y+1;
        }
        ++ctx.total;
        for(int r=0;r<=8-height;++r)
            for(int c=0;c<=8-width;++c) {
                uint64_t m=0;
                for(int i=0;i<shape->count;++i)
                    m|=UINT64_C(1)<<((r+shape->y[i])*8+c+shape->x[i]);
                BLPlace *q=&ctx.placements[p][ctx.counts[p]++];
                q->mask=m;q->row=(unsigned char)r;q->col=(unsigned char)c;
            }
    }
    if(!ctx.total)return out;
    BLPath start={0};start.board=board;
    if(combo.known && combo.streak>0) {
        start.streak=combo.streak>63?63:combo.streak;
        start.misses=(combo.misses>=0&&combo.misses<3)?combo.misses:0;
    }
    search(&ctx,&start);
    out.exploredNodes=ctx.visited;
    out.exhaustive=!ctx.truncated;
    if(ctx.haveMax) {
        pack(&out.maxLines,&ctx.maxPath,ctx.truncated);
        out.maxFirstClear=ctx.maxPath.firstClear;
        out.maxComboPreserved=preserved(&ctx,&ctx.maxPath);
    }
    if(ctx.haveCombo) {
        pack(&out.keepCombo,&ctx.comboPath,ctx.truncated);
        out.comboFirstClear=ctx.comboPath.firstClear;
        out.keepComboPreserved=preserved(&ctx,&ctx.comboPath);
    }
    // Guard against budget-based pruning that misses a complete path.
    if(ctx.truncated && (out.maxLines.movesFound<ctx.total ||
                         out.keepCombo.movesFound<ctx.total)) {
        BLSolution fallback=BLSolveWithCombo(board,pieces,combo);
        if(fallback.movesFound>out.maxLines.movesFound)out.maxLines=fallback;
        if(fallback.movesFound>out.keepCombo.movesFound)out.keepCombo=fallback;
    }
    return out;
}
