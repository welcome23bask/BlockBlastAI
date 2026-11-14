// Frozen v1.1 BLINK baseline heuristic 3-piece beam solver. No I/O, allocation or network use.
#include "BlockBlastSolver.h"
#include <string.h>

#define BEAM_WIDTH 192
#define DEPTH_BONUS INT64_C(10000000)

typedef struct {
    uint64_t board;
    unsigned char used,depth,comboRun;
    int totalLines,comboBonus;
    BLMove moves[3];
    int64_t score;
} BLState;

typedef struct { int width,height; uint64_t shifts[64]; } BLPreparedPiece;
static int pc64(uint64_t v) { return __builtin_popcountll(v); }

static void prepare(const BLPiece *p, BLPreparedPiece *out) {
    memset(out,0,sizeof(*out));
    for (int i=0;i<p->count;++i) {
        int x=p->x[i],y=p->y[i];
        if (x+1>out->width) out->width=x+1;
        if (y+1>out->height) out->height=y+1;
    }
    for(int r=0;r<=8-out->height;++r) for(int c=0;c<=8-out->width;++c) {
        uint64_t mask=0;
        for(int i=0;i<p->count;++i)
            mask|=UINT64_C(1)<<((r+p->y[i])*8+c+p->x[i]);
        out->shifts[r*8+c]=mask;
    }
}

static uint64_t clear_lines(uint64_t placed,int *cleared) {
    uint64_t erased=0;
    int lines=0;
    for(int r=0;r<8;++r) {
        uint64_t m=UINT64_C(255)<<(r*8);
        if((placed&m)==m){erased|=m;++lines;}
    }
    for(int c=0;c<8;++c) {
        uint64_t m=UINT64_C(0x0101010101010101)<<c;
        if((placed&m)==m){erased|=m;++lines;}
    }
    *cleared=lines;
    return placed&~erased;
}

// Estimate how well the resulting board can fit common future shapes.
// The score intentionally values survival more than creating narrow 1-cell holes.
static int board_quality(uint64_t b) {
    int quality= -105*pc64(b),holes=0,rough=0,nearly=0;
    int fits2=0,fits3=0,lines4=0,rect23=0,rect32=0,lines5=0;
    for(int r=0;r<8;++r) {
        int filled=pc64((b>>(r*8))&255);
        if(filled>=5 && filled<=7) nearly+=(filled-4)*(filled-4);
        int prev=1;
        for(int c=0;c<8;++c) {
            int occupied=!!(b&(UINT64_C(1)<<(r*8+c)));
            rough+= (prev!=occupied);prev=occupied;
            if(!occupied) {
                int neighbors=(r==0 || (b&(UINT64_C(1)<<((r-1)*8+c))))!=0;
                neighbors+=(r==7 || (b&(UINT64_C(1)<<((r+1)*8+c))))!=0;
                neighbors+=(c==0 || (b&(UINT64_C(1)<<(r*8+c-1))))!=0;
                neighbors+=(c==7 || (b&(UINT64_C(1)<<(r*8+c+1))))!=0;
                if(neighbors==4) holes+=2;
                else if(neighbors==3) holes++;
            }
        }
        rough+=(prev==0);
        for(int c=0;c<=4;++c) {
            uint64_t mask=UINT64_C(15)<<(r*8+c);
            if(!(b&mask))lines4++;
            if(c<=3 && !(b&(UINT64_C(31)<<(r*8+c))))lines5++;
        }
    }
    for(int c=0;c<8;++c) {
        int filled=pc64((b>>c)&UINT64_C(0x0101010101010101));
        if(filled>=5 && filled<=7) nearly+=(filled-4)*(filled-4);
        int prev=1;
        for(int r=0;r<8;++r) {
            int occupied=!!(b&(UINT64_C(1)<<(r*8+c)));
            rough+=(prev!=occupied);prev=occupied;
        }
        rough+=(prev==0);
        for(int r=0;r<=4;++r) {
            uint64_t mask=UINT64_C(0x01010101)<<(r*8+c);
            if(!(b&mask))lines4++;
            if(r<=3 && !(b&(UINT64_C(0x0101010101)<<(r*8+c))))lines5++;
        }
    }
    for(int r=0;r<=6;++r)for(int c=0;c<=6;++c) {
        uint64_t m=UINT64_C(0x303)<<(r*8+c);
        if(!(b&m))fits2++;
        if(r<=5&&c<=5){
            uint64_t k=UINT64_C(0x70707)<<(r*8+c);
            if(!(b&k))fits3++;
        }
        if(c<=5 && !(b & (UINT64_C(0x707)<<(r*8+c))))rect23++;
        if(r<=5 && !(b & (UINT64_C(0x30303)<<(r*8+c))))rect32++;
    }
    // Maintain space for different likely future pieces, not just 3x3s.
    quality += fits2*13 + fits3*48 + lines4*9;
    quality += rect23*23 + rect32*23 + lines5*15;
    // Low capacity for either rectangle orientation is a future-tray risk.
    quality += (rect23<rect32?rect23:rect32)*24;
    quality += nearly*35 - holes*125 - rough*13;
    return quality;
}
// Favor routes that preserve room for the *current* remaining tray pieces.
static int future_options(const BLState *s,const BLPreparedPiece prep[3],
                          const BLPiece pieces[3]) {
    int score=0;
    for(int p=0;p<3;++p) {
        if(!pieces[p].count || (s->used&(1u<<p)))continue;
        const BLPreparedPiece *q=&prep[p];
        int options=0;
        for(int r=0;r<=8-q->height && options<12;++r)
            for(int c=0;c<=8-q->width && options<12;++c)
                options += !(s->board&q->shifts[r*8+c]);
        // A temporary zero might still be rescued by a line clear, so
        // penalize softly rather than treating it as permanently impossible.
        score+=options==0?-1800:45*options;
    }
    return score;
}
static int64_t grade(const BLState *s,const BLPreparedPiece prep[3],
                     const BLPiece pieces[3]) {
    return (int64_t)s->depth*DEPTH_BONUS +
           (int64_t)s->totalLines*5200 +
           (int64_t)s->comboBonus*700 +
           board_quality(s->board) +
           future_options(s,prep,pieces);
}

