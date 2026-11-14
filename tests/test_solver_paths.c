// Independent legality and board-transition property checks.
// This test deliberately does not call the solver's internal clear_lines().
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/BlockBlastSolver.h"

static uint32_t seed=0x9e3779b9u;
static uint32_t rnd(void) {
    uint32_t x=seed; x^=x<<13; x^=x>>17; x^=x<<5;
    return seed=x;
}
static const char *patterns[]={
    "#", "##", "###", "####", "#####", "#/#", "#/#/#", "#/#/#/#",
    "##/##", "###/###", "##/##/##", "###/###/###",
    "##./.##", ".##/##.", "#./##", ".#/##", ".#./###",
    "###/#.#", "#.#/###", "###/..#", "##./.##/..#"
};
static BLPiece piece(int shape) {
    BLPiece p={0};const char *a=patterns[shape];int x=0,y=0;
    for(;*a;++a) {
        if(*a=='/') {++y;x=0;continue;}
        if(*a=='#') {p.x[p.count]=x;p.y[p.count]=y;++p.count;}
        ++x;
    }
    return p;
}
static int legal(uint64_t b,const BLPiece *p,int row,int col) {
    if(row<0||col<0||row>=8||col>=8)return 0;
    for(int j=0;j<p->count;++j) {
        int x=col+p->x[j],y=row+p->y[j];
        if(x<0||y<0||x>=8||y>=8||(b&(UINT64_C(1)<<(y*8+x))))return 0;
    }
    return 1;
}
static uint64_t independent_transition(uint64_t b,const BLPiece *p,int row,int col,int *clears) {
    for(int j=0;j<p->count;++j)b|=UINT64_C(1)<<((row+p->y[j])*8+col+p->x[j]);
    uint64_t cleared=0;*clears=0;
    for(int r=0;r<8;++r) {
        int full=1;
        for(int c=0;c<8;++c) if(!(b&(UINT64_C(1)<<(r*8+c))))full=0;
        if(full) {++*clears;for(int c=0;c<8;++c)cleared|=UINT64_C(1)<<(r*8+c);}
    }
    for(int c=0;c<8;++c) {
        int full=1;
        for(int r=0;r<8;++r) if(!(b&(UINT64_C(1)<<(r*8+c))))full=0;
        if(full) {++*clears;for(int r=0;r<8;++r)cleared|=UINT64_C(1)<<(r*8+c);}
    }
    return b&~cleared;
}
int main(void) {
    uint32_t checked=0;
    for(int iteration=0;iteration<300;++iteration) {
        uint64_t board=0; BLPiece tray[3]={0};
        const int occupied=iteration%54;
        for(int j=0;j<occupied;++j)board|=UINT64_C(1)<<(rnd()%64);
        for(int p=0;p<3;++p) {
            tray[p]=piece((int)(rnd()%(sizeof(patterns)/sizeof(patterns[0]))));
            if((rnd()%7)==0)tray[p].count=0; // already-used tray pieces
        }
        uint64_t before=board;BLPiece unchanged[3];memcpy(unchanged,tray,sizeof(tray));
        BLSolution sol=BLSolve(board,tray);
        assert(board==before);
        assert(memcmp(unchanged,tray,sizeof(tray))==0);
        assert(sol.movesFound>=0 && sol.movesFound<=3);
        int used=0,totalClears=0;
        for(int j=0;j<sol.movesFound;++j) {
            BLMove move=sol.moves[j];
            assert(move.pieceIndex>=0 && move.pieceIndex<3);
            assert(!(used&(1<<move.pieceIndex)));
            assert(move.before==board);
            assert(tray[move.pieceIndex].count>0);
            assert(legal(board,&tray[move.pieceIndex],move.row,move.col));
            int lines=0;
            uint64_t expected=independent_transition(board,&tray[move.pieceIndex],move.row,move.col,&lines);
            assert(move.after==expected);
            assert(move.linesCleared==lines);
            board=move.after;totalClears+=lines;
            used|=1<<move.pieceIndex;
            checked++;
        }
        assert(totalClears==sol.totalLines);
    }
    printf("solver property tests passed: %u legal moves across 300 randomized states\n",checked);
    return 0;
}
