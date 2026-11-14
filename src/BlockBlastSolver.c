// BlockBlastAI v1.3 survival-first 3-piece beam solver.
// Predicts future *shape accessibility*, not unseen future pieces.
// No I/O, heap allocation, network calls or app-specific hooks.
#include "BlockBlastSolver.h"
#include <string.h>

#define BEAM_WIDTH 256
#ifndef BL_SURVIVAL_STRENGTH
#define BL_SURVIVAL_STRENGTH 100
#endif
// Strength can be changed when compiling controlled A/B tests; normal iPhone
// builds use 100. Shape frequencies are unknown: avoid pretending otherwise.
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

// Shape footprints for the NEXT unseen tray, NOT predictions of the actual RNG.
// Include difficult squares, long bars, broad rectangles and non-rectangles,
// in both orientations where appropriate. These are representative risks,
// not claimed frequencies or a reverse-engineered Block Blast generator.
typedef struct { uint64_t cells; unsigned char w,h,importance; } BLStressShape;
static const BLStressShape kFutureShapes[]={
    {UINT64_C(0x70707),3,3,6},           // 3x3 square
    {UINT64_C(0x1f),5,1,5},             // 1x5 bar
    {UINT64_C(0x0101010101),1,5,5},    // 5x1 bar
    {UINT64_C(0x707),3,2,4},           // 2x3 rectangle
    {UINT64_C(0x30303),2,3,4},         // 3x2 rectangle
    {UINT64_C(0x303),2,2,2},           // 2x2 square
    {UINT64_C(0x0f),4,1,3},            // 1x4 bar
    {UINT64_C(0x01010101),1,4,3},     // 4x1 bar
    {UINT64_C(0x10103),2,3,3},        // left-L 4
    {UINT64_C(0x20203),2,3,3},        // right-L 4
    {UINT64_C(0x107),3,2,3},          // L 4 variant
    {UINT64_C(0x407),3,2,3},          // mirrored L 4
    {UINT64_C(0x306),3,2,3},          // zigzag
    {UINT64_C(0x603),3,2,3}           // mirrored zigzag
};
static int count_fit(uint64_t occupied, const BLStressShape *shape) {
    int fits=0;
    for(int r=0;r<=8-(int)shape->h;++r)
        for(int c=0;c<=8-(int)shape->w;++c)
            if(!(occupied & (shape->cells << (r*8+c)))) ++fits;
    return fits;
}
// A near-full board with lots of individually empty cells can still be doomed:
// isolated empty components and a shortage of large-shape landing zones both
// increase the chance that the next unknown tray is impossible to place.
static int survival_shape_access(uint64_t occupied) {
    int free=64-pc64(occupied),covered=0,missing=0,flex=0,largeMissing=0;
    int n=(int)(sizeof(kFutureShapes)/sizeof(kFutureShapes[0]));
    for(int i=0;i<n;++i) {
        const BLStressShape *shape=&kFutureShapes[i];
        int fits=count_fit(occupied,shape);
        if(fits) {
            covered += shape->importance;
            int capped=fits>9?9:fits;
            flex += shape->importance*capped;
        } else {
            missing += shape->importance;
            if(i<5)largeMissing += shape->importance;
        }
    }
    int congestion=free<24 ? (24-free) : 0;
    // The progressive risk multiplier values shape access even before crisis.
    // Strongly prefer several *different* landing zones, not just one 3x3 hole.
    int score=covered*26+flex*11-missing*(45+congestion*7);
    score-=largeMissing*(free<29?100+congestion*12:30);
    if(free<18 && covered<20)score-= (18-free)*170;
    return score*BL_SURVIVAL_STRENGTH/100;
}
static int fragmented_space_penalty(uint64_t occupied) {
    uint64_t free=~occupied;
    int components=0,fragmentPenalty=0;
    const uint64_t notRight=UINT64_C(0x7f7f7f7f7f7f7f7f);
    const uint64_t notLeft=UINT64_C(0xfefefefefefefefe);
    while(free) {
        uint64_t region=free & (~free+1);
        uint64_t previous=0;
        do {
            previous=region;
            region|=(((region&notRight)<<1)|((region&notLeft)>>1)|
                      (region<<8)|(region>>8))&free;
        } while(region!=previous);
        int size=pc64(region);
        ++components;
        // Tiny separated voids cannot fit most useful shapes.
        if(size<3)fragmentPenalty+=(4-size)*95;
        else if(size<6)fragmentPenalty+=(6-size)*30;
        free&=~region;
    }
    if(components>1)fragmentPenalty+=(components-1)*110;
    return fragmentPenalty*BL_SURVIVAL_STRENGTH/100;
}

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
    quality += nearly*25 - holes*200 - rough*18;
    // Reward controllable free space and punish fragmentation. Large pieces
    // can be impossible to place with 20+ scattered empty cells, so raw
    // occupied-space count alone is not a sufficient survival heuristic.
    int occupancy=pc64(b);
    quality-=occupancy*occupancy*BL_SURVIVAL_STRENGTH/75;
    // The full 14-shape forecast is evaluated on beam finalists only.
    // Calling it for every possible candidate would stall live gameplay.
    quality-=fragmented_space_penalty(b);
    if(occupancy>31 && !fits3)quality-=(occupancy-30)*145*BL_SURVIVAL_STRENGTH/100;
    if(occupancy>27 && (lines5<2 || rect23+rect32<3))
        quality-=(occupancy-26)*100*BL_SURVIVAL_STRENGTH/100;
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
           (int64_t)s->totalLines*3200 +
           (int64_t)s->comboBonus*520 +
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
        // Explore with a fast, cheap local heuristic. Evaluate all *surviving*
        // beam states against harder unseen future shapes only once per depth.
        // This keeps the 0.24-second screen loop responsive.
        int64_t bestForecast=INT64_MIN;
        int chosen=0;
        for(int k=0;k<count;++k) {
            int64_t forecast=beam[k].score+(int64_t)survival_shape_access(beam[k].board);
            if(forecast>bestForecast) {bestForecast=forecast;chosen=k;}
        }
        best=beam[chosen];
    }
    solution.movesFound=best.depth;
    solution.totalLines=best.totalLines;
    for(int i=0;i<best.depth;++i)solution.moves[i]=best.moves[i];
    return solution;
}