// Deduplicate equivalent board + used-piece states; retain the higher-scored
// path, which may have a better clearing combo and/or placement order.
static void insert(BLState beam[BEAM_WIDTH],int *count,const BLState *v) {
    for(int k=0;k<*count;++k) {
        if(beam[k].board==v->board && beam[k].used==v->used) {
            if(beam[k].score>=v->score)return;
            for(int j=k;j+1<*count;++j)beam[j]=beam[j+1];
            --*count;
            break;
        }
    }
    int i=0;
    while(i<*count && beam[i].score>=v->score)++i;
    if(i>=BEAM_WIDTH)return;
    int limit=(*count<BEAM_WIDTH)?*count+1:*count;
    for(int j=limit-1;j>i;--j)beam[j]=beam[j-1];
    beam[i]=*v;
    *count=limit;
}

BLSolution BLSolve(uint64_t board,const BLPiece pieces[3]) {
    BLSolution solution={0};
    solution.approximate=1;
    BLPreparedPiece prep[3];
    int total=0;
    for(int p=0;p<3;++p) {
        if(pieces[p].count<0 || pieces[p].count>25)return solution;
        if(!pieces[p].count)continue;
        uint64_t seen=0;
        for(int i=0;i<pieces[p].count;++i) {
            int x=pieces[p].x[i],y=pieces[p].y[i];
            if(x<0 || x>=8 || y<0 || y>=8)return solution;
            uint64_t bit=UINT64_C(1)<<(y*8+x);
            if(seen&bit)return solution; // Invalid or duplicate piece squares.
            seen|=bit;
        }
        prepare(&pieces[p],&prep[p]);
        total++;
    }
    if(!total)return solution;
    BLState beam[BEAM_WIDTH],next[BEAM_WIDTH],best={0};
    int count=1;
    beam[0]=best;
    best.board=board;
    beam[0].board=board;
    for(int depth=0;depth<total;++depth) {
        int nextCount=0;
        for(int k=0;k<count;++k) {
            const BLState *prior=&beam[k];
            for(int p=0;p<3;++p) {
                if(!pieces[p].count || (prior->used&(1<<p)))continue;
                const BLPreparedPiece *q=&prep[p];
                for(int r=0;r<=8-q->height;++r)for(int c=0;c<=8-q->width;++c) {
                    uint64_t m=q->shifts[r*8+c];
                    if(prior->board&m)continue;
                    BLState candidate=*prior;
                    candidate.depth=(unsigned char)(prior->depth+1);
                    candidate.used=(unsigned char)(prior->used|(1<<p));
                    int cleared=0;
                    candidate.board=clear_lines(prior->board|m,&cleared);
                    candidate.totalLines+=cleared;
                    if(cleared>0) {
                        candidate.comboRun=prior->comboRun+1;
                        candidate.comboBonus+=(int)candidate.comboRun*(int)candidate.comboRun;
                    } else candidate.comboRun=0;
                    BLMove *move=&candidate.moves[prior->depth];
                    move->pieceIndex=p;
                    move->row=r;
                    move->col=c;
                    move->linesCleared=cleared;
                    move->before=prior->board;
                    move->after=candidate.board;
                    candidate.score=grade(&candidate,prep,pieces);
                    insert(next,&nextCount,&candidate);
                }
            }
        }
        if(!nextCount)break;
        count=nextCount;
        memcpy(beam,next,sizeof(BLState)*(size_t)count);
        best=beam[0];
    }
    solution.movesFound=best.depth;
    solution.totalLines=best.totalLines;
    for(int i=0;i<best.depth;++i)solution.moves[i]=best.moves[i];
    return solution;
}
