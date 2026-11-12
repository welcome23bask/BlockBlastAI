#include "BlockBlastSolver.h"
#include <string.h>

#define BEAM_WIDTH 100

typedef struct {
    uint64_t board;
    int used, depth, totalLines;
    BLMove moves[3];
    int64_t score;
} BLState;

static int popcount64(uint64_t value) { return __builtin_popcountll(value); }

static int shape_width(const BLPiece *p) {
    int w = 0;
    for (int i=0; i<p->count; ++i) if (p->x[i]+1 > w) w=p->x[i]+1;
    return w;
}
static int shape_height(const BLPiece *p) {
    int h = 0;
    for (int i=0; i<p->count; ++i) if (p->y[i]+1 > h) h=p->y[i]+1;
    return h;
}
static uint64_t piece_mask(const BLPiece *p, int r, int c) {
    uint64_t mask=0;
    for (int i=0; i<p->count; ++i) mask |= (UINT64_C(1) << ((r+p->y[i])*8 + c+p->x[i]));
    return mask;
}
static uint64_t apply_and_clear(uint64_t board, uint64_t mask, int *lines) {
    uint64_t after=board | mask;
    uint64_t clear=0;
    *lines=0;
    for (int r=0; r<8; ++r) {
        uint64_t row=UINT64_C(255) << (r*8);
        if ((after & row)==row) { clear |= row; ++*lines; }
    }
    for (int c=0; c<8; ++c) {
        uint64_t col=0;
        for (int r=0; r<8; ++r) col |= UINT64_C(1) << (r*8+c);
        if ((after & col)==col) { clear |= col; ++*lines; }
    }
    return after & ~clear;
}
static int board_flexibility(uint64_t b) {
    // Reward empty 3x3 regions, and penalize isolated one-cell holes.
    int open3=0, isolated=0;
    for (int r=0; r<=5; ++r) for (int c=0; c<=5; ++c) {
        int clear=1;
        for (int dr=0; dr<3 && clear; ++dr) for (int dc=0; dc<3; ++dc)
            if (b & (UINT64_C(1) << ((r+dr)*8+c+dc))) { clear=0; break; }
        open3+=clear;
    }
    for (int r=1; r<7; ++r) for (int c=1; c<7; ++c) {
        uint64_t bit=UINT64_C(1) << (r*8+c);
        if (b & bit) continue;
        int surround=!!(b & (bit << 1)) + !!(b & (bit >> 1)) +
                     !!(b & (bit << 8)) + !!(b & (bit >> 8));
        if (surround==4) ++isolated;
    }
    return open3*12-isolated*24;
}
static int64_t grade(const BLState *s) {
    // Ranking puts successful placement of all 3 above any incomplete solution.
    return (int64_t)s->depth * INT64_C(10000000) +
           (int64_t)s->totalLines * 7000 - popcount64(s->board)*30 +
           board_flexibility(s->board);
}
static void add_candidate(BLState beam[BEAM_WIDTH], int *n, const BLState *candidate) {
    // Maintain sorted descending scores. BEAM_WIDTH bounds runtime on phones.
    int i=0;
    while (i<*n && beam[i].score >= candidate->score) ++i;
    if (i>=BEAM_WIDTH) return;
    int newCount=*n<BEAM_WIDTH ? *n+1 : *n;
    for (int j=newCount-1; j>i; --j) beam[j]=beam[j-1];
    beam[i]=*candidate;
    *n=newCount;
}
BLSolution BLSolve(uint64_t board, const BLPiece pieces[3]) {
    BLSolution sol;
    memset(&sol,0,sizeof(sol));
    sol.approximate=1;
    for (int i=0; i<3; ++i) if (pieces[i].count<=0 || pieces[i].count>25) return sol;

    BLState beam[BEAM_WIDTH], next[BEAM_WIDTH], current;
    memset(&current,0,sizeof(current));
    current.board=board;
    current.score=grade(&current);
    beam[0]=current;
    int beamCount=1;
    BLState best=current;
    for (int depth=0; depth<3; ++depth) {
        int nextCount=0;
        for (int stateIndex=0; stateIndex<beamCount; ++stateIndex) {
            BLState prior=beam[stateIndex];
            for (int p=0; p<3; ++p) {
                if ((prior.used>>p)&1) continue;
                int w=shape_width(&pieces[p]), h=shape_height(&pieces[p]);
                if (w>8 || h>8) continue;
                for (int r=0; r<=8-h; ++r) for (int c=0; c<=8-w; ++c) {
                    uint64_t mask=piece_mask(&pieces[p],r,c);
                    if (prior.board & mask) continue;
                    int cleared=0;
                    BLState candidate=prior;
                    candidate.depth=prior.depth+1;
                    candidate.used=prior.used | (1<<p);
                    candidate.board=apply_and_clear(prior.board,mask,&cleared);
                    candidate.totalLines+=cleared;
                    BLMove *move=&candidate.moves[prior.depth];
                    move->pieceIndex=p;
                    move->row=r;
                    move->col=c;
                    move->before=prior.board;
                    move->after=candidate.board;
                    move->linesCleared=cleared;
                    candidate.score=grade(&candidate);
                    add_candidate(next,&nextCount,&candidate);
                }
            }
        }
        if (nextCount==0) break;
        beamCount=nextCount;
        memcpy(beam,next,sizeof(BLState)*(size_t)beamCount);
        best=beam[0];
    }
    sol.movesFound=best.depth;
    sol.totalLines=best.totalLines;
    for (int i=0; i<best.depth; ++i) sol.moves[i]=best.moves[i];
    return sol;
}
